#define LED_PIN 10
#define LIGHT_SENSOR 0

void setup() {
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, LOW);
	Serial.begin(115200);
}

// Modes of operation:
// 1. Alarm
// 2. Auto ON/OFF
// 3. Forced ON/OFF

enum SYSTEM_MODE{
	MODE_ALARM,
	MODE_AUTO_ON_OFF,
	MODE_MANUAL,
} mode = MODE_MANUAL;

// System state:
bool alarm_triggered = false;
bool manual_mode_led = false;

void handle_commands() {
	if (Serial.available() > 0) {
		char command = Serial.read();

		switch (command) {
			case 'e':
				Serial.print("Manual mode: LED on\n");
				manual_mode_led = true;
				mode = MODE_MANUAL;
				break;
			case 'd':
				Serial.print("Manual mode: LED off\n");
				manual_mode_led = false;
				mode = MODE_MANUAL;
				break;
			case 'a':
				Serial.print("Automatic ON/OFF mode\n");
				mode = MODE_AUTO_ON_OFF;
				break;
			case 's':
				Serial.print("Alarm mode: armed\n");
				alarm_triggered = false;
				mode = MODE_ALARM;
				break;
		}
	}
}

void handle_manual_mode() {
	digitalWrite(LED_PIN, manual_mode_led);
}

void handle_auto_on_off_mode() {
	int value = analogRead(LIGHT_SENSOR);

	if (value > 350) {
		digitalWrite(LED_PIN, LOW);
	} else {
		digitalWrite(LED_PIN, HIGH);
	}
}

void handle_alarm_mode() {
	int value = analogRead(LIGHT_SENSOR);
	static int state;

	if (value < 350)
		alarm_triggered = true;

	if (alarm_triggered) {
		state = state ^ 1;
		digitalWrite(LED_PIN, state);
	} else {
		digitalWrite(LED_PIN, LOW);
	}
}

void handle_event() {
	switch (mode) {
		case MODE_MANUAL:
			handle_manual_mode();
			break;
		case MODE_AUTO_ON_OFF:
			handle_auto_on_off_mode();
			break;
		case MODE_ALARM:
			handle_alarm_mode();
			break;
	}
}

long next_event = 0;

void loop() {
	handle_commands();

	long now = millis();

	if (now >= next_event) {
		next_event = now + 100;
		handle_event();
	}
}
