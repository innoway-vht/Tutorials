/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/

#include <WiFi.h>
#include "PubSubClient.h"
#include <Wire.h>
#include "AM2320.h"
AM2320 sensor(&Wire);

// Replace the next variables with your SSID/Password combination
const char* ssid = "IoT_1";
const char* password = "112345678990a@";

const char* token_id = "NlcxDqZeVNve0thCykGY0dboGTNRXfwx";
const char* device_id = "f183a064-5b18-4a14-af1b-4344946a5e31";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "mqtt.innoway.vn";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//uncomment the following lines if you're using SPI
/*#include <SPI.h>
  #define BME_SCK 18
  #define BME_MISO 19
  #define BME_MOSI 23
  #define BME_CS 5*/

// LED Pin
const int ledPin = 4;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
}

int read_AM2320(float *humi, float *temp) {
  Serial.println("\r\n Read sensor");
  int value = sensor.Read();
  switch (value) {
    case 2:
      Serial.println(" CRC failed");
      break;
    case 1:
      Serial.println(" Sensor offline");
      break;
    case 0:
      Serial.print(" Read success ");
      Serial.print(" Humidity = ");
      Serial.print(sensor.Humidity);
      Serial.print("%            ");
      *humi = sensor.Humidity;
      if (*humi < 0) *humi = 0;
      if (*humi > 100) *humi = 100;
      Serial.print(" Temperature = ");
      Serial.print(sensor.cTemp);
      Serial.print("*C");
      *temp = sensor.cTemp;
      Serial.println();
      break;
    default:
      Serial.print("Unknown this: ");
      Serial.println(value);
      break;
  }
  return value;
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
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
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("batky", "batky", token_id)) {
      Serial.println("connected");
//      // Subscribe
//      client.subscribe(topic_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
float humidity, temperature;
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    read_AM2320(&humidity, &temperature);

    char humString[128];
    char topic_pub[128];
    sprintf((char *)humString, "{\n\t\"curr_hum\":\"%.2f\",\n\t\"cn\":\"Wifi\"}", humidity);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    sprintf(topic_pub, "messages/%s/humidity", device_id);
    client.publish(topic_pub, humString);
    // Convert the value to a char array
  }
}
