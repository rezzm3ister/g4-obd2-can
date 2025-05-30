import serial
import minimalmodbus
from tqdm import tqdm
import datetime
import time

modb = minimalmodbus.Instrument(port="COM3",slaveaddress=0x69)
modb.serial.baudrate=500000
modb.serial.bytesize=8
modb.serial.parity=serial.PARITY_EVEN
TRIES=1000
times = []
intertime=[]
tstart2=time.time()
for i in tqdm(range(TRIES)):
# for i in range(TRIES):
    tstart=time.time()
    dat = modb.read_registers(0x0,125)
    tend=time.time()
    times.append(tend-tstart)
    # print(tend-tstart, dat)

print("Average time for 1000 reads: "+str(sum(times)/TRIES))
print("Max time for 1000 reads: "+str(max(times))) 
print("Min time for 1000 reads: "+str(min(times)))
print("Total time for 1000 reads: "+str(sum(times)))

