import serial
import minimalmodbus

def printhex(inlist:list):
    for i in inlist:
        print(hex(i),end=" ")


modb = minimalmodbus.Instrument(port="COM3",slaveaddress=0x69)
modb.serial.baudrate=500000
modb.serial.bytesize=8
modb.serial.parity=serial.PARITY_EVEN

for i in range(0x100):
    if(modb.read_register(i)):
        print(i,"supported")