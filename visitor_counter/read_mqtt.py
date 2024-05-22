import paho.mqtt.client as paho
from time import sleep


#def send_command(cmd):
#	device.write(cmd.encode())
#
#	# connection.in_waiting = количество данных в буфере
#	return device.read(resp_len[cmd]).decode()


def handle_connect_fn(client, userdata, flags, rc):
	print("Connected")

def handle_message_fn(client, userdata, message):
	cmd = message.payload.decode()
	print("event:", cmd)

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
input("Press enter to exit")
client.disconnect()
client.loop_stop()
