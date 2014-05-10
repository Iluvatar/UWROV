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

#include <SoftwareSerial.h>

// ports for SerialAlt
#define RX_PORT 10
#define TX_PORT 11

// Used to easily change which output values correspond with which direction
#define FORWARD LOW
#define BACKWARD HIGH

// The total number of motors on the robot.
#define NUM_MOTORS 6

// The total number of sensors the robot knows about.
#define NUM_SENSORS 3

// The total number of sensors on this Arduino
#define NUM_LOC_SENSORS 1

// The header for communication testing packets. Somewhat of a magic number.
#define COM_HEADER 170


/**
 * Class that represents a motor. Each motor has a current power (which is
 * negative for backward motion). Motors also have an ID that matches them to
 * their physical position on the robot, and a header that is used to send its
 * data through serial communication.
 */
class Motor
{
public:
    int power;
    int dir;
    int id;
    char power_header;
    char dir_header;
    
    Motor(int id, char power_header, char dir_header):
    id(id), power_header(power_header), dir_header(dir_header)
    {
        power = 0;
    }
};

/**
 * Class that represents a sensor. Each Sensor has a name to easily identify it
 * and a header that is used to send its data through serial communication.
 */
class Sensor
{
public:
    int value;
    String name;
    char header;
    
    Sensor(String name, char header) : name(name), header(header)
    {
        value = 0;
    }
};

class LocalSensor
{
public:
    int value;
    String name;
    char header;
    int port;
    
    LocalSensor(int port, String name, char header):
    port(port), name(name), header(header)
    {
        value = 0;
    }
};

// Contains all the Motors on the robot.
Motor* motors[NUM_MOTORS];

// Contains all the Sensors on the robot.
Sensor* sensors[NUM_SENSORS];

// Contains all the Local Sensors on this Arduino.
LocalSensor* local_sensors[NUM_LOC_SENSORS];

// connects to the other arduino
SoftwareSerial SerialAlt(RX_PORT, TX_PORT);

/**
 * Initializes all the motors and sensors with the right ports and other data.
 */
void initialize()
{
    motors[0] = new Motor(1, '1', 'a');
    motors[1] = new Motor(2, '2', 'b');
    motors[2] = new Motor(3, '3', 'c');
    motors[3] = new Motor(4, '4', 'd');
    motors[4] = new Motor(5, '5', 'e');
    motors[5] = new Motor(6, '6', 'f');
    
    sensors[0] = new Sensor("Pressure", 'P');
    sensors[1] = new Sensor("Humidity", 'H');
    sensors[2] = new Sensor("Temperature", 'T');
    
    local_sensors[0] = new LocalSensor(A0, "Current", 'C');
}

void setup()
{
    
    Serial.begin(9600);
    SerialAlt.begin(9600);
    
    // initializes motor and sensor values
    initialize();
}


// used for periodic tasks that aren't executed every loop
int time = 0;

void loop()
{
    // true if there is enough data for values to be extracted
    boolean new_vals = false;
    
    // wait for new sensor or motor data to arrive
    do
    {
        new_vals = Serial.available() >= 3 || SerialAlt.available() >= 3;
        
        if (time % 100 == 0)
        {
            // always write sensors values, as some sensors are on this Arduino
            write_sensor_values();
            
            // also always write motor values so the ROV knows it's connected
            write_motor_values();
        }
        
        time++;
        delay(1);
    } while (!new_vals);
    
    // read the new data
    read_input_values();
    
    time = 0;
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
void read_input_values()
{
    // read motor values from the computer
    while (Serial.available() >= 3)
    {
        int value = Serial.read();
        
        // signal to test communication
        // passes the packet on to the ROV
        if (value == COM_HEADER)
        {
            int num = validate_ser_values();
            if (num >= 0)
            {
                SerialAlt.write(COM_HEADER);
                SerialAlt.write(num);
                SerialAlt.write(num);
            }
        }
        
        // motor readings
        // valid prefixes are 1-6
        if (value >= '1' && value <= '6')
        {
            int num = validate_ser_values();
            if (num >= 0)
            {
                motors[value - '1']->power = num;
            }
        }
        
        // motor direction readings
        // valid headers are a-f, corresponding to motors 1-6
        if (value >= 'a' && value <= 'f')
        {
            int num = validate_ser_values();
            if (num >= 0)
            {
                motors[value - 'a']->dir = num == '0' ? BACKWARD : FORWARD;
            }
        }
    }
    
    // read sensor values from the rover
    while (SerialAlt.available() >= 3)
    {
        int value = SerialAlt.read();
        
        // signal that test communication was sent
        // passes the packet on to the computer
        if (value == COM_HEADER)
        {
            int num = validate_ser_alt_values();
            if (num >= 0)
            {
                Serial.write(COM_HEADER);
                Serial.write(num);
                Serial.write(num);
            }
        }
        
        // TODO make more elegant
        // sensor readings
        if (value == 'P')
        {
            int num = validate_ser_alt_values();
            if (num >= 0)
            {
                sensors[0]->value = num;
            }
        }
        else if (value == 'H')
        {
            int num = validate_ser_alt_values();
            if (num >= 0)
            {
                sensors[1]->value = num;
            }
        }
        else if (value == 'T')
        {
            int num = validate_ser_alt_values();
            if (num >= 0)
            {
                sensors[2]->value = num;
            }
        }
    }
}

/**
 * Called after a valid header is found in the Serial stream. Makes sure the
 * next two bytes are the same. If the are, returns the value; otherwise,
 * returns -1. Notice that this eats the next two bytes in the input stream.
 */
int validate_ser_values()
{
    int in1 = Serial.read();
    int in2 = Serial.read();
    
    if (in1 == in2)
    {
        return in1;
    }
    
    return -1;
}

/**
 * Called after a valid header is found in the SerialAlt stream. Makes sure the
 * next two bytes are the same. If the are, returns the value; otherwise,
 * returns -1. Notice that this eats the next two bytes in the input stream.
 */
int validate_ser_alt_values()
{
    int in1 = SerialAlt.read();
    int in2 = SerialAlt.read();
    
    if (in1 == in2)
    {
        return in1;
    }
    
    return -1;
}

/**
 * Writes the most recent motor values to serial communications, specifically to
 * the rover. Each value has its identifier as a header.
 */
void write_motor_values()
{
    for (int i = 0; i < NUM_MOTORS; i++)
    {
        SerialAlt.write(motors[i]->power_header);
        SerialAlt.write(motors[i]->power);
        SerialAlt.write(motors[i]->power);
        
        SerialAlt.write(motors[i]->dir_header);
        SerialAlt.write(motors[i]->dir);
        SerialAlt.write(motors[i]->dir);
    }
}

/**
 * Writes the most recent sensor values to serial communications, specifically
 * to the computer. Each value has its identifier as a header.
 */
void write_sensor_values()
{
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        Serial.write(sensors[i]->header);
        Serial.write(sensors[i]->value);
    }
    
    for (int i = 0; i < NUM_LOC_SENSORS; i++)
    {
        Serial.write(local_sensors[i]->header);
        
        // values are divided by 4 to fit in a byte (the range of analogRead is
        // [0, 1023]; analogWrite is [0, 255])
        Serial.write(analogRead(local_sensors[i]->port) / 4);
    }
}
