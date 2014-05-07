import tkinter as tk	
#import tkFont
from time import sleep;
from MOTOR import MOTOR

# Motor line coordinate constants
FRONT_Y = 40
BACK_Y = 205
LEFT_X = 40
RIGHT_X = 130
FRONT_VERT_X = 53
BACK_VERT_X = 195
VERT_Y = 65;

# Slider start position
START_POSITION = 0

class OrcusGUI(tk.Frame):	
	def __init__(self, master=None):
		# Initialize root frame
		tk.Frame.__init__(self, master)  
		# Initialize motor speed values
		self.frontLeft = 0
		self.frontRight = 0
		self.backLeft = 0
		self.backRight = 0
		self.frontVert = 0
		self.backVert = 0
		# Initialize ESTOP
		self.ESTOP = False # starts with system running?  True means stop
		# Initialize sensor values
		self.ROVconnect = "disconnected"
		self.controllerConnect = "disconnected"
		self.pressureValue = 0
		self.masterCurrentValue = 0
		
		# Displays banner, choose image or text
		self.imageBanner()
		# self.textBanner()		
		
		# Creates eStop button
		self.estopButton()
		
		# Displays sensor readings
		self.sensorReadings()
		
		# Draws ROV top view with translational motor arrows
		self.drawRovTopView()
		
		# Draws ROV side view with vertical motor arrows
		self.drawRovSideView()
		
		# Reads sliders and updates motor values
		self.getSliderInput()
		
		self.grid()	
	
	# Draws banner from image
	def imageBanner(self):
		self.banner = tk.Canvas(self, width=812, height=111)
		self.bannerImage = tk.PhotoImage(file='./UWROV_Banner.gif')
		self.b = self.banner.create_image(1,1,image=self.bannerImage, anchor='nw')
		self.banner.grid(row=0, column=0, columnspan=2)
	
	# Draws banner from text
	"""
    def textBanner(self):
		self.bannerFont = tkFont.Font(family='Calibri', size=20, weight='bold')
		self.banner = tk.Label(self, text='University of Washington Underwater Robotics Club\nUWROV', font = self.bannerFont, background='#883199')
		self.banner.grid(row=0, column=0, columnspan=3)
    """
	
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
		# Creates label frame for auxiliary control sliders
		self.sliderFrame = tk.LabelFrame(self, text='ROV Auxiliary Control')
	
		# Create list of callback functions
		motorValueCallbacks = [self.frontLeftMotorValue, self.frontRightMotorValue, self.backLeftMotorValue, self.backRightMotorValue, self.frontVertMotorValue, self.BackVertMotorValue]
		
		# Create empty list of sliders
		self.sliderFrame.motorScrolls = [None]*6
		
		# Create list of motor labels
		motorLabels = ["FrontLeft", "FrontRight", "BackLeft", "BackRight", "FrontVert", "BackVert"]
		
		# Create scroll bars for each motor
		for i in range(0,6):
			label = motorLabels[i]
			self.sliderFrame.motorScaleLabel = tk.Label(self.sliderFrame, text=label)
			self.sliderFrame.motorScaleLabel.grid(row=i, column=0, sticky=tk.W)
			
			self.sliderFrame.motorScrolls[i] = tk.Scale(self.sliderFrame, command=motorValueCallbacks[i], orient=tk.HORIZONTAL, from_=-100, length=450)
			self.sliderFrame.motorScrolls[i].set(START_POSITION)
			self.sliderFrame.motorScrolls[i].grid(row=i, column=1, sticky=tk.W+tk.E)
			
		self.sliderFrame.grid(row=4, column=0, columnspan=2, ipadx=10, sticky=tk.W+tk.E)
	
	# Draws ROV top view with motor speed vector overlay
	def drawRovTopView(self):
		self.rovTop = tk.Canvas(self, height=235, width=169)
		self.rovTopView = tk.PhotoImage(file='./orcusTop.gif')
		#self.rov = self.rovTop.create_rectangle(20,20,180,255, fill='#4c0099')
		self.rov = self.rovTop.create_image(1,1,image=self.rovTopView, anchor='nw')
		self.frontLeftLine = self.rovTop.create_line(0,0,0,0)
		self.frontRightLine = self.rovTop.create_line(0,0,0,0)
		self.backLeftLine = self.rovTop.create_line(0,0,0,0)
		self.backRightLine = self.rovTop.create_line(0,0,0,0)
		self.rovTop.grid(row = 1, column = 0, padx = 30, pady = 30)
	
	# draws ROV side view with motor speed vector overlay
	def drawRovSideView(self):
		self.rovSide = tk.Canvas(self, height=148, width=235)
		self.rovSideView = tk.PhotoImage(file='./orcusSide.gif')
		self.rov = self.rovSide.create_image(1,1,image=self.rovSideView, anchor='nw')
		self.frontVertLine = self.rovTop.create_line(0,0,0,0)
		self.backVertLine = self.rovTop.create_line(0,0,0,0)
		self.rovSide.grid(row = 1, column = 1, padx = 30, pady = 30)
	
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
			self.frontLeft = 0
			self.frontRight = 0
			self.backLeft = 0
			self.backRight = 0
			self.frontVert = 0
			self.backVert = 0
			
			for i in range(0,6):
				self.sliderFrame.motorScrolls[i].set(START_POSITION)
		
	# Creates eStop button
	def estopButton(self):
		#self.estopFont = tkFont.Font(family='Calibri', size=20, weight='bold')
		self.estop = tk.Button(self, text='ESTOP', command=self.estopCallback, background='red', width=7, padx=20, pady=20)
		self.estop.grid(row=3, column=1)
	
	# Displays sensor readings
	def sensorReadings(self):
		# Creates label frame for all sensor readings
		self.sensorFrame = tk.LabelFrame(self, text='System Information', width=200, height=110)
		
		# ROV connected
		self.sensorFrame.ROV = tk.Label(self.sensorFrame, text='ORCUS: ' + self.ROVconnect)
		self.sensorFrame.ROV.grid(row=0, column=0, sticky='w')
		# Control connected
		self.sensorFrame.controller = tk.Label(self.sensorFrame, text='Controller: ' + self.controllerConnect)
		self.sensorFrame.controller.grid(row=1, column=0, sticky='w')
		# Pressure sensor
		self.sensorFrame.pressure = tk.Label(self.sensorFrame, text='Pressure Reading: ' + str(self.pressureValue))
		self.sensorFrame.pressure.grid(row=2, column=0, sticky='w')
		# Current sensor
		self.sensorFrame.masterCurrent = tk.Label(self.sensorFrame, text='Master Current Reading: ' + str(self.masterCurrentValue))
		self.sensorFrame.masterCurrent.grid(row=3, column=0, sticky='w')
		
		self.sensorFrame.grid(row=3, column=0, ipadx=10)
		self.sensorFrame.grid_propagate(0)
		
	# Updates sensor reading labels
	def updateSensorReadings(self):
		self.sensorFrame.ROV.configure(text='ORCUS: ' + self.ROVconnect)
		self.sensorFrame.controller.configure(text='Controller: ' + self.controllerConnect)
		self.sensorFrame.pressure.configure(text='Pressure Reading: ' + str(self.pressureValue))
		self.sensorFrame.masterCurrent.configure(text='Master Current Reading: ' + str(self.masterCurrentValue))
		
	
	# draws ROV motors status
	# TODO redo changing line length with configure method
	def drawMotorStatus(self, motors):	
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
		

