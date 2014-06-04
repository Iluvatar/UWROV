// Motor layout, top view. Motors 1 and 2 point towards the back. Motors 3 and 4
// point towards the front. Motors 5 and 6 point up.
// 
//      Front
//  1           2
//   /---------\
//  /|         |\
//   |    5    |
//   |         |
//   |         |
//   |         |
//   |    6    |
//  \|         |/
//   \---------/
//  4           3

// Used to easily change which output values correspond with which direction
#define FORWARD LOW
#define BACKWARD HIGH

// the delay between updates in milliseconds
#define REFRESH_RATE 100

// The total number of motors on the robot.
#define NUM_MOTORS 6

// The total number of sensors the robot knows about.
#define NUM_SENSORS 3

// The header for communication testing packets. Somewhat of a magic number.
#define COM_HEADER 170


/**
 * Class that represents a vector for a motor. Encampasses the direction and
 * speed of the motor, and gives a number (negative for backward) that
 * represents them.
 */
class Velocity
{
    // returns the power in the range [0, 255]
    int validate_abs_power(int power)
    {
        return max(0, min(255, power));
    }
    
    // returns the direction the motor is powered
    int direc(int num)
    {
        return num >= 0 ? FORWARD : BACKWARD;
    }
    
public:
    int power;
    int dir;
    
    Velocity(int power, int dir) : power(power), dir(dir) { }
    
    // returns the power of the motor in the range [-255, 255]
    int velnum(void)
    {
        return validate_abs_power(power) * (dir == FORWARD ? 1 : -1);
    }
    
    // updates the motor value from velNum in the range [-255, 255]
    void update_vel(int velnum)
    {
        power = validate_abs_power(abs(velnum));
        dir = direc(velnum);
    }
};

/**
 * Class that represents a motor. Each motor has a port for each power output
 * and direction output. It also has a current velocity and the desired
 * velocity, as well as whether its output direction should be flipped
 * because it is facing backwards. Motors also have an ID that matches them to
 * their physical position on the robot.
 */
class Motor
{
public:
    int power_port;
    int dir_port;
    
    // the power the motor should be running at
    Velocity * cur_vel;
    
    // the power the motor should be tending towards
    Velocity * new_vel;
    
    boolean flipped;
    int id;
    
    Motor(int power_port, int dir_port, int id):
    power_port(power_port), dir_port(dir_port), id(id)
    {
        cur_vel = new Velocity(0, FORWARD);
        new_vel = new Velocity(0, FORWARD);
    }
};

/**
 * Class that represents a sensor. Each Sensor has a port from which data is
 * read, a name to easily identify it, and a header that is used to send its
 * data through serial communication.
 */
class Sensor
{
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
void initialize()
{
    // 2  3  4  5  6  7
    // 23 25 27 22 24 26
    motors[4] = new Motor(2, 23, 1);
    motors[5] = new Motor(3, 25, 2);
    motors[0] = new Motor(4, 27, 3);
    motors[1] = new Motor(5, 22, 4);
    motors[2] = new Motor(6, 24, 5);
    motors[3] = new Motor(7, 26, 6);
    
    sensors[0] = new Sensor(A0, "Pressure", 'P');
    sensors[1] = new Sensor(A1, "Temperature", 'T');
    sensors[2] = new Sensor(A2, "Humidity", 'H');
}

void setup()
{
    Serial3.begin(9600);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
    
    // initializes motor and sensor values
    initialize();
    
    // set the pin modes for the motor power and direction ports
    for (int i = 0; i < NUM_MOTORS; i++)
    {
        pinMode(motors[i]->power_port, OUTPUT);
        pinMode(motors[i]->dir_port, OUTPUT);
    }
    
    // set the pin modes for the sensors
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        pinMode(sensors[i]->port, INPUT);
    }
}


// used for periodic tasks that aren't executed every loop
int time = 0;
int light_state = LOW;

void loop()
{
    // wait for enough data to arrive
    do
    {
        // after about 1 second of lost communication, stop the motors
        if (time > 1000)
        {
            reset_motors();
        }
        
        // update value every 50ms
        if (time % 50 == 0)
        {
            update_motor_values();
            write_sensor_values();
            set_motor_values();
        }
        
        delay(1);
        time++;
    } while (Serial3.available() < 3);
    
    // read and update on the new data
    read_input_values();
    set_motor_values();
    
    // toggle light for visual feedback if we recieve data
    if (light_state == LOW) {
        digitalWrite(13, HIGH);
        light_state = HIGH;
    } else {
        digitalWrite(13, LOW);
        light_state = LOW;
    }
    
    time = 0;
}

/**
 * Updates the motor values to speeds closer to the latest value. Prevents 
 * velocity changes that are too drastic.
 */
void update_motor_values()
{
    for (int i = 0; i < NUM_MOTORS; i++)
    {
        int true_power = motors[i]->new_vel->velnum();
        int current_power = motors[i]->cur_vel->velnum();
        
        int diff = true_power - current_power;
        if (true_power < current_power)
        {
            motors[i]->cur_vel->update_vel(max(true_power,
                                               current_power + diff / 10 - 2));
        }
        else if (true_power > current_power)
        {
            motors[i]->cur_vel->update_vel(min(true_power,
                                               current_power + diff / 10 + 2));
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
void read_input_values()
{
    // read motor values from the controller
    while (Serial3.available() >= 3)
    {
        int value = Serial3.read();
        
        // signal to test communication
        // returns the values sent
        if (value == COM_HEADER)
        {
            int num = validate_ser_values();
            if (num >= 0)
            {
                Serial3.write(COM_HEADER);
                Serial3.write(num);
                Serial3.write(num);
            }
        }
        
        // motor power readings
        // valid heaers are 1-6
        if (value >= '1' && value <= '6')
        {
            int num = validate_ser_values();
            if (num >= 0)
            {
                motors[value - '1']->new_vel->power = num;
            }
        }
        
        // motor direction readings
        // valid headers are a-f, corresponding to motors 1-6
        if (value >= 'a' && value <= 'f')
        {
            int num = validate_ser_values();
            if (num >= 0)
            {
                motors[value - 'a']->new_vel->dir = num;
            }
        }
    }
}

/**
 * Called after a valid header is found in the Serial3 stream. Makes sure the
 * next two bytes are the same. If the are, returns the value; otherwise,
 * returns -1. Notice that this eats the next two bytes in the input stream.
 */
int validate_ser_values()
{
    int in1 = Serial3.read();
    int in2 = Serial3.read();
    
    if (in1 == in2)
    {
        return in1;
    }
    
    return -1;
}

/**
 * Sets the motor power and directions from the most recently updated values.
 */
void set_motor_values()
{
    for (int i = 0; i < NUM_MOTORS; i++)
    {
        analogWrite(motors[i]->power_port, motors[i]->cur_vel->power);
        digitalWrite(motors[i]->dir_port, motors[i]->cur_vel->dir);
    }
}


/**
 * Sets all the motor powers to 0.
 */
void reset_motors()
{
    for (int i = 0; i < NUM_MOTORS; i++)
    {
        motors[i]->new_vel->update_vel(0);
    }
}


/**
 * Writes the most recent sensor values to serial communications. Each value
 * has its identifier as a header.
 */
void write_sensor_values()
{
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        Serial3.write(sensors[i]->header);
        
        // Values are divided by 4 to fit in a byte (the range of analogRead is
        // 0-1023; analogWrite is 0-255).
        Serial3.write(analogRead(sensors[i]->port) / 4);
        Serial3.write(analogRead(sensors[i]->port) / 4);
    }
}
