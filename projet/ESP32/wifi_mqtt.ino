#include <WiFi.h> 
#include <PubSubClient.h>

/* wifi  */
const char* ssid = "Xiaomi 11 Lite 5G NE";
const char* password = "Zxc008800";

/* mqtt */
const char mqtt_broker[] = "192.168.185.112";
const char *topic = "topic";
const int mqtt_port = 1883;


WiFiClient espClient;
PubSubClient client(espClient);

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

    /* mqtt dirty */

   client.setServer(mqtt_broker, mqtt_port);
  //client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (client.connect( client_id.c_str())) {
          Serial.println("Public EMQX MQTT broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      } 
      delay(2000);
  }

  // publish and subscribe
  client.publish(topic, "Hi, I'm ESP32 ^^");



}

void loop(){
  client.loop();
}