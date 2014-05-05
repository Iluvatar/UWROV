import Tkinter as tk	
import tkFont
# import serial, math;
# # from enum import Enum;
# from serial import Serial, SerialException, SerialTimeoutException;
from time import sleep;
from MOTOR import MOTOR

START_POSITION = 50

class OrcusGUI(tk.Frame):	
	def __init__(self, master=None):
		tk.Frame.__init__(self, master)  
		self.frontLeft = 0
		self.frontRight = 0
		self.backLeft = 0
		self.backRight = 0
		self.frontVert = 0
		self.backVert = 0
		self.ESTOP = False # starts with system running?  True means stop
		
		# Displays banner
		self.bannerFont = tkFont.Font(family='Calibri', size=20, weight='bold')
		self.banner = tk.Label(self, text='University of Washington Underwater Robotics\nUWROV', font = self.bannerFont, background='#4c0099')
		self.banner.grid(row=0, column=0, columnspan=3)
		
		# creates eStop button
		self.estopButton()
		
		# Draws ROV top view with translational motor arrows
		self.rovTop = tk.Canvas(self, height=235, width=169)
		self.rovTopView = tk.PhotoImage(file='./orcusTop.gif')
		#self.rov = self.rovTop.create_rectangle(20,20,180,255, fill='#4c0099')
		self.rov = self.rovTop.create_image(1,1,image=self.rovTopView, anchor='nw')
		self.frontLeftLine = self.rovTop.create_line(0,0,0,0)
		self.frontRightLine = self.rovTop.create_line(0,0,0,0)
		self.backLeftLine = self.rovTop.create_line(0,0,0,0)
		self.backRightLine = self.rovTop.create_line(0,0,0,0)
		self.rovTop.grid(row = 1, column = 0, padx = 30, pady = 30)
		
		# Draws ROV side view with vertical motor arrows
		self.rovSide = tk.Canvas(self, height=148, width=235)
		self.rovSideView = tk.PhotoImage(file='./orcusSide.gif')
		self.rov = self.rovSide.create_image(1,1,image=self.rovSideView, anchor='nw')
		self.frontVertLine = self.rovTop.create_line(0,0,0,0)
		self.backVertLine = self.rovTop.create_line(0,0,0,0)
		self.rovSide.grid(row = 1, column = 1, padx = 30, pady = 30)
		
		self.grid()		
		self.getSliderInput()
		#self.drawMotorStatus()
	
	# Motor control slider callbacks
	def frontLeftMotorValue(self, value = 0):
		self.frontLeft = int(value)
	def frontRightMotorValue(self, value = 0):
		self.frontRight = int(value)
	def backLeftMotorValue(self, value = 0):
		self.backLeft = int(value)
	def backRightMotorValue(self, value = 0):
		self.backRight = int(value)
	def frontVertMotorValue(self, value = 0):
		self.frontVert = int(value)
	def BackVertMotorValue(self, value = 0):
		self.backVert = int(value)
			
	# gets control information from sliders
	def getSliderInput(self):
		motorValueCallbacks = [self.frontLeftMotorValue, self.frontRightMotorValue, self.backLeftMotorValue, self.backRightMotorValue, self.frontVertMotorValue, self.BackVertMotorValue]
		
		self.motorScrolls = [None]*6
		
		motorLabels = ["FrontLeft", "FrontRight", "BackLeft", "BackRight", "FrontVert", "BackVert"]
		
		# Create scroll bars for each motor
		for i in range(0,6):
			label = motorLabels[i]
			self.motorScaleLabel = tk.Label(self, text = label)
			self.motorScaleLabel.grid(row = i + 3, column = 0, sticky = tk.W)
			
			self.motorScrolls[i] = tk.Scale(self, command=motorValueCallbacks[i], orient=tk.HORIZONTAL)
			self.motorScrolls[i].set(START_POSITION)
			self.motorScrolls[i].grid(row = i + 3, column = 1, ipadx = 100)
	
	# prints motor values from sliders
	# Not used ****
	def printMotorValues(self):
		print ("FrontLeft: " + str(self.frontLeft))
		print ("FrontRight: " + str(self.frontRight))
		print ("BackLeft: " + str(self.backLeft))
		print ("BackRight: " + str(self.backRight))
		print ("FrontVert: " + str(self.frontVert))
		print ("BackVert: " + str(self.backVert))
	
	# Changes the states of ESTOP to latch
	def estopCallback(self):
		self.ESTOP = not self.ESTOP
		if (self.ESTOP):
			self.estop.configure(background='green', text='Run')
		else:
			self.estop.configure(background='red', text='ESTOP')
	
	# When eStop button is pressed all the motor speeds are set to 0
	# Sliders are also returned to the zero position
	def estopControl(self):
		if (self.ESTOP):
			self.frontLeft = 50
			self.frontRight = 50
			self.backLeft = 50
			self.backRight = 50
			self.frontVert = 50
			self.backVert = 50
			
			for i in range(0,6):
				self.motorScrolls[i].set(START_POSITION)
		
	# Creates eStop button
	def estopButton(self):
		self.estopFont = tkFont.Font(family='Calibri', size=20, weight='bold')
		self.estop = tk.Button(self, text='ESTOP', font=self.estopFont, command=self.estopCallback, background='red', width=7, padx=20, pady=20)
		self.estop.grid(row=1, column=3)
	
	# draws ROV motors status
	def drawMotorStatus(self, motors):
		# Motor line coordinate constants
		FRONT_Y = 40
		BACK_Y = 205
		LEFT_X = 40
		RIGHT_X = 130
		FRONT_VERT_X = 53
		BACK_VERT_X = 195
		VERT_Y = 65;
		
		# Delete old line
		self.rovTop.delete(self.frontLeftLine)
		self.rovTop.delete(self.frontRightLine)
		self.rovTop.delete(self.backLeftLine)
		self.rovTop.delete(self.backRightLine)
		self.rovSide.delete(self.frontVertLine)
		self.rovSide.delete(self.backVertLine)
		
		# Draw motor lines
		self.frontLeftLine = self.rovTop.create_line(LEFT_X, FRONT_Y, LEFT_X+0.5*motors[MOTOR.FR_LF].power/3, FRONT_Y-0.866*motors[MOTOR.FR_LF].power/3, width=10, arrow=tk.LAST, fill='red')
		
		self.frontRightLine = self.rovTop.create_line(RIGHT_X, FRONT_Y, RIGHT_X-0.5*motors[MOTOR.FR_RT].power/3, FRONT_Y-0.866*motors[MOTOR.FR_RT].power/3, width=10, arrow=tk.LAST, fill='red')
		
		self.backLeftLine = self.rovTop.create_line(LEFT_X, BACK_Y, LEFT_X+0.5*motors[MOTOR.BA_LF].power/3, BACK_Y+0.866*motors[MOTOR.BA_LF].power/3, width=10, arrow=tk.LAST, fill='red')
		
		self.backRightLine = self.rovTop.create_line(RIGHT_X, BACK_Y, RIGHT_X-0.5*motors[MOTOR.BA_RT].power/3, BACK_Y+0.866*motors[MOTOR.BA_RT].power/3, width=10, arrow=tk.LAST, fill='red')
		
		self.frontVertLine = self.rovSide.create_line(FRONT_VERT_X, VERT_Y, FRONT_VERT_X, VERT_Y-motors[MOTOR.FR_VT].power/3, width=10, arrow=tk.LAST, fill='red')
		
		self.backVertLine = self.rovSide.create_line(BACK_VERT_X, VERT_Y, BACK_VERT_X, VERT_Y-motors[MOTOR.BA_VT].power/3, width=10, arrow=tk.LAST, fill='red')
		

