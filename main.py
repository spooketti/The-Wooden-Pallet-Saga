from serial import Serial
import pygame
import time
arduino = Serial(port='COM3',   baudrate=9600, timeout=.1)
killSwitch = False

joystickPos = 0 #-1 to 1

pygame.init()
pygame.joystick.init()
joystick = pygame.joystick.Joystick(0)
joystick.init()

leftKill = False
rightKill = False
breaking = True

"""
Controls: 
leftstick drive accelerate forward/backward
b: toggle break
both triggers: kill switch
"""

breakMap = {True:b'BREAK',
 False:b'UNBREAK'}

while(not killSwitch):
    lastBreakState = breaking
    leftKill = False
    rightKill = False
    for event in pygame.event.get():
        if event.type == pygame.JOYAXISMOTION:
            if event.axis == 1:
                print(event.value)
        if event.type == pygame.JOYBUTTONDOWN:
            if event.button == pygame.CONTROLLER_BUTTON_B:
                breaking = not breaking
            if event.button == pygame.CONTROLLER_BUTTON_LEFTSHOULDER:
                leftKill = True
            if event.button == pygame.CONTROLLER_BUTTON_RIGHTSHOULDER:
                rightKill = True
    if(leftKill and rightKill):
        killSwitch = True
        arduino.write(b'KILL')
        break
    if(breaking != lastBreakState):
        arduino.write(breakMap[breaking])
    arduino.write(f"{joystickPos}\n".encode('utf-8'))
    time.sleep(0.05) 
