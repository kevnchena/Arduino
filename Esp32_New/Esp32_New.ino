#include <Adafruit_GFX.h>

//Oled
#include <SPI.h>
#include <U8g2lib.h>
#include <Wire.h>
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/22, /* data=*/21, /* reset=*/U8X8_PIN_NONE);
#define imgWidth 128
#define imgHeight 48

//Wifi
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define USE_SERIAL Serial

#define SECRET_SSID "K_Room"      // replace MySSID with your WiFi network name
#define SECRET_PASS "0970087977"  // replace MyPassword with your WiFi password
#define WIFI_CHANNEL 6

char ssid[] = SECRET_SSID;      // your network SSID (name)
char password[] = SECRET_PASS;  // your network password

int PIR = 5;
int PIR_move = 0;

//Line
#include <TridentTD_LineNotify.h>
#define LINE_TOKEN "WkBhVhy7Jzpw83cWd0LNjg4nQgjkHQ0HGWYTLzTmREh"

//aAdafruit IO
#include "config.h"
int ada_value;

#define FEED_OWNER "z6256875"
AdafruitIO_Feed *sharedFeed = io.feed("voice", FEED_OWNER);

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


  //oled
  u8g2.begin();



  // put your setup code here, to run once:

  Serial.begin(115200);
  while (!Serial) { delay(100); }
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_HelvetiPixelOutline_te);
    u8g2.setCursor(20, 40);
    u8g2.print(ssid);
  } while (u8g2.nextPage());

  WiFi.begin(ssid, password, 6);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    u8g2.firstPage();
    do {
      u8g2.setCursor(20, 40);
      u8g2.print("connecting");
    } while (u8g2.nextPage());
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //for sensor
  pinMode(PIR, INPUT);

  //relay
  pinMode(14, OUTPUT);

  //adafruit io
  io.connect();
  sharedFeed->onMessage(handleMessage);

  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println(io.statusText());
  sharedFeed->get();

  // MQTT Broker

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {

    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());

    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {

      Serial.println("Public emqx mqtt broker connected");

      u8g2.firstPage();
      do {
        u8g2.setCursor(20, 40);
        u8g2.print("mqtt broker");
        u8g2.setCursor(20, 50);
        u8g2.print("connected");

      } while (u8g2.nextPage());
      delay(500);
    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());

      u8g2.firstPage();
      do {
        u8g2.setCursor(20, 40);
        u8g2.print("mqtt broker");
        u8g2.setCursor(20, 50);
        u8g2.print("failed");
      } while (u8g2.nextPage());

      delay(2000);
    }

    // publish and subscribe
    client.publish(topic, "Hi EMQX I'm ESP32 ^^");
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
  if ((char)payload[0] == 'N') PIR_move = 1;
  else if ((char)payload[0] == 'F')  PIR_move = 0;

  Serial.println();
  Serial.println("-----------------------");
}


int counter = 0;


void loop() {
  //adafruit io
  //shuold always at the top
  io.run();

  //MQTT
  client.loop();

  //esp32 status
  if (io.status() < AIO_CONNECTED) {

    u8g2.firstPage();
    do {
      u8g2.setCursor(20, 30);
      u8g2.print("Line lost");

    } while (u8g2.nextPage());
  }

  if (WiFi.status() != WL_CONNECTED) {
///oled print
    u8g2.firstPage();
    do {
      u8g2.setCursor(20, 30);
      u8g2.print("Wifi lost");

    } while (u8g2.nextPage());
    ///oled print
    client.publish(topic, "Hi EMQX I'm ESP32 ^^");
    client.subscribe(topic);
  }

  //Linebot message
  if (ada_value == 100) digitalWrite(14, HIGH);

  //for sensor

  if (PIR_move == 1 && ada_value == 0) {
    counter++;
    //如果沒有遙控開再我送給我Line警告
    digitalWrite(14, HIGH);

    ///oled print
    u8g2.firstPage();
    do {
      u8g2.setCursor(30, 40);
      u8g2.print("Sensor on");

    } while (u8g2.nextPage());
    ///

  } else if(PIR_move == 0 && ada_value == 0) {
    digitalWrite(14, LOW);
  }


  Serial.print(PIR_move);


  //Line notify
  if (counter > 10 && ada_value == 0) {
    LINE.setToken(LINE_TOKEN);
    LINE.notify("注意門口!!");
    counter = 0;
  }
  delay(1000);
}

void handleMessage(AdafruitIO_Data *data) {

  Serial.print("received <- ");
  Serial.println(data->toInt());
  ada_value = data->toInt();

  u8g2.firstPage();
  do {
    u8g2.setCursor(20, 30);
    u8g2.print("remoted");
  } while (u8g2.nextPage());
}