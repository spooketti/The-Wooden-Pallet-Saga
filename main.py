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
brakeing = True

"""
Controls: 
leftstick drive accelerate forward/backward
b: toggle brake
both triggers: kill switch
"""

brakeMap = {True:b'BRAKE\n',
 False:b'UNBRAKE\n'}

while(not killSwitch):
    lastbrakeState = brakeing
    leftKill = False
    rightKill = False
    for event in pygame.event.get():
        if event.type == pygame.JOYAXISMOTION: #TODO: this has problems with the arduino deadman (0 is not an event most likely)
            #even though this script will most likely get converted into display data anyway cause of
            #arduino libraries for xbox input
            if event.axis == 1:
                print(event.value)
        if event.type == pygame.JOYBUTTONDOWN:
            if event.button == pygame.CONTROLLER_BUTTON_B:
                brakeing = not brakeing
            if event.button == pygame.CONTROLLER_BUTTON_LEFTSHOULDER:
                leftKill = True
            if event.button == pygame.CONTROLLER_BUTTON_RIGHTSHOULDER:
                rightKill = True
    if(leftKill and rightKill):
        killSwitch = True
        arduino.write(b'KILL\n')
        break
    if(brakeing != lastbrakeState):
        arduino.write(brakeMap[brakeing])
    arduino.write(f"{joystickPos}\n".encode('utf-8'))
    time.sleep(0.05) 
