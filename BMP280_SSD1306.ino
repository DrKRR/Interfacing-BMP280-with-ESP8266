//File Name:- EMQX_BMP280_OLED_1
//Hardware is same to that of BM280 Program
//This program displays Temp,Press and Altitude on OLED
//Displays Temp., Press.,on EMQ X cloud client


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET LED_BUILTIN
#define Sea_Level_Press_hPa (1013.25)

// WiFi
const char* ssid = "";  // Enter WiFi SSID here
const char* password = "";  //Enter WiFi Password here
// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "NodeMCU_BMP_Test";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int   mqtt_port = 1883;
const char *temperature_topic = "BMP_Temperature";
const char *pressure_topic = "BMP_Pressure";

WiFiClient NodeMCUClient;
PubSubClient client(NodeMCUClient);

Adafruit_BMP280 bmp;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);
  delay(100);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  delay(2000);

  bool status = bmp.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
  delay(500);

  Serial.println("");//Flushout Garbage
  // connecting to a WiFi network
  Serial.println("connecting to:");
  Serial.println(ssid);
  WiFi.begin(ssid, password); //connect to your Local WiFi Network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected!");
  Serial.print("Got IP:");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
}


//---------------
void reconnect() {
  // Loop until we're reconnected
  //client.setServer(mqtt_broker, mqtt_port);
  //client.setCallback(callback);
  while (!client.connected()) {
    Serial.print("Attempting EMQ X_MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("NodeMCUClient", mqtt_username, mqtt_password)) {
      Serial.println("Public emqx MQTT broker connected");
    } else {
      Serial.print("failed, rc= ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  client.publish(topic, "hello emqx");
  client.subscribe(topic);
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

long lastMsg = 0;
float temp = 0.0;
float pres = 0.0;
float diff = 1.0;


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    float newTemp = bmp.readTemperature();
    float newPres = bmp.readPressure() / 100.0F;
    if (checkBound(newTemp, temp, diff)) {
      temp = newTemp;
      Serial.print("New temperature:");
      Serial.println(String(temp).c_str());
      client.publish(temperature_topic, String(temp).c_str(), true);
      /*display.setTextSize(1);
        display.setCursor(0,0);
        display.print("Temperature: ");
        display.setTextSize(1);
        display.setCursor(0,10);
        display.print(String(temp).c_str());
        display.print(" ");
        display.setTextSize(1);
        display.cp437(true);
        display.write(167);
        display.setTextSize(1);
        display.print(" C");
        delay(1000);*/
    }
    if (checkBound(newPres, pres, diff)) {
      pres = newPres;
      Serial.print("New pressure:");
      Serial.println(String(pres).c_str());
      client.publish(pressure_topic, String(pres).c_str(), true);
      /*display.setTextSize(1);
        display.setCursor(0, 20);
        display.print("Pressure: ");
        display.setTextSize(1);
        display.setCursor(0, 30);
        display.print(String(pres).c_str());
        display.print(" hPa");
        display.display();
        delay(1000); */
    }
    // display temperature
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Temperature: ");
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.print(String(bmp.readTemperature()));
    display.print(" ");
    display.setTextSize(1);
    display.cp437(true);
    display.write(167);
    display.setTextSize(1);
    display.print("C");
    delay(1000);

    // display presssure
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("Pressure: ");
    display.setTextSize(1);
    display.setCursor(0, 30);
    display.print(String(bmp.readPressure() / 100.0F));
    display.print(" hPa");
    display.display();
    delay(1000);



    //display altitude
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.print("Altiitude: ");
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print(String(bmp.readAltitude(Sea_Level_Press_hPa)));
    display.print(" m");
    display.display();
    delay(1000);

    display.clearDisplay();
    //client.loop();
  }
  delay(1000);
}

void callback(char *topic1, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic1);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}