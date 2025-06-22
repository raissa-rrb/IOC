from paho.mqtt import client as mqtt

MQTT_BROKER = "localhost"
MQTT_TOPIC = "led"

def on_connect( client, userdata, flags, rc):
    if( rc == 0 ):
        print("publish to :\"", MQTT_TOPIC, "\"")
    else:
        print("bad connection to :\"", MQTT_TOPIC, "\"")

def on_publish(client, userdata, mid):
    print("message sent ! check the led\n")

client = mqtt.client()
client.on_connect = on_connect
client.on_publish = on_publish

client.connect(MQTT_BROKER)

while True :
    message = input("enter msg to publish:")

    client.publish(MQTT_TOPIC, message)
    print("sent\n")
        
