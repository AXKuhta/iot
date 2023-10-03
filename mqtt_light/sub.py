import paho.mqtt.client as paho
from serial import Serial # pyserial?
from time import sleep

# Windows = COM4
# Linux = /dev/ttyUSB0
device = Serial("/dev/ttyUSB0", baudrate=115200, timeout=10)

resp_len = {
	'd': len("led off\n"),
	'u': len("led on\n")
}

def send_command(cmd):
	device.write(cmd.encode())

	# connection.in_waiting = количество данных в буфере
	return device.read(resp_len[cmd]).decode()


def handle_message_fn(client, userdata, message):
	cmd = message.payload.decode()
	print("Received command:", cmd)
	send_command(cmd)

#broker = "broker.hivemq.com"
broker = "broker.emqx.io"
client = paho.Client("axkuhta-sub-1")
client.on_message = handle_message_fn

client.connect(broker)
client.loop_start()
print("Subscribing")
client.subscribe("axkuhta/mcu/led")
input("Press enter to exit")
client.disconnect()
client.loop_stop()
