import random
import time

from paho.mqtt import client as mqtt_client

broker = 'broker.emqx.io'
port = 1883
topic = "python/mqtt"
client_id = f'python-mqtt-{random.randint(0,1000)}'
# username = 'emqx'
# password = 'public'


""" function to connect the broker """

def connect_mqtt():
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

def publish(client):
    msg_count = 1

    while True:
        time.sleep(1)
        msg = f"messages:{msg_count}"
        result = client.publish(topic , msg)

        #result : [0,1]
        status = result[0]
        if status == 0:
            print(f"Send `{msg}` to topic `{topic}`")
        msg_count += 1

        if msg_count > 5:
            break

def run():
    client = connect_mqtt()
    client.loop_start()
    publish(client)
    client.loop_stop()

if __name__ == '__main__':
    run()
