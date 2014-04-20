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

// ports for SerialAlt
#define RX_PORT 10
#define TX_PORT 11

// Used to easily change which output values correspond with which direction
#define FORWARD HIGH
#define BACKWARD LOW

// The total number of motors on the robot.
#define NUM_MOTORS 6

// The total number of sensors the robot knows about.
#define NUM_SENSORS 3

// The total number of sensors on this Arduino
#define NUM_LOC_SENSORS 1

// used for periodic tasks that aren't executed every loop
int time = 0;

/**
 * Class that represents a motor. Each motor has a current power (which is
 * negative for backward motion). Motors also have an ID that matches them to
 * their physical position on the robot, and a header that is used to send its
 * data through serial communication.
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

class LocalSensor {
public:
    int value;
    String name;
    char header;
    int port;
    
    LocalSensor(int port, String name, char header):
    port(port), name(name), header(header) {
        value = 0;
    }
};

// Contains all the Motors on the robot.
Motor* motors[NUM_MOTORS];

// Contains all the Sensors on the robot.
Sensor* sensors[NUM_SENSORS];

// Contains all the sensors on this Arduino.
LocalSensor* localSensors[NUM_LOC_SENSORS];

// connects to the other arduino
SoftwareSerial SerialAlt(RX_PORT, TX_PORT);

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
    
    localSensors[0] = new LocalSensor(A0, "Current", 'C');
}

void setup() {
    
    Serial.begin(9600);
    SerialAlt.begin(9600);
        pinMode(5, OUTPUT); // REMOVE
        pinMode(6, OUTPUT); // REMOVE
        pinMode(13, OUTPUT); // REMOVE
        digitalWrite(13, HIGH); // REMOVE
    
    // initializes motor and sensor values
    initialize();
}

void loop() {
    // true if there is enough data for values to be extracted
    boolean newMotorVals = false;
    
    // wait for enough data to arrive
    do {
        newMotorVals = Serial.available() >= 3;
        
        if (time % 10 == 0) {
            // always write sensors values, as some sensors are on this Arduino
            writeSensorValues();
        }
        
        time++;
        delay(10);
    } while (!newMotorVals);
    
    readInputValues();
    
    // only write new motor value if they were updated
    if (newMotorVals) {
        writeMotorValues();
    }
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
    while (Serial.available() >= 3) {
        int value = Serial.read();
        
        // motor readings
        // valid prefixes are 1-6
        if (value >= '1' && value <= '6') {
            int in1 = Serial.read();
            int in2 = Serial.read();
            if (in1 == in2) {
                motors[value - '1']->power = in1;
                //motors[value - '1']->power = 254;
            }
        }
        
        // motor direction readings
        // valid headers are a-f, corresponding to motors 1-6
        if (value >= 'a' && value <= 'f') {
            int in1 = Serial.read();
            int in2 = Serial.read();
            if (in1 == in2) {
                motors[value - 'a']->dir = in1 == '0' ? BACKWARD : FORWARD;
            }
        }
    }
    
    // read sensor values from the rover
    while (SerialAlt.available() >= 2) {
        int value = SerialAlt.read();
        
        // TODO make more elegant
        // sensor readings
        if (value == 'P') {
            sensors[0]->value = SerialAlt.read();
        } else if (value == 'H') {
            sensors[1]->value = SerialAlt.read();
        } else if (value == 'T') {
            sensors[2]->value = SerialAlt.read();
        }
    }
}

/**
 * Writes the most recent motor values to serial communications, specifically to
 * the rover. Each value has its identifier as a header.
 */
void writeMotorValues() {
    for (int i = 0; i < NUM_MOTORS; i++) {
        SerialAlt.write(motors[i]->powerHeader);
        SerialAlt.write(motors[i]->power);
        SerialAlt.write(motors[i]->power);
        
        SerialAlt.write(motors[i]->dirHeader);
        SerialAlt.write(motors[i]->dir);
        SerialAlt.write(motors[i]->dir);
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
    
    for (int i = 0; i < NUM_LOC_SENSORS; i++) {
        Serial.write(localSensors[i]->header);
        
        // Values are divided by 4 to fit in a byte (the range of analogRead is
        // 0-1023; analogWrite is 0-255).
        Serial.write(analogRead(localSensors[i]->port) / 4);
    }
}
