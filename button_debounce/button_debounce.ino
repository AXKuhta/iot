
#define BUTTON_PIN  5 // Используем D1 (GPIO5) на плате NodeMCU

// Частота таймера с TIM_DIV1: 80 МГц или 160 МГц
// Разрешение таймера: 23 бита
// https://github.com/khoih-prog/ESP8266TimerInterrupt/blob/master/src/ESP8266TimerInterrupt.h
#define DEBOUNCE_MS (50) // 50мс для подавления дребезга
#define TIMER_CLOCKS (ESP8266_CLOCK / (1000 / DEBOUNCE_MS))

int press_count = 0;

void button_debounce_sample() {
  int val = digitalRead(BUTTON_PIN);

  if (val == 0) {
    //Serial.println("Button pressed");
    press_count++;
  }
}

void ICACHE_RAM_ATTR process_button_press() {
  //int val = digitalRead(BUTTON_PIN);
  //Serial.println(val);

  timer1_write(TIMER_CLOCKS);
  timer1_enable(TIM_DIV1, TIM_EDGE, TIM_SINGLE);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nHello!");

  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, 0);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  timer1_attachInterrupt(button_debounce_sample);

  //attachInterrupt(5, process_button_depress, RISING); // Получается обнаружить FALLING или RISING по отдельности, но не оба одновременно
  attachInterrupt(5, process_button_press, FALLING);
}

void loop() {
  Serial.print("Button presses during last 2s: ");
  Serial.println(press_count);
  press_count = 0;
  delay(2000);
}

