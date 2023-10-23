#include "config.h"

// Sketch -> Include Library -> Manage libraries
// ESP8266WiFi

// WifiEsp by bportaluri
// Wifi by arduino
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti wifiMulti;
WiFiClient wifiClient;

String ip = "IP not set";

// Выцепить последние два байта из MAC адреса ESP
String ap_ssid_id() {
  int mac_len = WL_MAC_ADDR_LENGTH;
  uint8_t mac[mac_len];

  WiFi.softAPmacAddress(mac);

  String mac_id = String(mac[mac_len - 2], HEX) +
                  String(mac[mac_len - 1], HEX);

  return mac_id;
}

// Запуск ESP в режиме SoftAP
bool start_ap_mode() {
  IPAddress ap_IP(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  String network_name = AP_NAME + ap_ssid_id();

  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ap_IP, ap_IP, subnet); // IP, Gateway, Subnet
  WiFi.softAP(network_name.c_str(), AP_PASSWORD.c_str());

  Serial.print("WiFi started in AP mode: ");
  Serial.print(network_name);
  Serial.print("\n");

  ip = WiFi.softAPIP().toString();

  return true;
}

void stop_ap_mode() {
  WiFi.softAPdisconnect();
}

// Запуск ESP в режиме клиента
bool start_client_mode(const char* client_ssid, const char* client_pass) {
  wifiMulti.addAP(client_ssid, client_pass);

  if (wifiMulti.run() != WL_CONNECTED) {
    return false;
  }

  ip = WiFi.localIP().toString();

  Serial.print("WiFi started in client mode; connected to ");
  Serial.print(client_ssid);
  Serial.print(" as ");
  Serial.print(ip);
  Serial.print("\n");

  return true;
}

bool client_mode_status() {
  return WiFi.status() == WL_CONNECTED;
}
