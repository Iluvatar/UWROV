// Motor layout, top view. Motors 1 and 2 point towards the back. Motors 3 and 4
// point towards the front. Motors 5 and 6 point up.
// 
//      Front
//  1           2
//   /---------\
//  /|         |\
//   |    5    |
//   |         |
//   |    6    |
//  \|         |/
//   \---------/
//  4          3

#include <SoftwareSerial.h>

// the delay between updates
#define REFRESH_RATE 100

// ports for Serial2
#define RX_PORT 10
#define TX_PORT 11

// Used to easily change which output values correspond with which direction
#define FORWARD HIGH
#define BACKWARD LOW

// The total number of motors on the robot.
#define NUM_MOTORS 6

// The total number of sensors the robot knows about.
#define NUM_SENSORS 3

/**
 * Class that represents a motor. Each motor has a current power and direction.
 * Motors also have an ID that matches them to their physical position on the
 * robot, and a header that is used to send its data through serial
 * communication.
 */
class Motor {
public:
    int power;
    int dir;
    int ID;
    char powerHeader;
    char dirHeader;
    
    Motor(int ID, char powerHeader, char dirHeader):
    ID(ID), powerHeader(powerHeader), dirHeader(dirHeader) {
        power = 0;
        dir = FORWARD;
    }
};

/**
 * Class that represents a sensor. Each Sensor has a name to easily identify it
 * and a header that is used to send its data through serial communication.
 */
class Sensor {
public:
    int value;
    String name;
    char header;
    
    Sensor(String name, char header) : name(name), header(header) {
        value = 0;
    }
};

// Contains all the Motors on the robot.
Motor* motors[NUM_MOTORS];

// Contains all the Sensors on the robot.
Sensor* sensors[NUM_SENSORS];

// connects to the other arduino
SoftwareSerial Serial2(RX_PORT, TX_PORT);


/**
 * Initializes all the motors and sensors with the right ports and other data.
 */
void initialize() {
    motors[0] = new Motor(1, '1', 'a');
    motors[1] = new Motor(2, '2', 'b');
    motors[2] = new Motor(3, '3', 'c');
    motors[3] = new Motor(4, '4', 'd');
    motors[4] = new Motor(5, '5', 'e');
    motors[5] = new Motor(6, '6', 'f');
    
    sensors[0] = new Sensor("Pressure", 'P');
    sensors[1] = new Sensor("Humidity", 'H');
    sensors[2] = new Sensor("Temperature", 'T');
}

void setup() {
    Serial.begin(9600);
    Serial2.begin(9600);
        pinMode(13, OUTPUT); // REMOVE
        digitalWrite(13, LOW); // REMOVE
    
    // initializes motor and sensor values
    initialize();
}

void loop() {
    readInputValues();
    writeMotorValues();
    writeSensorValues();
    
    delay(REFRESH_RATE);
}

/**
 * Reads all values that come from connected sources. In this case, read and 
 * update motor values and directions from the computer, and sensor readings
 * from the rover.
 * 
 * Data is asynchronously sent and accumulates in the buffer. Whenever this
 * method runs, it iterates through the buffer and looks for values that have
 * valid headers.
 */
void readInputValues() {
    // read motor values from the computer
    while (Serial.available()) {
        int value = Serial.read();
        
        // motor readings
        // valid prefixes are 1-6
        if (value >= '1' && value <= '6') {
            motors[value - '1']->power = Serial.read();
        }
        
        // motor direction readings
        // valid headers are a-f, corresponding to motors 1-6
        if (value >= 'a' && value <= 'f') {
            motors[value - 'a']->dir = Serial.read() == '0' ? BACKWARD : FORWARD;
        }
    }
    
    // read sensor values from the rover
    while (Serial2.available()) {
        int value = Serial2.read();
        
        // TODO make more elegant
        // sensor readings
        if (value == 'P') {
            sensors[0]->value = Serial2.read();
        } else if (value == 'H') {
            sensors[1]->value = Serial2.read();
        } else if (value == 'T') {
            sensors[2]->value = Serial2.read();
        }
    }
}

/**
 * Writes the most recent motor values to serial communications, specifically to
 * the rover. Each value has its identifier as a header.
 */
void writeMotorValues() {
    for (int i = 0; i < NUM_MOTORS; i++) {
        Serial2.write(motors[i]->powerHeader);
        Serial2.write(motors[i]->power);
        
        Serial2.write(motors[i]->dirHeader);
        Serial2.write(motors[i]->dir);
    }
}

/**
 * Writes the most recent sensor values to serial communications, specifically
 * to the computer. Each value has its identifier as a header.
 */
void writeSensorValues() {
    for (int i = 0; i < NUM_SENSORS; i++) {
        Serial.write(sensors[i]->header);
        Serial.write(sensors[i]->value);
    }
}
