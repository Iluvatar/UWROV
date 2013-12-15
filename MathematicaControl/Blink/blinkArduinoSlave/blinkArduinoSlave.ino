#include <EasyTransfer.h>

// Receives signals from the master to control the LED and sends back
// potentiometer readings.

EasyTransfer ET_SEND;
EasyTransfer ET_RECEIVE;

int potPin = A0;
int ledPin = 13;

// data that will be sent to the master
struct SEND_DATA_STRUCTURE {
    int potVal;
};

// data that will be received from the master
struct RECEIVE_DATA_STRUCTURE {
    int blinkData;
};

SEND_DATA_STRUCTURE sendData;
RECEIVE_DATA_STRUCTURE receiveData;

void setup() {
    Serial.begin(9600);
    ET_SEND.begin(details(sendData), &Serial);
    ET_RECEIVE.begin(details(receiveData), &Serial);
    
    pinMode(ledPin, OUTPUT);
}

void loop() {
    // for now, only '1' and '0' are sent, corresponding to on and off
    // (characters are sent, as this is easier than using raw numbers)
    if (ET_RECEIVE.receiveData()) {
        if (receiveData.blinkData == '1') {
            digitalWrite(13, HIGH);
        } else if (receiveData.blinkData == '0') {
            digitalWrite(13, LOW);
        }
    }
    
    // read from pot and send the value
    sendData.potVal = analogRead(potPin);
    ET_SEND.sendData();
    
    delay(20);
}
