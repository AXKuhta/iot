import paho.mqtt.client as paho
from time import time, sleep
import sys

if len(sys.argv) != 2:
	print("Usage: python3 pub_daytime_adaptive.py topic")
	print("Example: python3 pub_daytime_adaptive.py esp8266_1b9b/command")
	exit()

topic = sys.argv[1]

start = time()

broker = "broker.emqx.io"
client = paho.Client("axkuhta-1")
client.connect(broker)
client.loop_start()
print("Publishing")

daystart = 20
dayend = 40

start_offset =	[0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 6]
end_offset = 	[0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 5]
i = 0

while True:
	sec_elapsed = (time() - start) // 1
	day = sec_elapsed % 60
	i = int((sec_elapsed // 60) % 11)

	s = start_offset[i]
	e = end_offset[i]

	state = "e" if (dayend - e) >= day >= (daystart + s) else "d"
	print(f"time is {day}; state is {state}; year offset {i}")
	client.publish(topic, state)

	sleep(1)

client.disconnect()
client.loop_stop()
