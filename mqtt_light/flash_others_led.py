import paho.mqtt.client as paho
import time

#broker = "broker.hivemq.com"
broker = "broker.emqx.io"
client = paho.Client("axkuhta-pub-1")
client.connect(broker)
client.loop_start()
print("Publishing")

while True:
	client.publish("sergei/mcu/led", "u")
	time.sleep(1)
	client.publish("sergei/mcu/led", "d")
	time.sleep(1)

client.disconnect()
client.loop_stop()
