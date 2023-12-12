#define US_TRIG 2
#define US_ECHO 3
#define IR_LEVEL A0

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(9600);
  
  //
  // Ultrasonic init
  //
  pinMode(US_TRIG, OUTPUT);
  pinMode(US_ECHO, INPUT);
  
  digitalWrite(US_TRIG, LOW);

  Serial.println("Ready.");
  Serial.println("Send `c 10 30` to start calibrating for range of 10cm to 30cm");
}

int us_distance() {
  digitalWrite(US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);
  
  int duration = pulseIn(US_ECHO, HIGH);
  float distance = duration * 0.034 / 2;
  
  return distance;
}

void ir_measure() {
  int level = analogRead(A0);

  Serial.println(level);
}

enum SYSTEM_MODES {
  AWAITING_COMMANDS,
  CALIBRATING
} system_mode = AWAITING_COMMANDS;

struct cal_state_t {
  int start;
  int stop;
  long* bins; // Корзины значений с инфракрасного дальномера, одна корзина - один сантиметр
  char* count; // Количество значений в корзине, будет использоваться для усреднения
} cal_state = {0};

static void start_calibration(int start, int stop) {
  if (cal_state.bins) free(cal_state.bins);
  if (cal_state.count) free(cal_state.count);

  cal_state.bins = calloc(stop + 1, sizeof(long));
  cal_state.count = calloc(stop + 1, 1);

  cal_state.start = start;
  cal_state.stop = stop;

  system_mode = CALIBRATING;

  Serial.println("Now in CALIBRATING");
}

static void wait_commands() {
  if (Serial.available() > 1) {
    char cmd = Serial.read();
    char sep = Serial.read();

    switch (cmd) {
      case 'c':
        int start = Serial.parseInt();
        int stop = Serial.parseInt();

        start_calibration(start, stop);

        break;
      default:
        Serial.println("Unknown command");
        break;
    }
  }
}

static void calibration_complete() {
  Serial.println("Leaving CALIBRATION.");

  for (int i = cal_state.start; i <= cal_state.stop; i++) {
    Serial.print(i);
    Serial.print("\t");
    Serial.println(cal_state.bins[i]);
  }

  Serial.println("Send `c 10 30` to start calibrating for range of 10cm to 30cm");
  system_mode = AWAITING_COMMANDS;
}

static void calibration_step() {
  for (int i = cal_state.start; i <= cal_state.stop; i++) {
    if (cal_state.count[i] < 50)
      break;

    if (i == cal_state.stop)
      calibration_complete();
  }

  // Берём расстояние по звуку
  int distance = us_distance();

  // Если расстояние вне диапазона калибровки - мигать светодиодом очень быстро
  if (distance < cal_state.start or distance > cal_state.stop) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(40);
    return;
  }

  // Если соответствующая корзина уже полна - светодиод горит
  // Иначе светодиод выключен и мы накапливаем значения
  if (cal_state.count[distance] >= 50) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    return;
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  cal_state.bins[distance] += analogRead(A0);
  cal_state.count[distance]++;

  if (cal_state.count[distance] == 50) {
    Serial.print("Finished calibrating for distance ");
    Serial.println(distance);
  }
}

void loop()
{
  switch (system_mode) {
    case AWAITING_COMMANDS:
      wait_commands();
      break;
    case CALIBRATING:
      calibration_step();
      break;
  }

  //us_measure();
  //ir_measure();
  //delay(250);
}
