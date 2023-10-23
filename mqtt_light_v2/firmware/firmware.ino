#include "wifi.h"
#include "server.h"
#include "mqtt.h"

#include <PubSubClient.h>

bool mqtt_up = false;

void modeswitch_mqtt() {
  stop_server();
  stop_ap_mode();
  init_mqtt();

  mqtt_up = true;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);

  Serial.begin(115200);
  Serial.print("\f\rSystem startup\n");

  start_ap_mode();
  init_server();
}

void loop() {
  server_handle();
  
  if (mqtt_up) {
    mqtt_handle();

    if (client_mode_status() == false) {
        Serial.print("Network failure; switching into SETUP\n");
        start_ap_mode();
        init_server();
        mqtt_up = false;
    }
  }
}
