import serial
import serial.tools.list_ports
import sys, time, math
import msvcrt
import threading 

global count
global serial_port

def read():
        global count
        serial_strin = serial_port.readline(1)
        serial_strin = serial_strin.decode()
        if serial_strin == '1':
            count = count + 1
            if count == 1:
                print("you have collected ", count, " coin")
            elif count == 20:
                print("congrats you are rich")
            else:
                print("you have collected ", count, " coins")
        elif serial_strin == '2':
            print("EDGE DETECTED: CHECK ROBOT")
def write():
    choice = input()
    saved_choice = choice
    choice = choice.encode("ASCII")
    serial_port.write(choice)
    if saved_choice == 'c' or saved_choice == '1':
         while 1:
              read()

if __name__ == '__main__':
    try:
        ser.close() # try to close the last opened port
    except:
        print('')
    portlist=list(serial.tools.list_ports.comports())
    for p in portlist:
        print(p)
    serial_port = serial.Serial(
        port='COM9',
        baudrate=9600,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        xonxoff=0
    )
    count = 0

    print("ready to rip")
    write()
    while True:
            write()



