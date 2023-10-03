
void setup() {
	pinMode(BUILTIN_LED, OUTPUT);
	digitalWrite(BUILTIN_LED, LOW);
	Serial.begin(115200);
}

void handle_commands() {
	if (Serial.available() > 0) {
		char command = Serial.read();

		switch (command) {
			case 'u':
				Serial.print("led on\n");
				digitalWrite(BUILTIN_LED, LOW);
				break;
			case 'd':
				Serial.print("led off\n");
				digitalWrite(BUILTIN_LED, HIGH);
				break;
		}
	}
}

void loop() {
	handle_commands();
}
