# NOTE: this is not fully functional yet. As in, it still doesn't work properly.

import serial, math;
from enum import Enum;
from serial import Serial, SerialException, SerialTimeoutException;
from time import sleep;


"""
      Front

 FR_LF     FR_RT
   /---------\
  /|         |\
   |  FR_VT  |
   |    o    |
   |         |
   |    o    |
   |  BA_VT  |
  \|         |/
   \---------/
 BA_LF     BA_RT

"""

class Motor:
    def __init__(self, ID, pow_header, dir_header, power = 0, dir = 0):
        self.ID = ID;
        self.pow_header = pow_header;
        self.dir_header = dir_header;
        self.power = power;
        self.dir = dir;

class MOTOR(Enum):
    FR_LF = 1; # front left
    FR_RT = 2; # front right
    BA_RT = 3; # back right
    BA_LF = 4; # back left
    FR_VT = 5; # front vertical
    BA_VT = 6; # back vertical


# range -1 to 1
trans_x, trans_y, yaw, rise = 0, 0, 0, 0;

motors = {MOTOR.FR_LF: Motor(MOTOR.FR_LF, b'1', b'a'),
          MOTOR.FR_RT: Motor(MOTOR.FR_RT, b'2', b'b'),
          MOTOR.BA_RT: Motor(MOTOR.BA_RT, b'3', b'c'),
          MOTOR.BA_LF: Motor(MOTOR.BA_LF, b'4', b'd'),
          MOTOR.FR_VT: Motor(MOTOR.FR_VT, b'5', b'e'),
          MOTOR.BA_VT: Motor(MOTOR.BA_VT, b'6', b'f')};

# range 0 to 255
# pressure, humidity, temperature, current
sensor_values = {b'P': 0, b'H': 0, b'T': 0, b'C': 0};



def connect(port_name):
    """Returns a Serial object that is connected to the port_name. Returns
    None if the connection could not be made."""
    
    try:
        return Serial(port_name, timeout = .05, writeTimeout = .05);
    except SerialException:
        print("connect: could not connect to port " + port_name);
        return None;


def read_data_values(ser, data_values):
    """Reads data from ser and updates the dict data_values.
    
    The keys in data_values should be bytes that are the headers for sensor
    values. Attempts to find each key in the read data and then updates it if
    the next two bytes are consistent."""
    
    if not isinstance(data_values, dict):
        raise ValueError("read_data_values: data_values not of type dict")
    if not isinstance(ser, Serial):
        raise ValueError("read_data_values: ser not of type Serial")
        
    # so we don't read too much data
    read_chars = min(ser.inWaiting(), len(data_values) * 5);
    raw_data = ser.read(read_chars);
    
    for key in data_values.keys():
        pos = raw_data[:-2].rfind(key);
        if pos != -1:
            bit1 = key[pos + 1];
            bit2 = key[pos + 2];
            if bit1 == bit2:
                data_values[key] = bit1;
    
    if read_chars > 12:
        ser.flushInput();


def write_motor_values(ser):
    """Writes the motor power and direction to the serial port.
    
    Each write consists of a header followed by the value repeated twice.o"""
    
    # do something about this
    global motors;
    
    if not isinstance(ser, Serial):
        raise ValueError("write_motor_values: ser not of type Serial")
    for motor in motors:
        pow = get_motor_power(motor);
        dir = b'1' if pow > 0 else b'0';
        ser.write(motors[motor].pow_header + bytes([abs(pow)] * 2));
        ser.write(motors[motor].dir_header + dir * 2);







def get_motor_power(n):
    """Takes the motor number and returns the magnitude of the total power it
    should output, from -255 to 255. Returns 0 if an invalid motor is given."""
    
    try:
        return (get_trans_power(n) + get_yaw_power(n) + get_rise_power(n)) * 255;
    except ValueError as bad_motor:
        print(bad_motor.args[0]);
        return 0;


def get_trans_power(n):
    """Takes the motor number and returns the power it should output for
    translational motion, from -1 to 1.
    
    Raises a ValueError if the motor number is unrecognized."""
    
    # do something about this
    global trans_x, trans_y;
    
    # these motors don't have an effect on translational speed
    if n == MOTOR.FR_VT or n == MOTOR.BA_VT:
        return 0;
    if n == MOTOR.FR_LF or n == MOTOR.BA_RT:
        # TODO implement
        print("Finish get_trans_power");
        return -1 * trans_y;
    if n == MOTOR.FR_RT or n == MOTOR.BA_LF:
        # TODO implement
        print("Finish get_trans_power");
        return trans_y;
    raise ValueError("get_trans_power: Illegal motor number");


def get_yaw_power(n):
    """Takes the motor number and returns the power it should output for
    rotational motion, from -1 to 1.
    
    Raises a ValueError if the motor number is unrecognized."""
    
    # do something about this
    global yaw;
    
    # these motors don't have an effect on translational speed
    if n == MOTOR.FR_VT or n == MOTOR.BA_VT:
        return 0;
    if n == MOTOR.FR_LF or n == MOTOR.BA_RT:
        return yaw;
    if n == MOTOR.FR_RT or n == MOTOR.BA_LF:
        return -1 * yaw;
    raise ValueError("get_yaw_power: Illegal motor number");


def get_rise_power(n):
    """Takes the motor number and returns the power it should output for
    vertical motion, from -1 to 1.
    
    Raises a ValueError if the motor number is unrecognized."""
    
    # do something about this
    global rise;
    
    # these motors don't have an effect on translational speed
    if n == MOTOR.FR_LF or n == MOTOR.BA_RT or \
       n == MOTOR.FR_RT or n == MOTOR.BA_LF:
        return 0;
    if n == MOTOR.FR_VT or n == MOTOR.BA_VT:
        return rise;
    raise ValueError("get_rise_power: Illegal motor number");







def main():
    # do something about this
    global sensor_values;
    
    ser = connect("/dev/console");
    
    while True:
        try:
            write_motor_values(ser);
        except SerialTimeoutException:
            print("write timeout");
        
        read_data_values(ser, sensor_values);
        
        sleep(5);

main();
