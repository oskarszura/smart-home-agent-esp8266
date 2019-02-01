#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
 
ESP8266WebServer server(80);
const char* ssid = "ESPWebServer";
const char* password = "12345678";

String readString;
 
void setup() {
  Serial.begin(9600);
  //WiFi.begin("", "");
 
  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(500);
  //}
  WiFi.mode(WIFI_AP);           //Only Access point
  WiFi.softAP(ssid, password);  //Start HOTspot removing password will disable security
 
  IPAddress myIP = WiFi.softAPIP();
 
  server.on("/", handleRootPath);   
  server.on("/api", handleApiPath);
  server.on("/config", handleConfigPath);
  
  server.begin();        

  EEPROM.begin(512);
}
 
void loop() {
  if (Serial.available()) {
    char c = Serial.read();  
    readString += c;
  }
 
  server.handleClient();
}

void handleRootPath() {
  server.send(200, "text/plain", "Hardware: agent-type-1");
}

void handleApiPath() {
  server.send(200, "text/plain", readString);
  readString = "";
}

void handleConfigPath() {
  const String ssid = server.arg("ssid");
  const String pass = server.arg("pass");
  const int maxLen = 20;

  if (ssid != "" && pass != "") {
    for(int n = 0; n < maxLen; n++) {
      char ssidChar = ' ';
      char passChar = ' ';
  
      if (n < ssid.length()) {
           ssidChar = ssid[n];
      }
      if (n < pass.length()) {
           passChar = pass[n];
      }    
  
      EEPROM.write(n, ssidChar); 
      EEPROM.write(maxLen + n, passChar); 
    }
    
    EEPROM.commit();
    server.send(200, "text/plain", "Persisted");
  } else {
    String ssid = "";
    String pass = "";
    
    for(int n = 0; n < maxLen; n++) {
      ssid += char(EEPROM.read(n)); 
      pass += char(EEPROM.read(maxLen + n)); 
    }

    ssid.trim();
    pass.trim();
    
    server.send(200, "text/plain", "[" + ssid + "|" + pass + "]");
  }
}
