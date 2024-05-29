from time import sleep, strftime, gmtime, struct_time
import paho.mqtt.client as paho
import struct
import json
import zlib

def handle_connect_fn(client, userdata, flags, rc):
	print("Connected")

broker = "broker.emqx.io"
port = 8883

client = paho.Client("axkuhta-pub-1")
client.on_connect = handle_connect_fn

client.tls_set("broker.emqx.io-ca.crt")
client.connect(broker, port)
client.loop_start()

def publish_mock_message():
	device_timestamp = strftime("%Y-%m-%d %H:%M:%S", struct_time(gmtime()))

	message = {
		"device_timestamp": device_timestamp,
		"u": 0,
		"d": 0,
		"l": 0,
		"r": 0
	}

	client.publish("axkuhta/esp32_ld2450_mmmm/visitors4", json.dumps(message))

while True:
	publish_mock_message()
	sleep(3)

input("Press enter to exit")
client.disconnect()
client.loop_stop()
