from paho.mqtt import client as mqtt

MQTT_BROKER = "localhost"
MQTT_TOPIC = "topic"

def on_connect( client, userdata, flags, rc):
  if (rc == 0):
    client.subscribe(MQTT_TOPIC)
    print("subscribe to :\"", MQTT_TOPIC, "\"")
    print("cleaning file...")
    open("stockage.txt","w")

  else : 
    print("bad connection to :\"", MQTT_TOPIC, "\"")

def on_message(client, userdata, msg):
  print("message : \"", str(msg.payload.decode("utf-8")), "\"")
  fichier = open("stockage.txt","a")
  fichier.write( str(msg.payload.decode("utf-8")))
  fichier.write("\n")
  fichier.close()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER)
client.loop_forever()
