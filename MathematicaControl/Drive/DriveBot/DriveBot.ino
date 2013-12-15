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

// the delay between updates
#define REFRESH_RATE 100

// The total number of motors on the robot.
#define NUM_MOTORS 6

// The total number of sensors the robot knows about.
#define NUM_SENSORS 3

// Used to easily change which output values correspond with which direction
#define FORWARD HIGH
#define BACKWARD LOW

/**
 * Class that represents a motor. Each motor has a port for each power output
 * and direction output. It also has a current power and direction, as well as
 * whether its output direction should be flipped because it is facing
 * backwards. Motors also have an ID that matches them to their physical
 * position on the robot.
 */
class Motor {
public:
    int powerPort;
    int dirPort;
    int power;
    int dir;
    boolean flipped;
    int ID;
    
    Motor(int powerPort, int dirPort, boolean flipped, int ID):
    powerPort(powerPort), dirPort(dirPort), flipped(flipped), ID(ID) {
        power = 0;
        dir = FORWARD;
    }
};

/**
 * Class that represents a sensor. Each Sensor has a port from which data is
 * read, a name to easily identify it, and a header that is used to send its
 * data through serial communication.
 */
class Sensor {
public:
    int port;
    String name;
    char header;
    
    Sensor(int port, String name, char header):
    port(port), name(name), header(header) { }
};

// Contains all the Motors on the robot.
Motor* motors[NUM_MOTORS];

// Contains all the Sensors on the robot.
Sensor* sensors[NUM_SENSORS];

/**
 * Initializes all the motors and sensors with the right ports and other data.
 */
void initialize() {
    motors[0] = new Motor(2, 22, true, 1);
    motors[1] = new Motor(3, 23, true, 2);
    motors[2] = new Motor(4, 24, false, 3);
    motors[3] = new Motor(5, 25, false, 4);
    motors[4] = new Motor(6, 26, false, 5);
    motors[5] = new Motor(7, 27, false, 6);
    
    sensors[0] = new Sensor(A0, "Pressure", 'P');
    sensors[1] = new Sensor(A1, "Humidity", 'H');
    sensors[2] = new Sensor(A2, "Temperature", 'T');
}

void setup() {
    Serial.begin(9600);
        pinMode(13, OUTPUT); digitalWrite(13, LOW); // REMOVE
    
    // initializes motor and sensor values
    initialize();
    
    // set the pin modes for the motor power and direction ports
    for (int i = 0; i < NUM_MOTORS; i++) {
        pinMode(motors[i]->powerPort, OUTPUT);
        pinMode(motors[i]->dirPort, OUTPUT);
    }
    
    // set the pin modes for the sensors
    for (int i = 0; i < NUM_SENSORS; i++) {
        pinMode(sensors[i]->port, INPUT);
    }
}

void loop() {
    readInputValues();
    setMotorValues();
    writeSensorValues();
        
    delay(100);
}

/**
 * Reads all values that come from connected sources. In this case, read and 
 * update motor values and directions.
 * 
 * Data is asynchronously sent and accumulates in the buffer. Whenever this
 * method runs, it iterates through the buffer and looks for values that have
 * valid headers.
 */
void readInputValues() {
    // read motor values from the controller
    while (Serial.available()) {
        int value = Serial.read();
        
        // motor power readings
        // valid heaers are 1-6
        if (value >= '1' && value <= '6') {
            motors[value - '1']->power = Serial.read();
        }
        
        // motor direction readings
        // valid headers are a-f, corresponding to motors 1-6
        if (value >= 'a' && value <= 'f') {
            motors[value - 'a']->dir = Serial.read();
        }
    }
}

/**
 * Sets the motor power and directions from the most recently updated values.
 */
void setMotorValues() {
    for (int i = 0; i < NUM_MOTORS; i++) {
        analogWrite(motors[i]->powerPort, motors[i]->power);
        
        // account for flipped motors
        int dir = motors[i]->dir;
        if (motors[i]->flipped) {
            dir = dir == LOW ? HIGH : LOW;
        }
        
        digitalWrite(motors[i]->dirPort, dir);
    }
}

/**
 * Writes the most recent sensor values to serial communications. Each value
 * has its identifier as a header.
 */
void writeSensorValues() {
    for (int i = 0; i < NUM_SENSORS; i++) {
        Serial.write(sensors[i]->header);
        
        // Values are divided by 4 to fit in a byte (the range of analogRead is
        // 0-1023; analogWrite is 0-255).
        Serial.write(analogRead(sensors[i]->port) / 4);
    }
}
