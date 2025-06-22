#include <WiFi.h>

const char* ssid = "Xiaomi 11 Lite 5G NE";
const char* password = "Zxc008800";

void init_wifi(){
  WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void setup(){
    Serial.begin(115200);
    delay(1000);

    init_wifi();
}

void loop(){}