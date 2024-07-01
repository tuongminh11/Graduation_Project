maxKM = 300

def intoBytes(data):
    return data.to_bytes(3, byteorder='little')

print(" ".join(hex(n) for n in intoBytes(maxKM)))