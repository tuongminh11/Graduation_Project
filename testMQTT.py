import paho.mqtt.client as mqtt
import json
import time
from random import randint

# MQTT Broker Configuration
MQTT_USERNAME = "wallnut"
MQTT_PASSWORD = "wallnut"
MQTT_BROKER = "183.80.8.132"
MQTT_PORT = 8083
MQTT_KEEPALIVE_INTERVAL = 60

# MQTT Topics
USER_NAME = "kieubaduy"
TOPIC_HEAD = "evse_service/" + USER_NAME
TOPIC_BATTERY_INFO = TOPIC_HEAD + "1/SoH" 
TOPIC_CHARGING_INFO = TOPIC_HEAD + "1/ChargingInfo"
TOPIC_SoC = TOPIC_HEAD + "1/SoC"
TOPIC_SoH = TOPIC_HEAD + "1/SoH"
TOPIC_InitSoC = TOPIC_HEAD + "1/InitSoC"
TOPIC_InitU = TOPIC_HEAD + "1/InitU"
TOPIC_InitI = TOPIC_HEAD + "1/InitI"
TOPIC_U = TOPIC_HEAD + "1/U"
TOPIC_I = TOPIC_HEAD + "1/I"
TOPIC_AUTH = TOPIC_HEAD + "1/EVSE_auth"
TOPIC_CONNECT_GUN = TOPIC_HEAD + "1/EVSE_connector"
TOPIC_START_CHARGE = TOPIC_HEAD + "1/EVSE_charging" 
TOPIC_SoT = TOPIC_HEAD + "1/SoT"
TOPIC_SoKM = TOPIC_HEAD + "1/SoKM"

# Initialize MQTT Client
client = mqtt.Client(transport='websockets')

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print(f"Failed to connect, return code {rc}")

def on_message(client, userdata, msg):
    print(f"Received message from {msg.topic}: {msg.payload.decode()}")

client.on_connect = on_connect
client.on_message = on_message

#set un pw
client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

# Connect to MQTT Broker
client.connect(MQTT_BROKER, MQTT_PORT, MQTT_KEEPALIVE_INTERVAL)

# Function to publish messages
def publish(topic, message):
    client.publish(topic, json.dumps(message))
    print(f"Published to {topic}: {message}")

# global sửa chô này
SoH = 90
SoC = 30
InitSoC = 100
InitU = 600
InitI = 20
U = randint(570, 600)
I = randint(18, 25)
SoT = randint(40, 50)
SoKM = 300 * (SoC/InitSoC)


# gen init value
def gen_inti_data():
    publish(TOPIC_SoH, {"SoH": SoH})
    publish(TOPIC_SoC, {"SoC": SoC})
    publish(TOPIC_InitSoC, {"InitSoC": InitSoC})
    publish(TOPIC_InitU, {"InitU": InitU})
    publish(TOPIC_InitI, {"InitI": InitI})

# Function to generate and publish fake data
def generate_fake_data():
    U = randint(570, 600)
    I = randint(18, 25)
    SoT = randint(40, 50)
    publish(TOPIC_SoC, {"SoC": SoC})
    publish(TOPIC_U, {"U": U})
    publish(TOPIC_I, {"I": I})
    publish(TOPIC_SoT, {"SoT": SoT})
    publish(TOPIC_SoKM, {"SoKM": SoKM})

# Main Loop to send fake data periodically
if __name__ == "__main__":
    client.loop_start()
    try:
        while True:
            beginLoop = int(input())

            if beginLoop == 1:
                print("begin simulation")
                publish(TOPIC_CONNECT_GUN, {"EVSE_connector": bool(1)})
                time.sleep(2) # sửa time stamp này

            elif beginLoop == 2:    
                gen_inti_data()
                time.sleep(2) 

            elif beginLoop == 3: 
                publish(TOPIC_START_CHARGE, {"EVSE_charging": True})
                time.sleep(2) 

            elif beginLoop == 4: 
                while True:
                    if SoC + 10 > SoH:
                        SoC = SoH
                        SoKM = 300 * (SoC/InitSoC)
                        generate_fake_data()
                        
                        #reset var
                        SoH = 90
                        SoC = 30
                        InitSoC = 100
                        InitU = 600
                        InitI = 20
                        SoKM = 300 * (SoC/InitSoC)

                        break
                    else:
                        SoC += 10
                        SoKM = 300 * (SoC/InitSoC)
                        generate_fake_data()
                        time.sleep(2)  

            elif beginLoop == 5: 
                publish(TOPIC_START_CHARGE, {"EVSE_charging": bool(0)})
                publish(TOPIC_CONNECT_GUN, {"EVSE_connector": bool(0)})
                time.sleep(2) 
    except KeyboardInterrupt:
        print("Stopping...")
    finally:
        client.loop_stop()
        client.disconnect()
