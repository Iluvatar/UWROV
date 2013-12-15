#include <EasyTransfer.h>

// Talks to the computer and acts as an interpreter for the slave. Sends the
// blink control data and relays the pot value (divided by 4 so that it
// corresponds to analog out values) back to the computer.

EasyTransfer ET_SEND;
EasyTransfer ET_RECEIVE;

int ledPin = 13;

// data that will be sent to the slave
struct SEND_DATA_STRUCTURE {
    int blinkData;
};

// data that will be received from the slave
struct RECEIVE_DATA_STRUCTURE {
    int potVal;
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
    while (Serial.available() > 0) {
        int inByte = Serial.read();
        
        // send the control signal to the slave
        sendData.blinkData = inByte;
        ET_SEND.sendData();
        
        // echo control signal output on the master
        // '1' is on, '0' is off
        // (characters are sent, as this is easier than using raw numbers)
        if (inByte == '1') {
            digitalWrite(13, HIGH);
        } else if (inByte == '0') {
            digitalWrite(13, LOW);
        }
        
        delay(100);
    }
    
    Serial.write(receiveData.potVal / 4);
    Serial.println("");
    Serial.print("----------Data: ");
    Serial.print(receiveData.potVal / 4);
    Serial.print("");
    
    delay(5000);
}
