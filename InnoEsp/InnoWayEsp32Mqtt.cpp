#include "InnoWayEsp32Mqtt.h"

/**************************************************************************
 * Overloaded constructors
 ***************************************************************************/

InnoWay::InnoWay(const char* token, const char* deviceID) {
    _builder(token, deviceID, deviceID, INNOWAY_BROKER, INNOWAY_MQTT_PORT);
}

InnoWay::InnoWay(const char* token, const char* deviceID,const char* clientName) {
    _builder(token, deviceID, clientName, INNOWAY_BROKER, INNOWAY_MQTT_PORT);
}

InnoWay::InnoWay(const char* token, const char* deviceID,const char* clientName, const char* broker) {
    _builder(token, deviceID, clientName, broker, INNOWAY_MQTT_PORT);
}

InnoWay::InnoWay(const char* token, const char* deviceID,const char* clientName, const char* broker, const int brokerPort) {
    _builder(token, deviceID, clientName, broker, brokerPort);
}

/**************************************************************************
 * Overloaded destructor
 ***************************************************************************/

InnoWay::~InnoWay() {
#ifdef  MEM_ALLOCATE
    free(_token);
    free(_deviceID);
    free(_clientName);
    free(_broker);
#endif
    _clientMqttInno.disconnect();
}

void InnoWay::_builder(const char* token, const char * deviceID, const char* clientName, const char* broker, const int brokerPort) {
#ifdef  MEM_ALLOCATE
    _token = strdup(token);
    _deviceID = strdup(deviceID);    
    _clientName = strdup(clientName);
    _broker = strdup(broker);
    _brokerPort = brokerPort;
#else
    strcpy(_token, token);
    strcpy(_deviceID, deviceID);
    strcpy(_clientName, clientName);
    strcpy(_broker, broker);
    _brokerPort = brokerPort;
#endif
}

void InnoWay::setup() {
    Serial.print("token:");
    Serial.println(_token);
    Serial.print("deviceID:");
    Serial.println(_deviceID);
    Serial.print("clientName:");
    Serial.println(_clientName);
    Serial.print("broker:");
    Serial.println(_broker);
    Serial.print("brokerPort:");
    Serial.println(_brokerPort);
    _clientMqttInno.setServer(_broker, _brokerPort);
}

/***************************************************************************
FUNCTIONS TO SEND/RETRIEVE DATA
***************************************************************************/

/**
 * Retrieves data from InnoWay by subscribing to a custom topic
 * @arg topic [Mandatory] topic to be subscribed
 */
bool InnoWay::subscribe(const char* topic) { return _clientMqttInno.subscribe(topic); }

/**
 * Sends data to InnoWay
 * @arg topic [Mandatory] topic to be published data
 * @arg payload [Mandatory] data need to publish 
 * @arg retained [Optional] MQTT retained
 */
bool InnoWay::publish(const char* topic, const char* payload) {
    return _clientMqttInno.publish(topic, payload, false);
}
bool InnoWay::publish(const char* topic, const char* payload, boolean retained) {
    return _clientMqttInno.publish(topic, payload, retained);
}

/***************************************************************************
FUNCTIONS TO MANAGE SOCKET CONNECTION
***************************************************************************/

/**
 * returns true if the ESP32 is connected to the broker
 */
bool InnoWay::connected() { return _clientMqttInno.connected(); };

/**
 * Overloaded connect() methods.
 * Connects to the broker using a custom username and password
 * @arg id [Optional] Unique MQTT client id
 * @arg user [Optional] MQTT username to be identified by the broker
 * @arg pass [Optional] MQTT password to be identified by the broker
 * @arg willTopic [Optional] MQTT willTopic
 * @arg willQos [Optional] MQTT willQos
 * @arg willRetain [Optional] MQTT willRetain
 * @arg willMessage [Optional] MQTT willMessage
 * @arg cleanSession [Optional] MQTT cleanSession
 */
boolean InnoWay::connect() {
    return connect(_deviceID,_deviceID,_token,0,0,0,0,1);
}
boolean InnoWay::connect(const char *id, const char *user, const char *pass) {
    return connect(id,user,pass,0,0,0,0,1);
}
boolean InnoWay::connect(const char *id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage) {
    return connect(id,_deviceID,_token,willTopic,willQos,willRetain,willMessage,1);
}
boolean InnoWay::connect(const char *id, const char *user, const char *pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage) {
    return connect(id,user,pass,willTopic,willQos,willRetain,willMessage,1);
}
boolean InnoWay::connect(const char *id, const char *user, const char *pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession) {
    bool result = _clientMqttInno.connect(id,user,pass,willTopic,willQos,willRetain,willMessage,cleanSession);
    if (_enableLog) {
        Serial.println("attempting to connect again ...");
        if (!result) {
            Serial.print("failed, rc=");
            Serial.print(_clientMqttInno.state());
        }
    }
    return result;
}

/**
 * Disconnects gracefully from the broker, closing the socket
 */
void InnoWay::disconnect() { 
  _clientMqttInno.disconnect(); 
}

/**
 * Maintains the socket connection and sends periodically the keep alive command
 */
bool InnoWay::loop() { 
    return _clientMqttInno.loop(); 
}

/**
 * Attempts to reconnect to the server using as password and username the InnoWay token
 * This is a blocking function
 */
void InnoWay::reconnect() {
    while (!_clientMqttInno.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (_clientMqttInno.connect(_deviceID,_deviceID, _token)) {
            Serial.println("connected");
            break;
        } 
        else {
            Serial.print("failed, rc=");
            Serial.print(_clientMqttInno.state());
            Serial.println(" try again in 3 seconds");
            delay(3000);
        }
    }
}

/***************************************************************************
AUXILIAR FUNCTIONS
***************************************************************************/
/**
 * Sets the callback to be used to process the data incoming from the subscribed topics
 * @arg callback [Mandatory] Pointer to the callback function that will process the incoming data
 */
void InnoWay::setCallback(void (*callback)(char*, uint8_t*, unsigned int)) {
    _clientMqttInno.setCallback(callback); 
}

/**
 * Makes available the debug messages
 */
void InnoWay::setLog(bool enableLog) {
    _enableLog = enableLog;
}

/**
 * Connects to the WiFi network. This method supports just WAP and WAP2 connections
 * @arg ssid [Mandatory] SSID of the network to connect to
 * @arg pass [Mandatory] WiFi network password
 */
void InnoWay::connectToWifi(const char* ssid, const char* pass) {
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());
}

/**
 * Converts from float to char, returning the less possible length char array with the
 * float value conversion
 * @arg str_value [Mandatory] Pointer to the char array that will store the result
 * @arg value [Mandatory] float type value
 */
void InnoWay::_floatToChar(char* str_value, float value) {
    char temp_arr[20];
    sprintf(temp_arr, "%17g", value);
    uint8_t j = 0;
    uint8_t k = 0;
    while (j < 20) {
        if (temp_arr[j] != ' ') {
        str_value[k] = temp_arr[j];
        k++;
        }
        if (temp_arr[j] == '\0') {
        str_value[k] = temp_arr[j];
        break;
        }
        j++;
    }
}
