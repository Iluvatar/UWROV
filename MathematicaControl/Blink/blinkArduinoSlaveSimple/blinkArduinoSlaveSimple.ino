int masterPin = 2;
int ledPin = 13;

void setup() {
    Serial.begin(9600);
    pinMode(ledPin, OUTPUT);
    pinMode(masterPin, INPUT);
}

void loop() {
    // change led according to signal from master
    if (digitalRead(masterPin) == HIGH) {
        digitalWrite(ledPin, HIGH);
    } else {
        Serial.println("LOW----");
        digitalWrite(ledPin, LOW);
    }
    
    Serial.println("loop");
    delay(100);
}
