import serial
import minimalmodbus

import datetime

modb = minimalmodbus.Instrument(port="COM3",slaveaddress=0x69)
modb.serial.baudrate=115200
modb.serial.bytesize=8
modb.serial.parity=serial.PARITY_EVEN


print(datetime.datetime.now())
data=modb.read_registers(0x0,20)
sec     =str(data[0])
min     =str(data[1])
hour    =str(data[2])
dow     =str(data[3])
date    =str(data[4])
month   =str(data[5])
year    =str(data[6])

print(data)
print("stm says it is now: "+hour+":"+min+":"+sec+","+dow+", "+month+"/"+date+"/"+year)
