#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> 
 
ESP8266WebServer server(80);

String readString;

const String agentId = "SHA-1";
 
void setup() {
  Serial.begin(9600);
  WiFi.begin("", "");
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
 
  server.on("/", handleRootPath);   
  server.on("/api", handleApiPath);
  
  server.begin();                   
}
 
void loop() {
  if (Serial.available()) {
    char c = Serial.read();  
    readString += c;
  }
 
  server.handleClient();
}

void handleRootPath() {
  server.send(200, "text/plain", "Agent Id: " + agentId + " IP address: " + WiFi.localIP());
}

void handleApiPath() {
  server.send(200, "text/plain", readString);
  readString = "";
}

