#include <EasyTransfer.h>

EasyTransfer ET_SEND;
EasyTransfer ET_RECEIVE;

double joyx;
double joyy;

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
    // range 0-255
    int motorFrontLeft; // motor 1
    int motorFrontRight; // motor 2
    int motorBackLeft; // motor 3
    int motorBackRight; // motor 4
};

struct RECEIVE_DATA_STRUCTURE {
    // range 0-1023
    double pressure;
    double humidity;
    double temperature;
};

SEND_DATA_STRUCTURE sendData;
RECEIVE_DATA_STRUCTURE receiveData;

void setup() {
    Serial.begin(9600);
    ET_SEND.begin(details(sendData), &Serial);
    ET_RECEIVE.begin(details(receiveData), &Serial);
}

void loop() {
    joyx = 0;
    joyy = 0;
    
    sendData.motorFrontLeft = getMotorSpeed(1);
    sendData.motorFrontRight = getMotorSpeed(2);
    sendData.motorBackLeft = getMotorSpeed(3);
    sendData.motorBackRight = getMotorSpeed(4);
    ET_SEND.sendData();
    
    if (ET_RECEIVE.receiveData()) {
        Serial.print("pressure: ");
        Serial.print(receiveData.pressure);
        Serial.print(" humidity: ");
        Serial.print(receiveData.humidity);
        Serial.print(" temperature: ");
        Serial.println(receiveData.temperature);
    }
    
    delay(10);
}

double joynorm() {
    return sqrt(joyx * joyx + joyy * joyy);
}

double getMotorSpeed(int motorNumber) {
    if (motorNumber == 1 || motorNumber == 4) {
        if (signbit(joyx) == signbit(joyy)) {
            return signbit(joyx) * joynorm();
        } else {
            return joynorm() * atan(-joyx / joyy - M_PI / 4)  * (4 / M_PI) * signbit(joyy);
        }
    } else {
        if (signbit(-joyx) == signbit(joyy)) {
            return signbit(-joyx) * joynorm();
        } else {
            return joynorm() * atan(joyx / joyy - M_PI / 4)  * (4 / M_PI) * signbit(joyy);
        }
    }
}
