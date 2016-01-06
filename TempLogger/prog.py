import serial,time
ser = serial.Serial(2, baudrate=9600)

print(ser.name)

file = open("Output.txt","w")
ser.write(b"M04\n")
ser.flush()
ser.readline().decode();
t = time.clock()

while t <= 180:
    ser.write(b"M105\n")
    ser.flush()
    resp = ser.readline().decode();
    t = time.clock()
    resp = resp[resp.find('T')+1:].split()[0]
    print(str(t) + " " + resp)
    file.write(str(t) + " " + resp + "\n")
    file.flush()
    time.sleep(0.1)
    

ser.write(b"M03\n")
ser.flush()
ser.close()
input()