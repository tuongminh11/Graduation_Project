from datetime import datetime


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

frame = create_frame(0x17, [0x00, 0x00, 0x01, 0x01])

print(f"{datetime.now()} - Sent: " + " ".join(hex(n) for n in frame))