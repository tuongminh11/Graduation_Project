import serial
import time
from datetime import datetime

connectors = [0, 0] # connected or not
users = [255, 255]  # belong to which connector
status = [0, 0]; #transaction in connect or not
InitSoC = [88, 60]
SoC = [20, 44]
maxKM = [300, 200]
currentKM = [round(maxKM[0] * SoC[0] / InitSoC[0]), round(maxKM[1] * SoC[1] / InitSoC[1])]

def intoBytes(data):
    return data.to_bytes(3, byteorder='little')

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

def beginCharge(ser, user_index):
    c_id = users[user_index]
    send_frames(ser, 0x17, [0x00, 0x00, c_id, 0x01]) #plug connector
    time.sleep(2)  # Thời gian nghỉ giữa các frame

    connectors[c_id-1] = 1 #connector
    send_frames(ser, 0x23, [0x3A, 0x00, 0x00, c_id]) #send Current Value Request
    send_frames(ser, 0x24, [0x49, 0x02, 0x00, c_id]) #send Voltage Value Request 
    send_frames(ser, 0x25, [0x00, intoBytes(InitSoC[c_id-1])[0], intoBytes(InitSoC[c_id-1])[1], c_id]) #send InitSoC
    send_frames(ser, 0x26, [0x00, intoBytes(SoC[c_id-1])[0], intoBytes(SoC[c_id-1])[1], c_id]) #send SoC Before Charge
    send_frames(ser, 0x27, [0x00, intoBytes(InitSoC[c_id-1])[0], intoBytes(InitSoC[c_id-1])[1], c_id]) #send SoH
    send_frames(ser, 0x28, [0x00, intoBytes(currentKM[c_id-1])[0], intoBytes(currentKM[c_id-1])[1], c_id]) #send SoKM

    time.sleep(2)  # Thời gian nghỉ giữa các frame
    send_frames(ser, 0x18, [0x00, 0x00, c_id, 0x01]) #send start transaction    
    time.sleep(2)  # Thời gian nghỉ giữa các frame

def inCharge(ser, connector):
    send_frames(ser, 0x20, [0x3C, 0x00, 0x00, connector]) #send Current Value On Charge
    send_frames(ser, 0x21, [0x58, 0x02, 0x00, connector]) #send Voltage Value On Charge
    send_frames(ser, 0x22, [0x38, intoBytes(SoC[connector-1])[0], intoBytes(SoC[connector-1])[1], connector]) #send Battery Temperature And SoC On Charge
    send_frames(ser, 0x28, [0x00, intoBytes(currentKM[connector-1])[0], intoBytes(currentKM[connector-1])[1], connector]) #send SoKM
    if(SoC[connector-1] + 10 <= InitSoC[connector-1]):
        SoC[connector-1] += 10
    else:
        SoC[connector-1] = InitSoC[connector-1]

    currentKM[connector-1] = round(maxKM[connector-1] * SoC[connector-1] / InitSoC[connector-1])
    time.sleep(5)  # Thời gian nghỉ giữa các frame

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
                        time.sleep(2)  # Thời gian nghỉ giữa các frame

                elif(received_frame[2] == 0x1B): #confirm begin/end
                    if(received_frame[3]): #begin
                        status[received_frame[5]-1] = 1
                        send_frames(ser, 0x15, [0x00, 0x00, received_frame[5], 0x00]) #send idtag
                        time.sleep(2)
                        EVSE_state()

                    else: #end
                        #reset global var
                        status[received_frame[5]-1] = 0
                        print("End transaction")
                        print(received_frame[5])
                        send_frames(ser, 0x15, [0x00, 0x00, received_frame[5], 0x00]) #send idtag/logout
                        time.sleep(2)  # Thời gian nghỉ giữa các frame
                        send_frames(ser, 0x17, [0x00, 0x00, received_frame[5], 0x00]) #unplug connector
                        time.sleep(2)  # Thời gian nghỉ giữa các frame

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
