//Wifi
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define USE_SERIAL Serial

#define SECRET_SSID "Brio_1F_Back"      // replace MySSID with your WiFi network name
#define SECRET_PASS "0970087977"  // replace MyPassword with your WiFi password
#define WIFI_CHANNEL 6

char ssid[] = SECRET_SSID;      // your network SSID (name)
char password[] = SECRET_PASS;  // your network password

int PIR = 25;

//MQTT Broker
#include <PubSubClient.h>
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "4b1g0176/PIRsensor";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {

  // put your setup code here, to run once:

  Serial.begin(115200);
  while (!Serial) { delay(100); }
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    //for sensor
    pinMode(PIR, INPUT);
    digitalWrite(PIR,LOW);
    // MQTT Broker

    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);

    while (!client.connected()) {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Public emqx mqtt broker connected");
        delay(500);
      } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(500);
      }

      // publish and subscribe
      client.publish(topic, "Sensor on");
      client.subscribe(topic);
    }
}

  void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
  }

  int counter = 0;
  int PIR_move = 0;


  void loop() {
    //MQTT
    client.loop();

    //for sensor
    int PIR_move = digitalRead(PIR);


    if (PIR_move == 1) {
      client.publish(topic, "N");
    } else {
      client.publish(topic, "F");
    }
    delay(1000);
  }