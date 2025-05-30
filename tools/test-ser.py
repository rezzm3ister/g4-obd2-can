import serial
import minimalmodbus

def printhex(inlist:list):
    for i in inlist:
        print(hex(i),end=" ")

# ser=serial.Serial(port="COM3",baudrate=115200,bytesize=8,parity=serial.PARITY_EVEN,timeout=2)


# ser.write([1,2,3,4,5,6,7,8,9,10,11])

# printhex(list(ser.read(300)))

modb = minimalmodbus.Instrument(port="COM3",slaveaddress=0x69)
modb.serial.baudrate=500000
modb.serial.bytesize=8
modb.serial.parity=serial.PARITY_EVEN



# modb.serial.write([1,2,3,4,5,6,7,8,9,10,11])
# printhex(modb.serial.read(100))
# modb.write_register(0x3,33,functioncode=6)
# modb.write_registers(0x5,[69,69,69])
for i in range(1):
    print(list(modb.read_registers(0x200,100)))
    # print(modb.read_registers(0x20,3))
    # modb.write_register(0x30,1)