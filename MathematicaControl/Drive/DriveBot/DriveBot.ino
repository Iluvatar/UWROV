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

// Used to easily change which output values correspond with which direction
#define FORWARD HIGH
#define BACKWARD LOW

// the delay between updates
#define REFRESH_RATE 100

// The total number of motors on the robot.
#define NUM_MOTORS 6

// The total number of sensors the robot knows about.
#define NUM_SENSORS 3

// used for periodic tasks that aren't executed every loop
int time = 0;

/**
 * Class that represents a vector for a motor. Encampasses the direction and
 * speed of the motor, and gives a number (negative for backward) that
 * represents them.
 */
class Velocity {
    
    int validateAbsPower(int power) {
        return max(0, min(255, power));
    }
    
    int direc(int num) {
        return num >= 0 ? FORWARD : BACKWARD;
    }
    
public:
    int power;
    int dir;
    
    Velocity(int power, int dir) : power(power), dir(dir) {
    }
    
    int velNum(void) {
        return validateAbsPower(power) * (dir == FORWARD ? 1 : -1);
    }
    
    void updateVel(int velNum) {
        power = validateAbsPower(abs(velNum));
        dir = direc(velNum);
    }
};

/**
 * Class that represents a motor. Each motor has a port for each power output
 * and direction output. It also has a current velocity and the desired
 * velocity, as well as whether its output direction should be flipped
 * because it is facing backwards. Motors also have an ID that matches them to
 * their physical position on the robot.
 */
class Motor {
public:
    int powerPort;
    int dirPort;
    Velocity * curVel;
    Velocity * newVel;
    boolean flipped;
    int ID;
    
    Motor(int powerPort, int dirPort, boolean flipped, int ID):
    powerPort(powerPort), dirPort(dirPort), flipped(flipped), ID(ID) {
        curVel = new Velocity(0, FORWARD);
        newVel = new Velocity(0, FORWARD);
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
    // 2  3  4  5  6  7
    // 23 25 27 22 24 26
    motors[4] = new Motor(2, 23, true, 1);
    motors[5] = new Motor(3, 25, false, 2);
    motors[0] = new Motor(4, 27, true, 3);
    motors[1] = new Motor(5, 22, true, 4);
    motors[2] = new Motor(6, 24, false, 5);
    motors[3] = new Motor(7, 26, true, 6);
    
    sensors[0] = new Sensor(A0, "Pressure", 'P');
    sensors[1] = new Sensor(A1, "Temperature", 'T');
    sensors[2] = new Sensor(A2, "Humidity", 'H');
}

void setup() {
    Serial3.begin(9600);
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
    // wait for enough data to arrive
    do {
        if (time % 100 == 0) {
            updateMotorValues();
            writeSensorValues();
            setMotorValues();
        }
        
        delay(1);
        time++;
    } while (Serial3.available() < 3);
    
    readInputValues();
    setMotorValues();
}

/**
 * Updates the motor values to speeds closer to the latest value. Prevents 
 * velocity changes that are too drastic.
 */
void updateMotorValues() {
    for (int i = 0; i < NUM_MOTORS; i++) {
        int truePower = motors[i]->newVel->velNum();
        int currentPower = motors[i]->curVel->velNum();
        
        if (truePower < currentPower) {
            motors[i]->curVel->updateVel(max(truePower,
                                             currentPower + (truePower - currentPower) / 10 - 2));
        } else if (truePower > currentPower) {
            motors[i]->curVel->updateVel(min(truePower,
                                             currentPower + (truePower - currentPower) / 10 + 2));
        }
    }
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
    while (Serial3.available() >= 3) {
        int value = Serial3.read();
        
        // motor power readings
        // valid heaers are 1-6
        if (value >= '1' && value <= '6') {
            int in1 = Serial3.read();
            int in2 = Serial3.read();
            if (in1 == in2) {
                motors[value - '1']->newVel->power = in1;
            }
        }
        
        // motor direction readings
        // valid headers are a-f, corresponding to motors 1-6
        if (value >= 'a' && value <= 'f') {
            int in1 = Serial3.read();
            int in2 = Serial3.read();
            if (in1 == in2) {
                motors[value - 'a']->newVel->dir = in1;
            }
        }
    }
}

/**
 * Sets the motor power and directions from the most recently updated values.
 */
void setMotorValues() {
    for (int i = 0; i < NUM_MOTORS; i++) {
        analogWrite(motors[i]->powerPort, motors[i]->curVel->power);
        
        // account for flipped motors
        int dir = motors[i]->curVel->dir;
        if (motors[i]->flipped) {
            dir = dir == BACKWARD ? FORWARD : BACKWARD;
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
        Serial3.write(sensors[i]->header);
        
        // Values are divided by 4 to fit in a byte (the range of analogRead is
        // 0-1023; analogWrite is 0-255).
        Serial3.write(analogRead(sensors[i]->port) / 4);
    }
}
