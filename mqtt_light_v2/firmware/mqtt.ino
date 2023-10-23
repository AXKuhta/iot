
//
// PubSubClient by Nick O'Leary
//

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <PubSubClient.h>

// MQTT
const int mqtt_port = 1883;
const char* mqtt_broker = "broker.emqx.io";

extern WiFiClient wifiClient;
String ap_ssid_id();

PubSubClient mqtt_client(wifiClient);

void handle_message_fn(char* topic, byte* payload, unsigned int len) {
  String command_topic = "esp8266_" + ap_ssid_id() + "/command";

  Serial.print("MQTT: message on topic ");
  Serial.print(topic);
  Serial.print(": ");

  if (String(topic) == command_topic) {
    digitalWrite(5, payload[0] == 'e');
  }

  Serial.print("\n");
}

static void reconnect() {
  String client_id = "esp8266_axkuhta_" + ap_ssid_id();
  Serial.println("MQTT client id is " + client_id);

  if (mqtt_client.connect(client_id.c_str())) {
    Serial.println("MQTT client connected");
  } else {
    Serial.println("Failed to connect to mqtt");
  }

  String command_topic = "esp8266_" + ap_ssid_id() + "/command";
  String state_topic = "esp8266_" + ap_ssid_id() + "/state";

  mqtt_client.subscribe(command_topic.c_str());
  mqtt_client.publish(state_topic.c_str(), "hello");

  Serial.println("MQTT ready\n");
  Serial.println("Command channel: " + command_topic);
  Serial.println("State messages: " + state_topic);
}

bool init_mqtt() {
  Serial.print("MQTT startup\n");

  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(handle_message_fn);

  reconnect();

  return true;
}

// No delay required
// https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino
void mqtt_handle() {
  if (!mqtt_client.connected()) {
    Serial.print("MQTT client lost connection; attempting to reconnect\n");
    reconnect();
  }

  mqtt_client.loop();
}
