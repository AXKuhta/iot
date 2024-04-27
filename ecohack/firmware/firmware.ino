#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SdsDustSensor.h>    // https://github.com/lewapek/sds-dust-sensors-arduino-library/tree/master
#include <time.h>             // https://forum.arduino.cc/t/ntp-server-doesnt-get-the-right-date-esp8266/938391/19


const char SSID[] = "0";
const char PASS[] = "00100011";

const int mqtt_port = 8883;
const char* mqtt_broker = "broker.emqx.io";

const char* sensors_topic = nullptr; // To be filled
const char* command_topic = nullptr; // To be filled

BearSSL::WiFiClientSecure tls_client;
//WiFiClient wifiClient;
PubSubClient mqtt_client(tls_client);

int rxPin = D1; // ESP8266 SDD2
int txPin = D2; // ESP8266 SDD3
SdsDustSensor sds(rxPin, txPin);

// Выцепить последние два байта из MAC адреса ESP
String ap_ssid_id() {
  int mac_len = WL_MAC_ADDR_LENGTH;
  uint8_t mac[mac_len];

  WiFi.softAPmacAddress(mac);

  String mac_id = String(mac[mac_len - 2], HEX) +
                  String(mac[mac_len - 1], HEX);

  return mac_id;
}

// ====================================================================================================================
// WiFi
// ====================================================================================================================
void wifi_init() {
  WiFi.begin(SSID, PASS);

  while( WiFi.status() != WL_CONNECTED ) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
  };
}

// ====================================================================================================================
// Real time
// ====================================================================================================================
char* timestamp() {
  char* buf = (char*)malloc(32);
  time_t now;
  tm tm;  

  time(&now);                       // read the current time
  localtime_r(&now, &tm);           // update the structure tm with the current time

  strftime(buf, 32, "%Y-%m-%d %H:%M:%S", &tm);

  return buf;
}

void print_time() {
  char* ts = timestamp();

  Serial.println(ts);

  free(ts);
}

void ntp_init() {
  Serial.print("NTP Startup\n");
  configTime("UTC", "time.google.com", "time.windows.com", "pool.ntp.org");

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    now = time(nullptr);
    delay(100);
  }

  Serial.print("Current time: ");
  print_time();
}

// ====================================================================================================================
// TLS
// ====================================================================================================================
static const char ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----
)EOF";

void tls_init() {
  randomSeed(micros());
  BearSSL::X509List *ca_crt = new BearSSL::X509List(ca_cert);
  tls_client.setTrustAnchors(ca_crt);
}

// ====================================================================================================================
// MQTT
// ====================================================================================================================
void handle_message_fn(char* topic, byte* payload, unsigned int len) {
  Serial.print("MQTT: message on topic ");
  Serial.print(topic);
  Serial.print(": ");
}

// Подключение или переподключение MQTT
static void reconnect() {
  String client_id = "esp8266_kt315_" + ap_ssid_id();

  if (mqtt_client.connect(client_id.c_str())) {
    Serial.println("MQTT OK");
  } else {
    Serial.println("MQTT Fail");
  }

  mqtt_client.subscribe(command_topic);
}

// Первичный запуск MQTT
void mqtt_init() {
  Serial.print("MQTT startup\n");

  String a = "ecohack_kt315/esp8266_" + ap_ssid_id() + "/sensors";
  String b = "ecohack_kt315/esp8266" + ap_ssid_id() + "/command";

  sensors_topic = strdup( a.c_str() );
  command_topic = strdup( b.c_str() );

  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(handle_message_fn);
}

// No delay required
// https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino
void mqtt_handle() {
  if (!mqtt_client.connected()) {
    reconnect();
  }

  mqtt_client.loop();
}

// ====================================================================================================================
// Dust
// ====================================================================================================================
void dust_init() {
  sds.begin();

  Serial.println(sds.queryFirmwareVersion().toString()); // prints firmware version
  Serial.println(sds.setActiveReportingMode().toString()); // ensures sensor is in 'active' reporting mode
  Serial.println(sds.setContinuousWorkingPeriod().toString()); // ensures sensor has continuous working period - default but not recommended
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  Serial.begin(115200);
  Serial.print("System startup\n");

  wifi_init();
  ntp_init();
  tls_init();
  mqtt_init();
  dust_init();

  Serial.print("Command channel: ");
  Serial.println(sensors_topic);
  Serial.print("Sensors channel: ");
  Serial.println(command_topic);
}

void publish_dust(float pm25, float pm10) {
  char* buf = (char*)malloc(512);
  char* ts = timestamp();

  Serial.print("pm2_5: ");
  Serial.print(pm25);
  Serial.print(" pm10: ");
  Serial.print(pm10);
  Serial.print("\r");

  String json = String("") + "{" +
    "\"device_timestamp\": \"" + ts + "\"," +
    "\"device_name\": \"iot_test_device\"," +
    "\"sensor_model\": \"sds011\"," +
    "\"sensordatavalues\": [" +
      "{\"value_type\": \"PM2_5\"," +
      "\"value\": " + pm25 + "},"
      "{\"value_type\": \"PM10\"," +
      "\"value\": " + pm10 + "}"
    + "]"
  "}";

  mqtt_client.publish(sensors_topic, json.c_str());

  free(ts);
  free(buf);
}

void loop() {
  mqtt_handle();

  PmResult pm = sds.readPm();

  if (pm.isOk()) {
    publish_dust(pm.pm25, pm.pm10);

    //mqtt_client.publish(, "hello");
  } else {
    // notice that loop delay is set to 0.5s and some reads are not available
    Serial.print("Could not read values from sensor, reason: ");
    Serial.println(pm.statusToString());
  }

  delay(500);
}
