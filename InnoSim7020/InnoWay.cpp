#include "InnoWay.h"
/**************************************************************************
 * Overloaded constructors
 ***************************************************************************/

InnoWaySimcom::InnoWaySimcom(const char* token, const char* deviceID) {
    _builder(token, deviceID, deviceID, INNOWAY_BROKER, INNOWAY_MQTT_PORT);
}

InnoWaySimcom::InnoWaySimcom(const char* token, const char* deviceID,const char* clientName) {
    _builder(token, deviceID, clientName, INNOWAY_BROKER, INNOWAY_MQTT_PORT);
}
                                                                 
InnoWaySimcom::InnoWaySimcom(const char* token, const char* deviceID,const char* clientName, const char* broker) {
    _builder(token, deviceID, clientName, broker, INNOWAY_MQTT_PORT);
}

InnoWaySimcom::InnoWaySimcom(const char* token, const char* deviceID,const char* clientName, const char* broker, const int brokerPort) {
    _builder(token, deviceID, clientName, broker, brokerPort);
}

InnoWaySimcom::~InnoWaySimcom() {
#ifdef  MEM_ALLOCATE
    free(_token);
    free(_deviceID);
    free(_clientName);
    free(_broker);
    free(_buffer);
#endif
    innoDisconnect();
}

void InnoWaySimcom::_builder(const char* token, const char * deviceID, const char* clientName, const char* broker, const int brokerPort) {
#ifdef  MEM_ALLOCATE
    _token = strdup(token);
    _deviceID = strdup(deviceID);    
    _clientName = strdup(clientName);
    _broker = strdup(broker);
    _buffer = (char*)malloc(MAX_BUFFER_LEN * sizeof(char));
    _brokerPort = brokerPort;
#else
    strcpy(_token, token);
    strcpy(_deviceID, deviceID);
    strcpy(_clientName, clientName);
    strcpy(_broker, broker);
    _brokerPort = brokerPort;
#endif
}

void InnoWaySimcom::setup() {
    if (_enableLog) {
        Serial.print("print setup\r\n");
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
    }
}

void InnoWaySimcom:: setLog(bool enableLog) {
    _enableLog = enableLog;
}

void InnoWaySimcom:: setCallback(void (*callback)(char*, char*, unsigned int)) {
    this->callback = callback;  
}

//void InnoWaySimcom:: setCallback(void (*callback)(char*, uint8_t*, unsigned int)){
////    this->callback = callback;
//}

//void InnoWaySimcom:: setCallback() {
//  
//}

/*------------------------------------------------*/
int InnoWaySimcom:: checkRetry(void)
{
    if(SIMCOM_ATCommand.TimeoutATC > 0 && SIMCOM_ATCommand.CurrentTimeoutATC < SIMCOM_ATCommand.TimeoutATC)
    {
        SIMCOM_ATCommand.CurrentTimeoutATC += TIMER_ATC_PERIOD;
        if(SIMCOM_ATCommand.CurrentTimeoutATC >= SIMCOM_ATCommand.TimeoutATC){
            if (_enableLog) {
                Serial.println("<--- Timeout AT command");
            }
            SIMCOM_ATCommand.CurrentTimeoutATC -= SIMCOM_ATCommand.TimeoutATC;
            if(SIMCOM_ATCommand.RetryCountATC > 0){
                SIMCOM_ATCommand.RetryCountATC--;
                SendATCommand();
            }
            else {
                SIMCOM_ATCommand.TimeoutATC = 0;
                return IN_TIMEOUT;
            }
        }

    }
    return 0;
}

int InnoWaySimcom:: innoHandler(char * data) {
    /*Return OK handler*/
    if(SIMCOM_ATCommand.ExpectResponseFromATC[0] != 0 && strstr((const char*)data, SIMCOM_ATCommand.ExpectResponseFromATC)) {
        SIMCOM_ATCommand.ExpectResponseFromATC[0] = 0;
        SIMCOM_ATCommand.TimeoutATC = 0;
        memset(data,0,sizeof(data));
        return IN_OK;
    }
    if(strstr((const char*)data, "ERROR")) {
        memset(data,0,sizeof(data));
        return IN_ERROR;
    }
}

int InnoWaySimcom:: pollingUart()
{
    int err = checkRetry();
    if (err) {
        return err;
    }
    int index = 0;
    bool receive_flag = false;
#ifdef POLLING_UART0
    while(Serial.available()) {
        char byte = Serial.read();
#endif
#ifdef POLLING_UART2
    while(Serial2.available()) {
        char byte = Serial2.read();
//        Serial.print(byte);
#endif
//        _buffer[index] = Serial.read();
        _buffer[index] = byte;
        index++;
//        Serial.print(byte);
        if( _buffer[index-1] == '\r' && _buffer[index-3] == 'O' && (_buffer[index-2] == 'R' || _buffer[index-2] == 'K')) {
            _buffer[index] = '\0';
            if (_enableLog) {
                Serial.println("\r\nSim7020 data rec:");
                Serial.print(_buffer);
            }
            return innoHandler(_buffer);
        }
        else if (callback) {
            if( _buffer[index-1] == '\r' && _buffer[index-2] == '\"' && strstr(_buffer, "+CMQPUB: 0")) {
                _buffer[index-1]=0;
                char data_sub[512];
                char topic[100];
                uint8_t count = 0;
                for (int i = 0; i < strlen(_buffer); i++) {
                    if (_buffer[i] == '\"') {
                        count ++;
                    }
                }
                filter_char(_buffer, 3,count,data_sub,34);
                filter_char(_buffer, 1,2,topic,34);
                if (_enableLog) {
                    Serial.print("\r\nData rec:");
                    Serial.print(_buffer);
//                    Serial.print("\r\nData sub return:");
//                    Serial.print(data_sub);
                }
                callback(topic,data_sub,strlen(data_sub));
            }
        }
    }
    return 0;
}
/*------------------------------------------------*/
void InnoWaySimcom:: SendATCommand()
{
    if (_enableLog) {
        Serial.print("\r\nSend AT command: ");
        Serial.print(SIMCOM_ATCommand.CMD);
    }
#ifdef RESET_UART
    #ifdef POLLING_UART0
        while(Serial.available()) {
            Serial.read();
        }
    #endif
    #ifdef POLLING_UART2
        while(Serial2.available()){
            Serial2.read();
        }
    #endif
#endif
    memset(_buffer,0,sizeof(_buffer));
    Serial2.print((const char *) SIMCOM_ATCommand.CMD);
}

void InnoWaySimcom:: ATC_SendATCommand(const char *Command, char *ExpectResponse, uint32_t Timeout, uint8_t RetryCount)
{
    strcpy(SIMCOM_ATCommand.CMD, Command);
    SIMCOM_ATCommand.lenCMD = strlen(SIMCOM_ATCommand.CMD);
    strcpy(SIMCOM_ATCommand.ExpectResponseFromATC, ExpectResponse);
    SIMCOM_ATCommand.RetryCountATC = RetryCount;
    SIMCOM_ATCommand.TimeoutATC = Timeout;
    SIMCOM_ATCommand.CurrentTimeoutATC = 0;
    SendATCommand();
}


int InnoWaySimcom:: innoPolling(uint32_t timeout)
{
    while(timeout) {
        int err = pollingUart();
        if (err) {
            return err;
        }
        delay(100);
        timeout = timeout - 100;
    }
    return IN_WHILE_TIMEOUT;
}

int InnoWaySimcom:: innoConnect() {
    return innoConnect(_deviceID,_deviceID,_token,300,1);
}

int InnoWaySimcom:: innoConnect(char * device_id,char *user, char * password,  uint32_t keep_alive, uint8_t clean_session) {
    ATC_SendATCommand("AT\r\n", "OK", 2000, 0);
    int err = innoPolling(3000);
    if (err != IN_OK) {
        return err;
    }
    
    ATC_SendATCommand("AT+CREVHEX=0\r\n", "OK", 2000, 0);
    err = innoPolling(3000);
    if (err != IN_OK) {
        return err;
    }
    
    ATC_SendATCommand("AT+COPS?\r\n", "\"45204\",9", 1000, 30);
    err = innoPolling(30000);
    if (err != IN_OK) {
        return err;
    }
    
    ATC_SendATCommand("AT+CMQDISCON=0\r\n", "OK",2000, 0);
    err = innoPolling(3000);
    if (err != IN_OK && err != IN_ERROR) {
        return err;
    }
    
    char buf[512];
    sprintf(buf, "AT+CMQNEW=\"%s\",\"%d\",10000,512\r\n",INNOWAY_BROKER, INNOWAY_MQTT_PORT);
    ATC_SendATCommand(buf, "OK", 10000, 2);
    err = innoPolling(20000);
    if (err != IN_OK) {
        return err;
    }

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "AT+CMQCON=0,3,\"%s\",%d,%d,0,\"%s\",\"%s\"\r\n",device_id, keep_alive, clean_session, user, password);
//    sprintf(buf, "AT+CMQCON=0,3,\"%s\",%d,%d,0\r\n",device_id, keep_alive, clean_session);
    ATC_SendATCommand(buf, "OK", 5000, 2);
    return innoPolling(10000);
    
}

int InnoWaySimcom:: innoPublish(char * topic, char *payload) {
    return innoPublish(topic, payload, 1, 0);
}

int InnoWaySimcom:: innoPublish(char * topic, char *payload, uint8_t qos, uint8_t retain ) {
    char buf[512];
    uint8_t count = 0;
    Add_char(payload);
    for (int i = 0; i < strlen(payload); i++) {
        if (payload[i] == '\"') {
            count ++;
        }
    }
    sprintf(buf, "AT+CMQPUB=%d,\"%s\",%d,%d,%d,%d,\"%s\"\r\n", 0,topic,1,0,0,strlen(payload) - count,payload);
    ATC_SendATCommand(buf, "OK", 5000, 1);
    return innoPolling(30000);
}

int InnoWaySimcom:: innoSubscribe(char *topic, uint8_t qos) {
    char buf[512];
    sprintf(buf, "AT+CMQSUB=0,\"%s\",%d\r\n",topic,qos);
    ATC_SendATCommand(buf, "OK",2000, 1);
    return innoPolling(3000);
}

int InnoWaySimcom:: innoUnsub(char *topic) {
    char buf[512];
    sprintf(buf, "AT+CMQUNSUB=0,\"%s\"\r\n",topic);
    ATC_SendATCommand(buf, "OK",2000, 1);
    return innoPolling(3000);
}

int InnoWaySimcom:: innoDisconnect() {
    ATC_SendATCommand("AT+CMQDISCON=0\r\n", "OK",2000, 0);
    return innoPolling(2000);
}

void InnoWaySimcom:: loop() {
    pollingUart();
}
