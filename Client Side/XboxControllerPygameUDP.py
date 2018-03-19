#Nasa-bot drive code written in pygame using UDP
import time
import socket
import pygame

"""Packet Meanings:
        DR = Drive
        ST = Steer(Actuator)
        AU = Auger
        TI = Tilt(Actuators)
        SL = BallsScrew Slide
        CO = Conveyor
        """

def stop():
        pygame.joystick.quit()
        pygame.quit()
        print("Clean exit")


def main():
    print("main")
    #Control upadate frequency
    CLOCK = pygame.time.Clock()
    clock_speed = 20

    #vars for if joystick is connected and program is running
    joystick_connect = True
    running = True

    #Create UDP socket connection
    #HOST = '192.168.1.153'
    HOST = '192.168.1.73'
    PORT = 5005
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    receiveSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    receiveSocket.bind((HOST, PORT))
    
    #initialize pygame to handle the xbox controller
    pygame.init()
    #use first joystick connected since only
    #one xbox remote is used
    joystick = pygame.joystick.Joystick(0)
    print("joystick")
    #initailize joystick
    joystick.init()
    #set initial values of the drive motors to off
    commandLF = 0
    commandRI = 0
    data = ""
    #Bools for motion
    SlR = True
    SlL = True
    #counter for pause message
    PACounter = 1
    while(running):
        for event in pygame.event.get():
            if event.type == pygame.JOYBUTTONDOWN or event.type == pygame.JOYAXISMOTION:
              
                #Left side tank drive
                if joystick.get_axis(1) > .1 or joystick.get_axis(1) < -.1:
                    commandLF = joystick.get_axis(1)*1000
                    commandLF = int(commandLF)
                    if commandLF > 900: 
                        commandLF = 900
                    elif commandLF < -900:
                        commandLF = -900
                    Str="LF"
                    Send(commandLF,s,Str)
                    #it sent but won't continue running
                    while(len(data)==0):
                            data, HOST = receiveSocket.recvfrom(8)
                            print("Here",data)
                    
                elif joystick.get_axis(1) < .1 and joystick.get_axis(1) > -.1 and commandLF != 0:
                    commandLF = 0
                    Str="LF"
                    Send(commandLF,s,Str)
                    
                #Right side tank drive
                if joystick.get_axis(3) > .1 or joystick.get_axis(3) < -.1:
                    commandRI = joystick.get_axis(3)*1000
                    commandRI = int(commandRI)
                    if commandRI > 900: 
                        commandRI = 900
                    elif commandRI < -900:
                        commandRI = -900
                    Str="RI"
                    Send(commandRI,s,Str)
                elif joystick.get_axis(3) < .1 and joystick.get_axis(3) > -.1 and commandRI != 0:
                    commandRI = 0
                    Str="RI"
                    Send(commandRI,s,Str)
                    

                #Conveyor Belt control using the X button
                if joystick.get_button(2) != 0:
                    Str = "CO"
                    Send(900,s,Str)

               #Auger control using the A button (Drill direction)
                if joystick.get_button(0) != 0:
                    Str = "AU"
                    Send(100,s,Str)
                else:
                    Str = "AU"
                    Send(0,s,Str)

                #Auger control using B button (Opposite direction)
                if joystick.get_button(1) != 0:
                    Str = "AU"
                    Send(-100,s,Str)

                #Ballscrew slide using the left stick Left
                if joystick.get_axis(2) > .1:
                    Str = "SL"
                    Send(700,s,Str)
                    SlL = True
                elif joystick.get_axis(2) < .1 and SlL:
                    SlL = False
                    Str = "SL"
                    Send(0,s,Str)

                #Ballscrew slide using right stick Right
                if joystick.get_axis(2) < -.1:
                    Str = "SL"
                    Send(-700,s,Str)
                    SlR = True
                elif joystick.get_axis(2) > -.1 and SlR:
                    Str = "SL"
                    SlR = False
                    Send(0,s,Str)

                #Tilt using left bumper
                if joystick.get_button(4) != 0:
                    Str = "TI"
                    Send(-900,s,Str)

                #Tilt using right bumper
                if joystick.get_button(5) != 0:
                    Str = "TI"
                    Send(900,s,Str)

                #Exit program if start button is pressed
                if joystick.get_button(7):
                    running = False
                    Str="QU"
                    commandRI = 0
                    Send(commandLF,s,Str)
                    Send(commandRI,s,Str)
                    print ("Stopping")
                #Pause the program when select is pressed
                if joystick.get_button(6):
                    Str = "PA"
                    if PACounter % 2 == 1:
                        print ("Paused. press select again to unpause")
                    else:
                        print ("Unpausing . . . ")
                    counter+=1
                    Send(50,s,Str)

    s.close()
    stop()
def Send(command,s,Str):
    HOST = '192.168.1.80'
    PORT = 5005
    msg = Str + str(command)
    print(msg)
    send = msg.encode()
    s.sendto(send,(HOST,PORT))



main()