#include <Wire.h>

//
// Slave device
//

#define DATASET_SIZE 64
#define TARGET_SAMPLE_COUNT 3

// Датасет на 64 строки
// Сохраняется (и накапливается для усреднения) разница между мастером и слейвом
struct dataset_t {
  int delta;
  uint8_t count;
} dataset[DATASET_SIZE] = {0};

void setup() {
  Serial.begin(115200);
  Serial.println("Slave startup");

  Wire.begin(1);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void loop() {
  delay(100);
}

int dataset_entries() {
  int count = 0;

  for (int i = 0; i < DATASET_SIZE; i++) {
    if (dataset[i].count == TARGET_SAMPLE_COUNT)
      count++;
  }

  return count;
}

void dataset_add(int x, int y) {
  int idx = x / (1024 / DATASET_SIZE);

  if (dataset[idx].count < TARGET_SAMPLE_COUNT) {
    int delta = y - x;
    dataset[idx].delta += delta;
    dataset[idx].count++;
  }
}

void dataset_dump() {
  Serial.println("Dataset dump");
  Serial.println("delta\tcount");

  for (int i = 0; i < DATASET_SIZE; i++) {
    Serial.print(dataset[i].delta);
    Serial.print("\t");
    Serial.println(dataset[i].count);
  }
}

void receiveEvent(int length) {
  int y = 0;

  if (Wire.available() >= 2) {
    y += Wire.read();
    y += Wire.read() << 8;
  }

  // Выкинуть остальные байты, чтобы не заблокировалось
  while (Wire.available())
    Wire.read();

  int x = analogRead(A0);

  int entries = dataset_entries();

  if (entries < 12) {
    Serial.println(entries);
    dataset_add(x, y);
  } else {

    // Линейная интерполяция между соседними корзинами датасета
    int idx_lo = x / (1024 / DATASET_SIZE);
    int idx_hi = idx_lo + (DATASET_SIZE - idx_lo > 1 ? 1 : 0);
    int lerp_step = x % (1024 / DATASET_SIZE);

    int cal_lo = dataset[idx_lo].delta / dataset[idx_lo].count;
    int cal_hi = dataset[idx_hi].delta / dataset[idx_hi].count;

    int cal = cal_lo + lerp_step*(cal_hi - cal_lo)/(1024 / DATASET_SIZE);
    

    //int idx = x / DATASET_SIZE;
    //int cal = dataset[idx].delta / dataset[idx].count;

    Serial.print(cal);
    Serial.print(" ");
    Serial.print(y);
    Serial.print(" ");
    Serial.println(x + cal);
  }
}

void requestEvent() {
  //Wire.write("Hello from slave");
}
