/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <WiFi.h>
#include "PubSubClient.h"
#include <Wire.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "IoT_1";
const char* password = "112345678990a@";

// MQTT Config
const char* token_id = "NlcxDqZeVNve0thCykGY0dboGTNRXfwx";
const char* user_mqtt = "Demo";
const char* device_id = "0b66b3f8-5e80-4cc4-8ce1-019455d184cd";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "mqtt.innoway.vn";
const char* client_id = "batky1";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
char topic_sub[128];
//uncomment the following lines if you're using SPI
/*#include <SPI.h>
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5*/

// LED Pin
const int GPIO_relay = 4;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(GPIO_relay, OUTPUT);
  sprintf(topic_sub, "messages/%s/control_pump", device_id);
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
  char mesResult[128];
  char mesSub[128];
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Get the value of "status" in payload, as it stays between 3rd and 4th comma "  
  filter_comma(messageTemp, 3, 4, (char *)mesResult);
  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (!strcmp(topic, topic_sub)) {
    if(!strcmp(mesResult, "on")){
      Serial.print("Changing output to ");
      Serial.println("on");
      digitalWrite(GPIO_relay, HIGH);
    }
    else if(!strcmp(mesResult, "off")){
      Serial.print("Changing output to ");
      Serial.println("off");
      digitalWrite(GPIO_relay, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(client_id,user_mqtt, token_id)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(topic_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
int filter_comma(String respond_data, int begin, int end, char *output)
{
    memset(output, 0, strlen(output));
    int count_filter = 0;
    int lim = 0;
    int start = 0;
    int finish = 0;
    int i;
    for (i = 0; i < respond_data.length(); i++) {
        if ( respond_data[i] == '\"') {
            count_filter ++;
            if (count_filter == begin)          start = i+1;
            if (count_filter == end)            finish = i;
        }
    }
    lim = finish - start;
    for (i = 0; i < lim; i++) {
        output[i] = respond_data[start];
        start ++;
    }
    output[i] = 0;
    return 0;
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
  }
}
