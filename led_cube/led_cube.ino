
// Clockwise sinks
#define S1 7
#define S2 2
#define S3 4
#define S4 8

// Layer sources
#define L1 5
#define L2 6

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);

  Serial.begin(115200);
  Serial.println("System startup");

  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(S4, OUTPUT);

  digitalWrite(S1, 0);
  digitalWrite(S2, 0);
  digitalWrite(S3, 0);
  digitalWrite(S4, 0);

  analogWrite(L1, 127);
  analogWrite(L2, 127);
}

// 4 bits -> sinks layer 1
// 4 bits -> sinks layer 2
const uint8_t lut[] = {
  0b10000000, // 1
  0b01000000, // 2
  0b00100000, // 3
  0b00010000, // 4
  0b00001000, // 1 TOP
  0b00000100, // 2 TOP
  0b00000010, // 3 TOP
  0b00000001, // 4 TOP
  0b10000010, // 1 TOP + 3 BOTTOM
  0b00101000, // 3 BOTTOM + 1 TOP
};

uint32_t led_cube_update_at = 0;
uint8_t led_pattern = 0;

void handle_led_cube() {
  static int layer = 0;

  if (millis() >= led_cube_update_at) {
    uint8_t nibble = led_pattern >> 4;

    if (layer)
      nibble = led_pattern & 0xF;

    digitalWrite(S1, !(nibble & 0b1000));
    digitalWrite(S2, !(nibble & 0b0100));
    digitalWrite(S3, !(nibble & 0b0010));
    digitalWrite(S4, !(nibble & 0b0001));

    if (layer) {
      digitalWrite(L1, 0);
      digitalWrite(L2, 1);
    } else {
      digitalWrite(L1, 1);
      digitalWrite(L2, 0);
    }

    led_cube_update_at += 1;
    layer = 1 - layer;
  }
}

void loop() {
  handle_led_cube();

  //int lut_idx = millis() % 10000 / 1000;
  //led_pattern = lut[lut_idx];

  led_pattern = millis() % 25600 / 100;
}
