import serial
import minimalmodbus
import time
import datetime

modb = minimalmodbus.Instrument(port="COM3",slaveaddress=0x69)
modb.serial.baudrate=115200
modb.serial.bytesize=8
modb.serial.parity=serial.PARITY_EVEN


data=modb.read_registers(0x0,10)
sec     =str(data[0])
min     =str(data[1])
hour    =str(data[2])
dow     =str(data[3])
date    =str(data[4])
month   =str(data[5])
year    =str(data[6])

print("stm says it is now: "+hour+":"+min+":"+sec+" ,"+month+"/"+date+"/"+year)

print("setting new time")

# print(datetime.datetime.now().year)

# exit()

modb.write_register(0x10,datetime.datetime.now().second,functioncode=6)
modb.write_register(0x11,datetime.datetime.now().minute,functioncode=6)
modb.write_register(0x12,datetime.datetime.now().hour,functioncode=6)
modb.write_register(0x13,datetime.datetime.now().weekday()+1,functioncode=6)
modb.write_register(0x14,datetime.datetime.now().day,functioncode=6)
modb.write_register(0x15,datetime.datetime.now().month,functioncode=6)
modb.write_register(0x16,datetime.datetime.now().year,functioncode=6)
modb.write_register(0x1F,1,functioncode=6)
time.sleep(1)
# data=modb.read_registers(0x10,10)
# sec     =str(data[0])
# min     =str(data[1])
# hour    =str(data[2])
# dow     =str(data[3])
# date    =str(data[4])
# month   =str(data[5])
# year    =str(data[6])

# print("stm set time is: "+hour+":"+min+":"+sec+" ,"+month+"/"+date+"/"+year)
# time.sleep(1)
data=modb.read_registers(0x0,10)
sec     =str(data[0])
min     =str(data[1])
hour    =str(data[2])
dow     =str(data[3])
date    =str(data[4])
month   =str(data[5])
year    =str(data[6])

print("stm new time is: "+hour+":"+min+":"+sec+" ,"+month+"/"+date+"/"+year)

