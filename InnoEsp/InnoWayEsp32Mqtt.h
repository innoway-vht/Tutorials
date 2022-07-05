#ifndef InnoWayESP32MQTT_H
#define InnoWayESP32MQTT_H
#include <PubSubClient.h>
#include <WiFi.h>

#include "InnoConstants.h"
#include "InnoTypes.h"

class InnoWay {
private:
    WiFiClient _clientTcpInno;
    PubSubClient _clientMqttInno = PubSubClient(_clientTcpInno);
    bool _enableLog = false;
#ifdef  MEM_ALLOCATE
    char* _token;
    char* _deviceID;
    char* _clientName;
    char* _broker;
#else
    char _token[100];
    char _deviceID[100];
    char _clientName[100];
    char _broker[100];
#endif
    uint16_t _brokerPort;
    void _floatToChar(char* strValue, float value);
    void _builder(const char* token, const char * deviceID, const char* clientName, const char* broker, const int brokerPort);

public:
    InnoWay(const char* token, const char* deviceID);
    InnoWay(const char* token, const char* deviceID,const char* clientName);
    InnoWay(const char* token, const char* deviceID,const char* clientName, const char* broker);
    InnoWay(const char* token, const char* deviceID,const char* clientName, const char* broker, const int brokerPort);
    ~InnoWay();
    bool connected();
    bool connect();
    bool connect(const char *id, const char *user, const char *pass);
    bool connect(const char *id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
    bool connect(const char *id, const char *user, const char *pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
    bool connect(const char *id, const char *user, const char *pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession);
    void disconnect();
    bool loop();
    void reconnect();
    void setCallback(void (*callback)(char*, uint8_t*, unsigned int));
    void setLog(bool enableLog);
    void connectToWifi(const char* ssid, const char* pass);  

    void setup();
    bool subscribe(const char* topic);
    bool publish(const char* topic, const char* payload);
    bool publish(const char* topic, const char* payload, boolean retained);
};

#endif
