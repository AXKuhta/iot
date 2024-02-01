// Предназначено для NodeMCU V3

#define COL2 14 // D5 -- D0 использовать нельзя, в нём нет internal pullup
#define COL1 5  // D1
#define ROW1 4  // D2
#define ROW2 0  // D3

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(COL1, INPUT_PULLUP);
  pinMode(COL2, INPUT_PULLUP);

  pinMode(ROW1, OUTPUT);
  pinMode(ROW2, OUTPUT);

  digitalWrite(ROW1, HIGH);
  digitalWrite(ROW2, HIGH);

  Serial.begin(115200);
  Serial.println("Startup");
}

void scan_button_matrix() {
  // Сканирование первой строки
  digitalWrite(ROW1, LOW);
  digitalWrite(ROW2, HIGH);

  int b1 = 1 - digitalRead(COL1);
  int b2 = 1 - digitalRead(COL2);

  // Сканирование второй строки
  digitalWrite(ROW1, HIGH);
  digitalWrite(ROW2, LOW);

  int b3 = 1 - digitalRead(COL1);
  int b4 = 1 - digitalRead(COL2);

  // Ждать без зажигания светодиодов
  digitalWrite(ROW1, HIGH);
  digitalWrite(ROW2, HIGH);

  Serial.println("Button states:");
  Serial.print(b1);
  Serial.print(b2);
  Serial.print(b3);
  Serial.print(b4);
  Serial.println("");  
}

void loop() {
  scan_button_matrix();
  delay(30);
}
