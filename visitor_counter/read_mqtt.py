import paho.mqtt.client as paho
from time import sleep
import struct
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

	print(topic, end=": ")

	if topic.endswith("sensors"):
		print("visitors:", payload.decode())
	elif topic.endswith("rawdata"):
		try:
			rawdata = zlib.decompress(payload)
			assert len(rawdata) % 32 == 0
			frames = len(rawdata) // 32

			for i in range(frames):
				frame = rawdata[32*i:32*i+32]
				x1, y1, v1, r1, x2, y2, v2, r2, x3, y3, v3, r3, t = struct.unpack("hhhhhhhhhhhhd", frame)
				print(t, x1, y1, v1, r1, x2, y2, v2, r2, x3, y3, v3, r3, sep=",")

			print(len(payload), "bytes gzip;", len(rawdata), "bytes raw")
		except Exception as e:
			print("rawdata: malformed", len(payload), e)
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

# axkuhta/esp32_ld2450_eed5/sensors
print("Subscribing")
client.subscribe("axkuhta/+/sensors")
client.subscribe("axkuhta/+/rawdata")
client.subscribe("axkuhta/+/telemetry")
input("Press enter to exit")
client.disconnect()
client.loop_stop()
