#ifndef _INNOWAY_H_
#define _INNOWAY_H_

#include <stdio.h>
#include <stdint.h>
#include <Arduino.h>
#include "InnoTool.h"

#define TIMER_ATC_PERIOD 100      //time ms
#define DAM_BUF_TX       512
#define RESET_UART
//#define POLLING_UART0      
#define POLLING_UART2      

#define IN_OK              1
#define IN_ERROR          -1
#define IN_TIMEOUT        -2
#define IN_WHILE_TIMEOUT  -3

typedef struct{
  char CMD[DAM_BUF_TX];
  uint32_t lenCMD;
  char ExpectResponseFromATC[20];
  uint32_t TimeoutATC;
  uint32_t CurrentTimeoutATC;
  uint8_t RetryCountATC;
} ATCommand_t;

//const char INNOWAY_BROKER[] = "116.101.122.190";
const char INNOWAY_BROKER[] = "125.212.248.229";
//const char INNOWAY_BROKER[] = "171.244.133.251";
const int  INNOWAY_MQTT_PORT = 1883;
#define MAX_BUFFER_LEN  512
#define MEM_ALLOCATE

class InnoWaySimcom {
private:
#ifdef  MEM_ALLOCATE
    char* _token;
    char* _deviceID;
    char* _clientName;
    char* _broker;
    char* _buffer;
#else
    char _token[100];
    char _deviceID[100];
    char _clientName[100];
    char _broker[100];
    char _buffer[MAX_BUFFER_LEN];
#endif
    uint16_t _brokerPort;
    bool _enableLog = false;
    ATCommand_t SIMCOM_ATCommand;
    void (*callback)(char*, char*, unsigned int);
    void _floatToChar(char* strValue, float value);
    void _builder(const char* token, const char * deviceID, const char* clientName, const char* broker, const int brokerPort);

public:
    InnoWaySimcom(const char* token, const char* deviceID);
    InnoWaySimcom(const char* token, const char* deviceID,const char* clientName);
    InnoWaySimcom(const char* token, const char* deviceID,const char* clientName, const char* broker);
    InnoWaySimcom(const char* token, const char* deviceID,const char* clientName, const char* broker, const int brokerPort);
    ~InnoWaySimcom();

    void setup();
    void setLog(bool enableLog);
    void setCallback(void (*callback)(char*, char*, unsigned int));
    int checkRetry(void);
    int innoHandler(char * data);
    int pollingUart();
    void SendATCommand();
    void ATC_SendATCommand(const char *Command, char *ExpectResponse, uint32_t Timeout, uint8_t RetryCount);

    int innoPolling(uint32_t timeout);
    int innoConnect();
    int innoConnect(char * device_id,char *user, char * password,  uint32_t keep_alive, uint8_t clean_session);
    int innoPublish(char * In_topic, char *In_data);
    int innoPublish(char * In_topic, char *In_data, uint8_t qos, uint8_t retain );
    int innoSubscribe(char *topic, uint8_t qos);
    int innoUnsub(char *topic);
    int innoDisconnect();
    void loop();
};
#endif
