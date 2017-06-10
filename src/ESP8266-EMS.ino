/*
  Projet ESP8266 Enviro Monitor Station
  Copyright (C) 2017 by Leon

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <DHT.h>
#include <NewPing.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include <SoftwareSerial.h>  //软件模拟第二个串口

#define WIFI_SSID        "wifi_ssid"
#define WIFI_PASSWORD    "wifi_pass"

#define MQTT_SERVER       "10.25.73.254"
#define MQTT_PORT         1883                      // use 8883 for SSL
#define MQTT_USER         "pi"                      //user for Mosquitto
#define MQTT_PASSWORD     "raspberry"               //passord for Mosquitto

#define temperature_topic "sensor/temperature"      //Topic Temperature
#define humidity_topic    "sensor/humidity"         //Topic Humidity
#define luminosity_topic  "sensor/luminosity"       //Topic Luminosity
//#define distance_topic    "sensor/distance"         //Topic Distance
#define gas_topic         "sensor/gas"              //Topic gas
#define ch2o_topic        "sensor/ch2o"             //Topic ch2o
#define pm1_topic         "sensor/pm1"              //Topic pm1
#define pm2_5_topic        "sensor/pm25"             //Topic pm2.5
#define pm10_topic        "sensor/pm10"             //Topic pm10
#define noise_topic       "sensor/noise"

char message_buff[100];
bool debug = true;           //Affiche sur la console si True

float temperature = 0;
float humidity = 0;
float light = 0;
int noise = 0;
float gas = 0;

#define ADC_COUNTS              1023
#define NOISE_READING_WINDOW    10
#define NOISE_BUFFER_SIZE       20

unsigned int noise_buffer_sum = 0;

//#define MAX_DISTANCE            300      // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.


#define RELAY_PIN D2
#define MIC_PIN D3            // Pin which is connected to the Noise SR-04
#define RGB_PIN D4            // Pin which is connected to the WS2812
#define LDR_PIN D5            // Pin which is connected to the LDR
#define DHT_PIN D6            // Pin which is connected to the DHT sensor.
const byte rxPinCh2o = D7;
const byte txPinCh2o = D8;
SoftwareSerial mySerialCh2o (rxPinCh2o, txPinCh2o);

int lightCount = 24;

// Uncomment the sensor type that you have (comment the other if applicable)
//#define DHTTYPE     DHT11    // DHT 11
#define DHTTYPE    DHT22       // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

//Création des objets
DHT dht(DHT_PIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(lightCount, RGB_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  mySerialCh2o.begin(9600);

  // Initialize physical pins on the ESP8266
  pinMode(LDR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(DHT_PIN, INPUT);
  pinMode(MIC_PIN, INPUT);
  pinMode(RGB_PIN, OUTPUT);

  setup_wifi();                                 //setup connection of wifi
  client.setServer(MQTT_SERVER, MQTT_PORT);     //Configuration for connecting to server MQTT
  client.setCallback(callback);                 //La fonction de callback qui est executée à chaque réception de message

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  dht.begin();
  strip.begin();
  //strip.setBrightness(30); //adjust brightness here
  strip.setBrightness(0);
  strip.show(); // Initialize all pixels to 'off'

}

//Connecfion WiFi
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("=> IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
}


//Reconnexion
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, Error: ");
      Serial.print(client.state());
      Serial.println(" Retrying MQTT connection in 5 seconds...");
      delay(5000);  // Wait 5 seconds before retrying
    }
  }
}

void loop() {
  ArduinoOTA.handle();
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  noiseLoop();
  light = getLight();

  noiseLoop();
  temperature = getTemperature();

  noiseLoop();
  humidity = getHumidity();

  // noiseLoop();
  //  getNoise();

  //  noiseLoop();
  //  gas = getGas();

  noiseLoop();
  run_ws2812();

  noiseLoop();
  check_ch2o();

  noiseLoop();
  check_pm();

  print_degug();
  client.publish(temperature_topic, String(temperature).c_str(), true);          //sent temperature message to MQTT
  client.publish(humidity_topic, String(humidity).c_str(), true);            //sent humidity message to MQTT
  client.publish(luminosity_topic, String(light).c_str(), true);             //sent luminosity message to MQTT
  //  client.publish(gas_topic, String(gas).c_str(), true);                      //sent gas message to MQTT
  delay(1000 * 60);
  noiseLoop();
}

int getLight() {
  return map(analogRead(LDR_PIN), 0, ADC_COUNTS, 100, 0);
}

// 0.5V/0.1mg/m3

float getTemperature() {
  return dht.readTemperature();
}

int getHumidity() {
  return dht.readHumidity();
}

/*
  int getNoise() {
  int value = 0;
  value = noise_buffer_sum / NOISE_BUFFER_SIZE;
  return value;
  }

  int getGas()  {
  if (digitalRead(MQ_D) == HIGH)
  {
    gas = analogRead(MQ_A);
    return gas;
  }
  else
  {
    return 0;
  }
  }
*/
void print_degug()
{
  if ( debug ) {
    Serial.print(" Humidite : ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print(" Temperature : ");
    Serial.print(temperature);
    Serial.print(" Luminosity: ");
    Serial.print(light);
    Serial.println(" %");
    Serial.print(" Noise: ");
    Serial.print(noise);
    Serial.println(" dB");
    //    Serial.print(" Gas Alarm: ");
    //    Serial.print(gas);
    //    Serial.println(" mg/m3");
  }
}

// Start after receiving the message
void callback(char* topic, byte* payload, unsigned int length) {
  int i = 0;
  if ( debug ) {
    Serial.println("Message recu =>  topic: " + String(topic));
    Serial.print(" | longueur: " + String(length, DEC));
  }
  // create character buffer with ending null terminator (string)
  for (i = 0; i < length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';

  String msgString = String(message_buff);
  if ( debug ) {
    Serial.println("Payload: " + msgString);
  }
}
