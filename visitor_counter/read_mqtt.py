import paho.mqtt.client as paho
from time import sleep
import zlib

#def send_command(cmd):
#	device.write(cmd.encode())
#
#	# connection.in_waiting = количество данных в буфере
#	return device.read(resp_len[cmd]).decode()


def handle_connect_fn(client, userdata, flags, rc):
	print("Connected")

def handle_message_fn(client, userdata, message):
	payload = message.payload
	topic = message.topic

	if topic.endswith("sensors"):
		print("visitors:", payload.decode())
	elif topic.endswith("rawdata"):
		try:
			rawdata = zlib.decompress(payload)
			print("rawdata:\n", rawdata.decode())
			print(len(payload), "bytes gzip;", len(rawdata), "bytes plaintext")
		except Exception as e:
			print("rawdata: malformed", len(payload))
	elif topic.endswith("telemetry"):
		print("telemetry:", payload.decode())
	else:
		assert 0

#broker = "broker.hivemq.com"
broker = "broker.emqx.io"
port = 8883

client = paho.Client("axkuhta-sub-1")
client.on_message = handle_message_fn
client.on_connect = handle_connect_fn

client.tls_set("broker.emqx.io-ca.crt")
client.connect(broker, port)
client.loop_start()
print("Subscribing")
client.subscribe("axkuhta/esp32_ld2450_eed5/sensors")
client.subscribe("axkuhta/esp32_ld2450_eed5/rawdata")
client.subscribe("axkuhta/esp32_ld2450_eed5/telemetry")
input("Press enter to exit")
client.disconnect()
client.loop_stop()
