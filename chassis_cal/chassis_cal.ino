#include <SoftwareSerial.h>
#include <EEPROM.h>

#define R_DIRECTION   4   // LOW - Вперёд
#define R_POWER       5
#define L_DIRECTION   7   // HIGH - Вперёд
#define L_POWER       6

#define EEP_BASE 512

SoftwareSerial soft_serial(10, 11); // RX, TX

struct cal_t {
  uint8_t profile;
  int compensation;
  int rotation_timings[4];
} cal = {0};

// FORWARD RIGHT
// BACK = !FORWARD
// LEFT = !RIGHT
struct profile_t { bool F1; bool F2; bool R1; bool R2; } profiles[8] = {
  HIGH, HIGH, HIGH, LOW,
  HIGH, HIGH, LOW, HIGH,
  LOW, LOW, HIGH, LOW,
  LOW, LOW, LOW, HIGH,
  HIGH, LOW, HIGH, HIGH,
  HIGH, LOW, LOW, LOW,
  LOW, HIGH, HIGH, HIGH,
  LOW, HIGH, LOW, LOW
};

// Вывести калибровочные значения на указанный последовательный интерфейс
void print_cal_data(Stream& stream) {
    stream.print("EEPROM loaded\n");
    stream.print("profile = ");
    stream.println(cal.profile);
    stream.print("compensation =");
    stream.println(cal.compensation);
    stream.print("rot90 = ");
    stream.println(cal.rotation_timings[0]);
    stream.print("rot180 = ");
    stream.println(cal.rotation_timings[1]);
    stream.print("rot270 = ");
    stream.println(cal.rotation_timings[2]);
    stream.print("rot360 = ");
    stream.println(cal.rotation_timings[3]);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);

  pinMode(R_DIRECTION, OUTPUT);
  pinMode(L_DIRECTION, OUTPUT);
  pinMode(R_POWER, OUTPUT);
  pinMode(L_POWER, OUTPUT);

  digitalWrite(R_DIRECTION, LOW);
  digitalWrite(L_DIRECTION, HIGH);

  Serial.begin(115200);
  Serial.println("System startup");

  soft_serial.begin(9600);
  soft_serial.println("Hello phone");

  EEPROM.get(EEP_BASE, cal);

  if (cal.profile > 7 or cal.compensation > 200 or cal.compensation < -200) {
    Serial.print("EEPROM contains no valid calibration\n");
    cal.profile = 0;
    cal.compensation = 0;
  } else {
    print_cal_data(Serial);
    print_cal_data(soft_serial);
  }
}

void set_power(int power) {
  int l = power;
  int r = power;

  // Замедляем левый либо замедляем правый
  if (cal.compensation > 0) {
    l -= cal.compensation;
  } else {
    r += cal.compensation;
  }

  if (l < 0) l = 0;
  if (r < 0) r = 0;

  if (l > 255) l = 0;
  if (r > 255) r = 0;

  analogWrite(R_POWER, r);
  analogWrite(L_POWER, l);
}

// Вперёд/назад
void forward(int power) {
  digitalWrite(R_DIRECTION, profiles[cal.profile].F1);
  digitalWrite(L_DIRECTION, profiles[cal.profile].F2);
  set_power(power);
}

void backwards(int power) {
  digitalWrite(R_DIRECTION, 1 - profiles[cal.profile].F1);
  digitalWrite(L_DIRECTION, 1 - profiles[cal.profile].F2);
  set_power(power);
}

// Поворот на месте
void rotate_left(int power) {
  digitalWrite(R_DIRECTION, 1 - profiles[cal.profile].R1);
  digitalWrite(L_DIRECTION, 1 - profiles[cal.profile].R2);
  set_power(power);
}

void rotate_right(int power) {
  digitalWrite(R_DIRECTION, profiles[cal.profile].R1);
  digitalWrite(L_DIRECTION, profiles[cal.profile].R2);
  set_power(power);
}

void stop() {
  analogWrite(R_POWER, 0);
  analogWrite(L_POWER, 0);
}

class ManualControl {
public:
  uint32_t update_at = 0;
  uint32_t x_down_at = 0;
  uint8_t x_slot = 0;

  void write_to_eeprom() {
    EEPROM.put(EEP_BASE, cal);

    // Запись в EEPROM является невозвратной операцией
    // Чтобы случайно не начать портить EEPROM
    while (1) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(100);
    }
  }

  void x_measure_stop() {
    uint32_t x_held_for = millis() - x_down_at;
    cal.rotation_timings[x_slot++] = x_held_for;

    if (x_slot == 4)
      write_to_eeprom();

    x_down_at = 0;
  }

  void x_measure_start() {
    x_down_at = millis();
  }

  // 100 Гц частота обновления ручного управления
  void update() {
    if (millis() < update_at)
      return;

    update_at += 10;

    while (soft_serial.available()) {
      char c = soft_serial.read();

      switch (c) {
        case 'F':
          forward(255);
          break;
        case 'L':
          rotate_left(255);
          break;
        case 'B':
          backwards(255);
          break;
        case 'R':
          rotate_right(255);
          break;
        case '0':
          stop();
          if (x_down_at) x_measure_stop();
          break;
        case 'T':
          cal.profile = ++cal.profile % 8;
          break;
        case 'S':
          cal.compensation += 3; // Veer left
          break;
        case 'C':
          cal.compensation -= 3; // Veer right
          break;
        case 'X':
          x_measure_start();
          rotate_left(255);
          break;
      }
    }
  }
};

ManualControl manual;

//    F      T
//   L R    S C
//    B      X
//
// 0   Key up
void bent_pipe() {
  if (soft_serial.available())
   Serial.write(soft_serial.read());

  if (Serial.available())
   soft_serial.write(Serial.read());
}

void loop() {
  manual.update();
  //bent_pipe();
}
