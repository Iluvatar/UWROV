int slavePin = 2;
int ledPin = 13;
int potPin = A0;

void setup() {
    Serial.begin(9600);
    pinMode(ledPin, OUTPUT);
    pinMode(slavePin, OUTPUT);
    pinMode(potPin, INPUT);
}

void loop() {
    if (Serial.available() > 0) {
        int inByte = Serial.read();
        
        // echo control signal output on the master
        // '1' is on, '0' is off
        // (characters are sent, as this is easier than using raw numbers)
        if (inByte == '1') {
            digitalWrite(ledPin, HIGH);
            digitalWrite(slavePin, HIGH);
        } else if (inByte == '0') {
            digitalWrite(ledPin, LOW);
            digitalWrite(slavePin, LOW);
        }
        
        delay(1);
    }
    
    // get the pot value and send the (value / 4) to the computer
    Serial.write(analogRead(potPin) / 4);
    
    delay(100);
}
