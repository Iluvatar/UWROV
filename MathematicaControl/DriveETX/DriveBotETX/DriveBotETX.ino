#include <EasyTransfer.h>

EasyTransfer ET_SEND;
EasyTransfer ET_RECEIVE;

int motorFLPort = 1; // motor 1 port
int motorFRPort = 2; // motor 2 port
int motorBLPort = 3; // motor 3 port
int motorBRPort = 4; // motor 4 port

int pressureSensor = 5;
int humiditySensor = 6;
int temperatureSensor = 7;

// motor layout
// 
//      Front
//  1          2
//   /---------\
//  /|         |\
//   |         |
//   |         |
//   |         |
//  \|         |/
//   \---------/
//  4          3

struct SEND_DATA_STRUCTURE {
    // range 0-1023
    double pressure;
    double humidity;
    double temperature;
};

struct RECEIVE_DATA_STRUCTURE {
    // range 0-255
    int motorFrontLeft;
    int motorFrontRight;
    int motorBackLeft;
    int motorBackRight;
};

SEND_DATA_STRUCTURE sendData;
RECEIVE_DATA_STRUCTURE receiveData;

void setup() {
    Serial.begin(9600);
    ET_SEND.begin(details(sendData), &Serial);
    ET_RECEIVE.begin(details(receiveData), &Serial);
}

void loop() {
    sendData.pressure = analogRead(pressureSensor);
    sendData.humidity = analogRead(humiditySensor);
    sendData.temperature = analogRead(temperatureSensor);
    ET_SEND.sendData();
    
    if (ET_RECEIVE.receiveData()) {
        analogWrite(motorFLPort, receiveData.motorFrontLeft);
        analogWrite(motorFRPort, receiveData.motorFrontRight);
        analogWrite(motorBLPort, receiveData.motorBackLeft);
        analogWrite(motorBRPort, receiveData.motorBackRight);
    }
    
    delay(10);
}
