#include <ESP8266WebServer.h>

static ESP8266WebServer server(80);

static void handle_root() {
  char* page_code = "<form action='CONFIGURE' method='POST'>"
                    "<input type='text' name='ssid' value='ssid'>"
                    "<input type='text' name='pass' value='password'>"
                    "<input type='submit'>"
                    "</form>";

  server.send(200, "text/html", page_code);
}

static void handle_success() {;
  server.send(200, "text/html", "<h1>Success! Switching to MQTT mode.</h1>");
}

static void handle_fail() {
  server.send(200, "text/html", "<h1>Failed to connect to Wi-Fi; please check ssid and password.</h1><br><a href='/'>Back</a>");
}

static void handle_configure() {
  Serial.print("Args:\n");

  String ssid;
  String pass;

  for (uint8_t i = 0; i < server.args(); i++) {
    String k = server.argName(i);
    String v = server.arg(i);

    if (k == "ssid") ssid = String(v);
    if (k == "pass") pass = String(v);

    Serial.print("Arg ");
    Serial.print(k);
    Serial.print(" = ");
    Serial.print(v);
    Serial.print("\n");
  }

  if (ssid and pass) {
    Serial.print("Credentials obtained:\n");
    Serial.println(ssid);
    Serial.println(pass);
  }

  bool status = start_client_mode(ssid.c_str(), pass.c_str());

  if (status) {
    server.sendHeader("Location", "/success");
    server.send(303);
    modeswitch_mqtt();
  } else {
    server.sendHeader("Location", "/fail");
    server.send(303);
  }
}

static void handle_not_found() {
  server.send(404, "text/html", "404 check url");
}

void init_server() {
  server.on("/", HTTP_GET, handle_root);
  server.on("/success", HTTP_GET, handle_success);
  server.on("/fail", HTTP_GET, handle_fail);
  server.on("/CONFIGURE", HTTP_POST, handle_configure);
  server.onNotFound(handle_not_found);
  server.begin();

  Serial.print("Server is up on port 80\n");
}

void stop_server() {
  server.stop();
}

void server_handle() {
  server.handleClient();
}
