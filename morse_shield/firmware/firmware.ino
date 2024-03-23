
uint32_t sample_at;
uint32_t tu = 10000;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.begin(115200);
  Serial.println("System startup");

  pinMode(3, OUTPUT); // GND
  digitalWrite(3, LOW);
  
  pinMode(2, INPUT); // Signal in

  pinMode(5, OUTPUT); // DATA
  pinMode(6, OUTPUT); // CLOCK
  pinMode(7, OUTPUT); // OE CLK
  
  display_digit(0);

  attachInterrupt(0, sync, CHANGE); // 0 обозначает INT0
}

//
//  1
// 3 2
//  4
// 5 8
//  6 (7)
//

static void display_digit(int idx) {
  char lut[] = {
    0b11101101, // 0
    0b01000001, // 1
    0b11011100, // 2
    0b11010101, // 3
    0b01110001, // 4
    0b10110101, // 5
    0b10111101, // 6
    0b11000001, // 7
    0b11111101, // 8
    0b11110101  // 9
  };
  
  digitalWrite(6, LOW);
  shiftOut(7, 5, LSBFIRST, lut[idx]);
  digitalWrite(6, HIGH);
}

typedef struct { uint8_t k; char v; } entry_t;

// Декодер морзе
class Decoder {
public:
  entry_t alpha1[2] = {
    0b0, 'E',
    0b1, 'T'
  };

  entry_t alpha2[4] = {
    0b01, 'A',
    0b00, 'I',
    0b11, 'M',
    0b10, 'N'
  };

  entry_t alpha3[8] = {
    0b100, 'D',
    0b110, 'G',
    0b101, 'K',
    0b111, 'O',
    0b010, 'R',
    0b000, 'S',
    0b001, 'U',
    0b011, 'W'
  };

  entry_t alpha4[12] = {
    0b1000, 'B',
    0b1010, 'C',
    0b0010, 'F',
    0b0000, 'H',
    0b0111, 'J',
    0b0100, 'L',
    0b0110, 'P',
    0b1101, 'Q',
    0b0001, 'V',
    0b1001, 'X',
    0b1011, 'Y',
    0b1100, 'Z'
  };

  uint8_t value = 0;
  uint8_t count = 0;

  char lookup(entry_t tab[], uint8_t key, size_t sz) {
    for (int i = 0; i < sz; i++) {
      if (tab[i].k == value) {
        return tab[i].v;
      }
    }
    return '?';
  }

  void dot() {
    value = (value << 1);
    count++;
  }

  void dash() {
    value = (value << 1) + 1;
    count++;
  }

  void flush() {
    switch (count) {
      case 1:
        Serial.print(lookup(alpha1, value, 2));
        break;
      case 2:
        Serial.print(lookup(alpha2, value, 4));
        break;
      case 3:
        Serial.print(lookup(alpha3, value, 8));
        break;
      case 4:
        Serial.print(lookup(alpha4, value, 12));
        break;
      default:
        Serial.println("Decoder: unknown length");
        break;
    }

    value = 0;
    count = 0;
  }
};

// Стейт-машина приёмника
class Receiver {
public:
    enum {
        DISCONNECT,
        IDLE,
        SPACE,
        MARK
    } mode = DISCONNECT;

    Decoder decoder;
    int count = 0;

    void transition_to_idle() {
        mode = IDLE;
        count = 0;
    }

    void transition_to_space() {
        mode = SPACE;
        count = 0;
    }

    void transition_to_mark() {
        mode = MARK;
        count = 0;
    }

    void transition_to_disconnect() {
        mode = DISCONNECT;
        count = 0;
    }

    void advance(bool input) {
        if (input) {
            switch (mode) {
                case IDLE:
                    count++;
                    return;
                case SPACE:
                    count++;
                    if (count > 1) {
                        //Serial.println("Event: transaction end");
                        Serial.print(" ");
                        transition_to_idle();
                    } else if (count == 1) {
                        decoder.flush();
                    }
                    return;
                case MARK:
                    count++;
                    switch (count) {
                      case 1:
                        decoder.dot();
                        break;
                      case 2:
                        decoder.dash();
                        break;
                      default:
                        Serial.println("Invalid mark");
                        break;
                    }
                    transition_to_space();
                    return;
                case DISCONNECT:
              		  Serial.println("Event: line connected");
                    transition_to_idle();
                    return;
                default:
                    while (1) {};
            }
        } else {
            switch (mode) {
                case IDLE:
                    //Serial.println("Event: transaction start");
                    transition_to_mark();
                    return;
                case SPACE:
                    transition_to_mark();
                    return;
                case MARK:
                    count++;
              		  if (count > 4) {
                      	Serial.println("Event: line disconnected");
                        transition_to_disconnect();
                    }
                    return;
                case DISCONNECT:
                    count++;
                    return;
                default:
                    while (1) {};
            }
        }
    }
};

Receiver recv;

void sample() {
  if (micros() >= sample_at) {
  	recv.advance( digitalRead(2) );
    sample_at += tu;
  }
}

void loop()
{
  int val = analogRead(A5);
  int speed = map(val, 0, 1023, 0, 9);
  display_digit(speed);

  //for (int i = 0; i < 10; i++) {
  //  display_digit(i);
  //  delay(100);
  //}
  //display_digit(val / (1023 / 9));
  //delay(40);
  sample();
}

// Синхронизация по спадам
// Возможный баг: потеря данных если micros() переполнился в процессе получения (происходит каждые 70 минут)
void sync() {
  sample_at = micros() + tu/2; // Выравниваемся по центру одного TU
}
