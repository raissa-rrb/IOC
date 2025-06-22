""" from paho.mqtt import client as mqtt_client """
import random

import time

from paho.mqtt import client as mqtt_client

""" using TCP connection
    setting the broker adress port

 """

broker = 'broker.emqx.io'
port = 1883
topic = "python/mqtt"
client_id = f'python-mqtt-{random.randint(0,100)}'
# username = 'emqx'
# password = 'public'

""" function to connect the broker """

def connect_mqtt() -> mqtt_client:
    def on_connect( client, userdata, flags, rc, properties):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)
        
    #Set connecting client id
    client = mqtt_client.Client(client_id=client_id, callback_api_version=mqtt_client.CallbackAPIVersion.VERSION2)

    #client.username_pw_set(username,password)
    client.on_connect = on_connect

    client.connect(broker , port)
    return client


def suscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
       print(f"message reçu: `{msg.payload.decode()}` from `{msg.topic}` topic")

       time.sleep(1)

    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    suscribe(client)
    client.loop_forever()

if __name__ == '__main__':
    run()





