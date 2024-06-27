import serial
import time
from datetime import datetime

connectors = [0, 0] # connected or not
users = [255, 255]  # belong to which connector
status = [0, 0]; #transaction in connect or not
def EVSE_state():
    print("Connector:\t", connectors)
    print("Status:\t\t",status)
    print("Users:\t\t",users)

def calculate_checksum(frame):
    return sum(frame[:-1]) & 0xFF

def create_frame(file_code, data):
    if len(data) != 4:
        raise ValueError("Data must be exactly 4 bytes long")
    
    frame = bytearray(8)
    frame[0] = 0xAB
    frame[1] = 0xCD
    frame[2] = file_code
    frame[3:7] = data
    frame[7] = calculate_checksum(frame)
    return frame

def send_frames(ser, file_code, data):
    frame = create_frame(file_code, data)
    ser.write(frame)
    print(f"{datetime.now()} - Sent: " + " ".join(hex(n) for n in frame))
    time.sleep(2)  # Thời gian nghỉ giữa các frame

def beginCharge(ser, user_index):
    send_frames(ser, 0x17, [0x00, 0x00, users[user_index], 0x01]) #plug connector
    connectors[users[user_index]-1] = 1 #connector
    # send_frames(ser, 0x23, [0x00, 0x00, 0x01, users[user_index]]) #send Current Value Request
    # send_frames(ser, 0x24, [0x00, 0x00, 0x01, users[user_index]]) #send Voltage Value Request 
    # send_frames(ser, 0x25, [0x00, 0x00, 0x01, users[user_index]]) #send InitSoC
    # send_frames(ser, 0x26, [0x00, 0x00, 0x01, users[user_index]]) #send SoC Before Charge
    # send_frames(ser, 0x27, [0x00, 0x00, 0x01, users[user_index]]) #send SoH
    # send_frames(ser, 0x28, [0x00, 0x00, 0x01, users[user_index]]) #send SoKM
    send_frames(ser, 0x18, [0x00, 0x00, users[user_index], 0x01]) #send start transaction    

def inCharge(ser, connector):
    send_frames(ser, 0x20, [0x00, 0x00, 0x3C, connector]) #send Current Value On Charge
    send_frames(ser, 0x21, [0x00, 0x02, 0x58, connector]) #send Voltage Value On Charge
    # send_frames(ser, 0x22, [0x3A, 0x00, 0x01, connector]) #send Battery Temperature And SoC On Charge
    # send_frames(ser, 0x28, [0x00, 0x00, 0x01, connector]) #send SoKM  

def receive_frames(ser):
    while True:
        received_frame = ser.read(8)
        if(status[0] == 1):
            inCharge(ser, 1)
        if(status[1] == 1):
            inCharge(ser, 2)
        if len(received_frame) == 8:
            if received_frame[7] == calculate_checksum(received_frame):
                print(f"{datetime.now()} - Received: " + " ".join(hex(n) for n in received_frame))
                if(received_frame[2] == 0x15): #idtag
                    EVSE_state()
                    if (received_frame[5] == 255 or status[received_frame[5]-1] == 0):
                        #đại diện cho rút súng sạc
                        if(users[0] == 255):
                            users[0] = 2
                        elif(users[1] == 255):
                            users[1] = 1
                        # users[received_frame[4]] = received_frame[4] + 1 # imagine users 1 plug to connector 1
                        beginCharge(ser, received_frame[4])
                        EVSE_state()
                    elif(status[received_frame[5]-1] == 1):
                        print("Send Stop")
                        send_frames(ser, 0x18, [0x00, 0x00, received_frame[5], 0x00]) #send stop transaction
                elif(received_frame[2] == 0x1B): #confirm begin/end
                    if(received_frame[3]): #begin
                        status[received_frame[5]-1] = 1
                        time.sleep(5)
                        send_frames(ser, 0x15, [0x00, 0x00, received_frame[5], 0x00]) #send idtag
                        EVSE_state()

                    else: #end
                        #reset global var
                        status[received_frame[5]-1] = 0
                        print("End transaction")
                        print(received_frame[5])
                        send_frames(ser, 0x15, [0x00, 0x00, received_frame[5], 0x00]) #send idtag/logout
                        send_frames(ser, 0x17, [0x00, 0x00, received_frame[5], 0x00]) #unplug connector
                        connectors[received_frame[5]-1] = 0
                        if(users[0] == received_frame[5]):
                            users[0] = 255
                        elif(users[1] == received_frame[5]):
                            users[1] = 255
                        EVSE_state()

                
            else:
                print(f"{datetime.now()} - Checksum is incorrect")
        else:
            continue

def main():
    # Thay đổi tên cổng COM cho phù hợp với hệ thống của bạn
    port = 'COM8'  # Hoặc COMx trên Windows, ví dụ: 'COM3'
    baudrate = 115200

    ser = serial.Serial(port, baudrate, timeout=1)
    time.sleep(2)  # Chờ một chút để cổng serial sẵn sàng

    try:
        receive_frames(ser)
    except Exception as e:
        print(f"Error: {e}")

    finally:
        ser.close()

if __name__ == "__main__":
    main()
#Backtrace: 0x4008bcb7:0x3ffd88b0 0x400d68e5:0x3ffd88c0 0x400d3911:0x3ffd88e0 0x400d3b35:0x3ffd8910 0x400d3eee:0x3ffd8960
  #0  0x4008bcb7 in strcmp at /builds/idf/crosstool-NG/.build/HOST-x86_64-w64-mingw32/xtensa-esp32-elf/src/newlib/newlib/libc/machine/xtensa/strcmp.S:470
  #1  0x400d68e5 in endTransaction(char const*, char const*, unsigned int) at .pio/libdeps/nodemcu-32s/MicroOcpp/src/MicroOcpp.cpp:383 (discriminator 2)
  #2  0x400d3911 in hmiTranControl(unsigned char*) at src/main.cpp:231
  #3  0x400d3b35 in process(unsigned char*) at src/main.cpp:310
  #4  0x400d3eee in CIMS_handle(void*) at src/main.cpp:515