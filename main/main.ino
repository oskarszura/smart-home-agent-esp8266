#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
 
ESP8266WebServer server(80);
WiFiServer tcpServer(81);
const char* hsSsid = "SmaHotSpot";
const char* hsPass = "12345678";
unsigned int retries = 200;

String cmdString;
String readString;
String tcpResponse;
WiFiClient client;

char prevToken = 0;
char channel = '0';
 
void setup() {
  EEPROM.begin(512);
  Serial.begin(9600);

  char* ssid;
  char* pass;

  getWiFiCredentials(&ssid, &pass); 
  
  WiFi.begin(ssid, pass);
  tcpServer.begin();

  while (WiFi.status() != WL_CONNECTED && retries > 0) {
    delay(500);
    retries--;
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hsSsid, hsPass);
  }
 
  server.on("/", handleRootPath);
  server.on("/api", handleApiPath);
  server.on("/config", handleConfigPath);
  
  server.begin();        
}
 
void loop() {
  server.handleClient();

  if (!client || client && !client.connected()) {
    client = tcpServer.available();
  }

  if (client && client.connected()) {
    if (client.available() > 0) {
        char c = client.read();
        Serial.write(c);
    }
  }

  if (Serial.available()) {
    char c = Serial.read();

    if (c == '[') {
      prevToken = c;
    } 
    else if (prevToken == '[' && c != ':') {
      channel = c;
    } 
    else if (prevToken == '[' && c == ':') {
      prevToken = c;
    }
    else if (prevToken == ':' && c != ']') {
      if (channel == '0') {
        cmdString += c;
      }
      else if (channel == '1') {
        tcpResponse += c;
      }
      else if (channel == '2') {
        readString += c;
      }
    }
    else if (c == ']') {
      if (channel == '0') {
        if (cmdString == "disconnect") {
          client.stop();
        }
        
        cmdString = "";
      }
      else if (channel == '1') {
        client.print(tcpResponse);
        tcpResponse = "";
      }
      channel = '0';
      prevToken = 0;
    }
  }
}

void handleRootPath() {
  server.send(200, "text/plain", "smart-evolution container v0.5.0");
}

void handleApiPath() {
  server.send(200, "text/plain", readString);
  readString = "";
}

void handleConfigPath() {
  const String ssid = server.arg("ssid");
  const String pass = server.arg("pass");

  if (ssid != "" && pass != "") {
    for(int n = 0; n <= ssid.length() + pass.length() + 1; n++) {
      char sign = ' ';
  
      if (n < ssid.length()) {
        sign = ssid[n];
      } else if (n == ssid.length()) {
        sign = ':';
      } else if (n > ssid.length() && n <= ssid.length() + pass.length()) {
        sign = pass[n - (ssid.length() + 1)];
      } else if (n == ssid.length() + pass.length() + 1) {
        sign = ';';
      }
  
      EEPROM.write(n, sign);
    }
    EEPROM.commit();
    server.send(200, "text/plain", "Persisted [" + ssid + "|" + pass + "]");
  } else {
    char* ssid;
    char* pass;
    getWiFiCredentials(&ssid, &pass);

    String output = "Container configuration\n"
      "ssid = [" + String(ssid) + "]\n"
      "pass = [" + String(pass) + "]\n"
      "mac addr = [" + WiFi.macAddress() + "]\n"
      "ip addr = [" + WiFi.localIP() + "]\n";
  
    server.send(200, "text/plain", output);
  }
}

void getWiFiCredentials(char** ssid, char** pass) {
  char currChar;
  char charBuff[100];
  unsigned int totalLen = 0;
  unsigned int ssidLen = 0;
  unsigned int passLen = 0;
  unsigned int separatorIndex = 0;
  
  while(totalLen < 100) {
    currChar = char(EEPROM.read(totalLen));

    if (currChar == ';') {
      break;
    }
    
    charBuff[totalLen] = currChar;

    if (currChar == ':') {
      separatorIndex = totalLen;
    }
    
    totalLen++;
  }

  *ssid = (char*)malloc(sizeof(char) * (separatorIndex + 1));
  *pass = (char*)malloc(sizeof(char) * (totalLen - separatorIndex + 1));

  for(int i = 0; i < separatorIndex; i++) {
    (*ssid)[i] = charBuff[i];
  }
  (*ssid)[separatorIndex] = '\0';
  for(int i = separatorIndex + 1; i < totalLen; i++) {
    (*pass)[i - (separatorIndex + 1)] = charBuff[i];
  }
  (*pass)[totalLen] = '\0';
}

