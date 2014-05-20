import math, random, os, string, sys
import Drive
from Drive import MOTOR
import pygame as pg
from pygame.locals import *

pg.init()



# colors
white = (255, 255, 255)
black = (0, 0, 0)
gray = (90, 90, 90)
red = (255, 0, 0)
light_blue = (100, 150, 255)
dark_blue = (20, 100, 195)

# joystick controls
A_button = 0
B_button = 1
X_button = 2
Y_button = 3
X_axis = 0
Y_axis = 1
Z_axis = 2



def load_image(name):
    path = os.path.join("images", name)
    try:
        image = pg.image.load(path)
    except pg.error, message:
        print "Cannot load image:", path
        raise SystemExit, message
    return image


def sum(t1, t2):
    return tuple(x + y for x, y in zip(t1, t2))

def diff(t1, t2):
    return tuple(x - y for x, y in zip(t1, t2))


class Arrow(pg.sprite.Sprite):
    _arrow_size = (48, 192)
    _arrow_shape = ((24, 0), (0, 48), (16, 42), (16, 96),
                    (32, 96), (32, 42), (48, 48), (24, 0))
    arrow_length = 180
    
    def __init__(self, base_pos, angle, max_scale):
        pg.sprite.Sprite.__init__(self)
        self._base_image = pg.surface.Surface(Arrow._arrow_size, pg.SRCALPHA, 32)
        pg.draw.polygon(self._base_image, red, Arrow._arrow_shape)
        
        self.image = pg.transform.rotate(self._base_image, angle)
        self.rect = self.image.get_rect()
        
        self._base_pos = base_pos
        self._angle = angle
        self._max_scale = max_scale
        
        # range [0, 1]
        self.scale = 0
        self.update()
    
    def update(self):
        if abs(self.scale) > 1:
            print "invalid scale:", self.scale
            return
        
        pix_scale = int(self.scale * self._max_scale)
        
        color_scale = int(abs(self.scale) * 255)
        pg.draw.polygon(self._base_image,
                        (color_scale, 255 - color_scale, 50),
                        Arrow._arrow_shape)
        self.image = pg.transform.scale(self._base_image,
                                        (Arrow._arrow_size[0], abs(pix_scale)))
        if pix_scale != 0:
            self.image = pg.transform.rotate(self.image, self._angle)
        if pix_scale < 0:
            self.image = pg.transform.flip(self.image, True, True)
        
        self.rect = self.image.get_rect()
        self.rect.center = self._base_pos


class Button(pg.sprite.Sprite):
    def __init__(self, base_pos, dim, text="Button", on_click=lambda: None,
                 on_unclick=lambda: None, toggleable=True, text_color=black,
                 font=pg.font.Font(None, 16), color=light_blue,
                 color_pressed=dark_blue):
        pg.sprite.Sprite.__init__(self)
        self.image = pg.Surface(dim).convert()
        self.image.fill(color)
        self.rect = self.image.get_rect()
        self.rect.topleft = base_pos
        
        self._toggleable = toggleable
        self._text = font.render(text, True, text_color)
        self._on_click = on_click
        self._on_unclick = on_unclick
        self._color = color
        self._color_pressed = color_pressed
        
        self._clicked = False
    
    def update(self):
        if self._clicked:
            self.image.fill(self._color_pressed)
        else:
            self.image.fill(self._color)
        text_pos = self._text.get_rect(center=self.image.get_rect().center)
        self.image.blit(self._text, text_pos)
    
    def _is_in_button(self, pos):
        return self.rect.collidepoint(pos)
    
    def click(self):
        if self._toggleable:
            if not self._clicked:
                self._on_click()
                self._clicked = True
            else:
                self._on_unclick()
                self._clicked = False
        else:
            self._on_click()
            
    def click_if_in_button(self, pos):
        if self._is_in_button(pos):
            self.click()


class SensorReading(pg.sprite.Sprite):
    def __init__(self, label, key, pos):
        pg.sprite.Sprite.__init__(self)
        #self._background = pg.surface.Surface((100, 25), pg.SRCALPHA, 32)
        self._label = label
        self._key = key
        self._pos = pos
    
    def update(self):
        value = Drive.sensor_values[self._key]
        loc = string.find(self._label, "%r")
        print_text = self._label[:loc] + str(value) + self._label[loc + 2:]
        self.image = pg.font.Font(None, 22).render(print_text, True, gray)
        self.rect = self.image.get_rect()
        self.rect.topleft = self._pos
        #self.image


################################################################################
# JOYSTICK CONTROL                                                             #
################################################################################

def joy_init():
    """Initializes pygame and the joystick, and returns the joystick to be
    used."""
    
    global use_joy
    
    pg.init()
    pg.joystick.init()
    if pg.joystick.get_count() == 0:
        print "joy_init: No joysticks connected"
        return
    joystick = pg.joystick.Joystick(0)
    joystick.init()
    use_joy = True
    return joystick



################################################################################
# TICK EVENTS                                                                  #
################################################################################

def update_control_values():
    """Master update for the control object."""
    
    if E_stop:
        Drive.control.trans_x = 0
        Drive.control.trans_y = 0
        Drive.control.rise = 0
        Drive.control.yaw = 0
    elif use_joy:
        Drive.control.trans_x = joystick.get_axis(X_axis)
        Drive.control.trans_y = -1 * joystick.get_axis(Y_axis)
        Drive.control.rise = -1 * joystick.get_axis(Z_axis)
        Drive.control.yaw = joystick.get_axis(4)
    else:
        Drive.control.trans_x = float(pg.mouse.get_pos()[0]) / 400 - 1
        Drive.control.trans_y = 1 - float(pg.mouse.get_pos()[1]) / 400



def main():
    global joystick
    
    # initialize everything
    joystick = joy_init()
    Drive.init()
    
    # set the screen size
    screen = pg.display.set_mode((800, 700))
    
    # load the images
    banner = load_image("UWROV_Banner.bmp").convert()
    rov_top = load_image("OrcusTop.bmp").convert()
    rov_side = load_image("OrcusSide.bmp").convert()
    
    # coordinates for the ROV pictures
    rov_top_coords = (132, 162)
    rov_side_coords = (433, 206)
    
    # create the background
    background = pg.Surface(screen.get_size()).convert()
    background.fill(white)
    background.blit(banner, (0, 0))
    background.blit(rov_top, rov_top_coords)
    background.blit(rov_side, rov_side_coords)
    pg.draw.rect(background, black, Rect(0, 448, 800, 10))
    
    clock = pg.time.Clock()
    
    # add the arrows
    front_left = Arrow(sum(rov_top_coords, (28, 38)), 150, Arrow.arrow_length)
    front_right = Arrow(sum(rov_top_coords, (141, 38)), -150, Arrow.arrow_length)
    back_right = Arrow(sum(rov_top_coords, (141, 193)), -30, Arrow.arrow_length)
    back_left = Arrow(sum(rov_top_coords, (28, 193)), 30, Arrow.arrow_length)
    front_top = Arrow(sum(rov_side_coords, (51, 74)), 0, Arrow.arrow_length)
    back_top = Arrow(sum(rov_side_coords, (193, 74)), 0, Arrow.arrow_length)
    arrows = pg.sprite.RenderPlain(front_left, front_right, back_right,
                                   back_left, front_top, back_top)
    
    def e_stop_fun():
        global E_stop
        E_stop = not E_stop
    e_stop = Button((625, 482), (150, 100), "E STOP", e_stop_fun, e_stop_fun,
                    True, black, pg.font.Font(None, 36), red, (200, 0, 0))
    buttons = pg.sprite.RenderPlain(e_stop)
    
    pressure = SensorReading("Pressure: %r mbar", b'P', (25, 482))
    humidity = SensorReading("Humidity: %r %", b'H', (25, 502))
    temperature = SensorReading("Temperature: %r degrees", b'T', (25, 522))
    current = SensorReading("Current: %r mA", b'C', (25, 542))
    sensors = pg.sprite.RenderPlain(pressure, humidity, temperature, current)
    
    while True:
        # limit to 30 fps
        clock.tick(30)
        
        update_control_values()
        Drive.tick()
        
        # calculate arrow display
        front_left.scale = float(Drive.motors[MOTOR.FT_LT].power) / 255
        front_right.scale = float(Drive.motors[MOTOR.FT_RT].power) / 255
        back_right.scale = float(Drive.motors[MOTOR.BK_RT].power) / 255
        back_left.scale = float(Drive.motors[MOTOR.BK_LT].power) / 255
        front_top.scale = float(Drive.motors[MOTOR.FT_VL].power) / 255
        back_top.scale = float(Drive.motors[MOTOR.BK_VL].power) / 255
        
        arrows.update()
        buttons.update()
        sensors.update()
        screen.blit(background, (0, 0))
        arrows.draw(screen)
        buttons.draw(screen)
        sensors.draw(screen)
        pg.display.flip()
        
        # look for buttons pressed, mouse clicks, etc.
        for event in pg.event.get():
            if event.type == pg.JOYBUTTONDOWN and \
               event.__dict__["button"] == B_button:
                # tare joystick
                Drive.control.tare()
            elif event.type == pg.JOYBUTTONDOWN and \
                 event.__dict__["button"] == A_button:
                # adjust constant vertical force downward
                if Drive.control.rise_control > -1:
                    Drive.control.rise_control -= .05
            elif event.type == pg.JOYBUTTONDOWN and \
                 event.__dict__["button"] == Y_button:
                # adjust constant vertical force upward
                if Drive.control.rise_control < 1:
                    Drive.control.rise_control += .05
            
            elif event.type == pg.MOUSEBUTTONDOWN: # and event.pos
                for button in buttons:
                    button.click_if_in_button(event.pos)
            
            elif event.type == KEYDOWN and event.key == K_SPACE:
                e_stop.click()
            
            elif event.type == QUIT:
                exit()
            
            # TODO REMOVE THIS - FOR TESTING PURPOSES ONLY (we don't want to
            # quit if someone accidentally presses escape)
            if event.type == KEYDOWN and event.key == K_ESCAPE:
                exit()



################################################################################
# GLOBAL VARIABLES :/                                                          #
################################################################################

# the joystick (if any)
joystick = None

# whether to use the joystick or not
use_joy = False

# whether the robot should stop all activity
E_stop = False



main()