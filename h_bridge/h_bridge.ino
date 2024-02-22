//
// Принципиальная схема:
//
//   3──────────────────┐
//                      │
//  11────┐             │
//        │             │
// +5V───PNP───────┐ ┌─NPN───GND
//                 │ │
//             ┌───┼─┘
//             │   │
//             ├─M─┤
//             │   │
//             │   │
// +5V───PNP───┘   └───NPN───GND
//        │             │
//  10────┘             │
//                      │
//   9──────────────────┘
//

// Убрать таймер от ног
// Мы не можем гарантировать состояние каналов OC при остановке таймера
// Так что не будем останавливать таймер, а будем отбирать у него ноги
static void tim1_inhibit() {
  digitalWrite(10, HIGH);
  digitalWrite(9, LOW);
  TCCR1A = 0b00000001;
}

static void tim1_connect() {
  TCCR1A = 0b10110001;
}

static void tim1_power(uint8_t x) {
  OCR1A = x;
  OCR1B = x;
}

static void tim2_inhibit() {
  digitalWrite(11, HIGH);
  digitalWrite(3, LOW);
  TCCR2A = 0b00000011;
}

static void tim2_connect() {
  TCCR2A = 0b10110011;
}

static void tim2_power(uint8_t x) {
  OCR2A = 255 - x;
  OCR2B = 255 - x;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  Serial.println("System startup");

  // https://docs.arduino.cc/tutorials/generic/secrets-of-arduino-pwm/
  // Timer output 	Arduino output
  // OC0A	          6
  // OC0B	          5
  // OC1A	          9
  // OC1B	          10
  // OC2A	          11
  // OC2B	          3

  // TIM1
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  digitalWrite(9, LOW);

  // TIM2
  pinMode(11, OUTPUT);
  pinMode(3, OUTPUT);
  digitalWrite(11, HIGH);
  digitalWrite(3, LOW);

  //
  // TIM1
  //

  OCR1A = 160;
  OCR1B = 160;

  // 10 = OC0A Non-inverting
  // 11 = OC0B Inverting
  // 00 = Unused
  // 01 = Fast PWM, 8 bit
  //TCCR1A = 0b10110001;
  TCCR1A = 0b00000001; // Но на старте убрать ноги от таймера, на всякий случай

  // 00 = No input capture
  // 0 = Unused
  // 01 = Fast PWM, 8 bit
  // 011 = /64 Prescaler, 000 = Stop
  TCCR1B = 0b00001011;

  //
  // TIM2
  //

  OCR2A = 32;
  OCR2B = 32;

  // 10 = OC0A Non-inverting
  // 11 = OC0B Inverting
  // 00 = Unused
  // 11 = Fast PWM
  //TCCR2A = 0b10110011;
  TCCR2A = 0b00000011; // Но на старте убрать ноги от таймера, на всякий случай

  // 00 = No force OC
  // 00 = Unused
  // 0 = Fast PWM
  // 011 = /64 Prescaler, 000 = Stop
  TCCR2B = 0b00000011;
}

static void drive(uint8_t direction, uint8_t power) {
  if (direction) {
    tim1_inhibit();
    tim2_connect();
    tim2_power(power);
  } else {
    tim2_inhibit();
    tim1_connect();
    tim1_power(power);
  }
}

static void demo() {
  for (int i = 0; i <= 255; i++) {
    drive(0, i);
    delay(10);
  }

  for (int i = 255; i >= 0; i--) {
    drive(0, i);
    delay(10);
  }
  
  
  for (int i = 0; i <= 255; i++) {
    drive(1, i);
    delay(10);
  }

  for (int i = 255; i >= 0; i--) {
    drive(1, i);
    delay(10);
  }
}

void loop() {
  demo();
}
