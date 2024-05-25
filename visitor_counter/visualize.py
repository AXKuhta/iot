from matplotlib.dates import AutoDateLocator, DateFormatter
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

df = pd.read_csv("real_data.csv")

def v1():
	plt.figure(figsize=[16, 9], dpi=150)
	plt.step(df.time, df.x1 / 1000, label="x (m)")
	plt.step(df.time, df.y1 / 1000, label="y (m)")
	plt.step(df.time, df.v1 / 100, label="v (m/s)")
	plt.legend()
	plt.show()

def v2():
	kernel = [1/4]*4
	kernel = np.blackman(10) / np.blackman(10).sum()

	x = np.convolve(df.x1 / 1000, kernel, mode="same")
	y = np.convolve(df.y1 / 1000, kernel, mode="same")
	v = np.convolve(df.v1 / 100, kernel, mode="same")

	plt.figure(figsize=[16, 9], dpi=150)
	plt.step(df.time, x, label="x (m)")
	plt.step(df.time, y, label="y (m)")
	plt.step(df.time, v, label="v (m/s)")
	plt.legend()
	plt.show()

def v3():
	x = df.x1 / 1000
	y = df.y1 / 1000
	v = df.v1 / 100

	v_thresh_ms = 0.0
	x_thresh_m = 0.0

	v_ = np.roll(v, 1)
	x_ = np.roll(x, 1)
	zv = (v_ < -v_thresh_ms) * (v >= v_thresh_ms)
	zx_r = (x_ < -x_thresh_m) * (x >= x_thresh_m)
	zx_l = (x < -x_thresh_m) * (x_ >= x_thresh_m)

	df.time /= (60*60*24)

	fig, axes = plt.subplots(figsize=[16, 9], dpi=150)
	axes.step(df.time, x, label="x (m)")
	axes.step(df.time, y, label="y (m)")
	axes.step(df.time, v, label="v (m/s)")
	axes.step(df.time, zv, label="zv")
	axes.step(df.time, zx_r, label="zx_r")
	axes.step(df.time, zx_l, label="zx_l")
	plt.legend()
	axes.xaxis.set_major_formatter(DateFormatter("%Y-%m-%d %H:%M:%S"))
	fig.autofmt_xdate()
	plt.show()

def v4():
	x = df.x1 / 1000
	y = df.y1 / 1000
	v = df.v1 / 100

	v_thresh_ms = 0.0
	x_thresh_m = 0.0

	kernel = np.blackman(10) / np.blackman(10).sum()
	v = np.convolve(v, kernel, mode="same")
	#x = np.convolve(x, kernel, mode="same")
	#y = np.convolve(y, kernel, mode="same")

	v_ = np.roll(v, 1)
	x_ = np.roll(x, 1)
	zv = (v_ < -v_thresh_ms) * (v >= v_thresh_ms)
	zx_r = (x_ < -x_thresh_m) * (x >= x_thresh_m)
	zx_l = (x < -x_thresh_m) * (x_ >= x_thresh_m)

	df.time /= (60*60*24)

	fig, axes = plt.subplots(figsize=[16, 9], dpi=150)
	axes.step(df.time, x, label="x (m)")
	axes.step(df.time, y, label="y (m)")
	axes.step(df.time, v, label="v (m/s)")
	axes.step(df.time, zv, label="zv")
	axes.step(df.time, zx_r, label="zx_r")
	axes.step(df.time, zx_l, label="zx_l")
	plt.legend()
	axes.xaxis.set_major_formatter(DateFormatter("%Y-%m-%d %H:%M:%S"))
	fig.autofmt_xdate()
	plt.show()

def v5():
	time = df.time / (60*60*24)
	x = df.x1 / 1000
	y = df.y1 / 1000
	v = df.v1 / 100

	class StateMachine():
		def __init__(self):
			self.state = "IDLE" # ["IDLE", "ALERT"]
			self.up, self.down, self.left, self.right = [], [], [], []
			self.logs = [self.up, self.down, self.left, self.right]

		def advance(self, x, y):

			up = 0
			down = 0
			left = 0
			right = 0

			if self.state == "IDLE":
				if -.5 < x < .5 and .2 < y < 2.:
					self.state = "ALERT"
			elif self.state == "ALERT":
				if y < .2:
					self.state = "IDLE"
					up = 1
				elif y > 2:
					self.state = "IDLE"
					down = 1
				elif x < -.5:
					self.state = "IDLE"
					left = 1
				elif x > .5:
					self.state = "IDLE"
					right = 1

			self.up.append(up)
			self.down.append(down)
			self.left.append(left)
			self.right.append(right)

	machine = StateMachine()

	for x_, y_, v_ in zip(x, y, v):
		machine.advance(x_, y_)


	up, down, left, right = machine.logs

	fig, axes = plt.subplots(figsize=[16, 9], dpi=150)
	axes.step(time, x, label="x (m)")
	axes.step(time, y, label="y (m)")
	axes.step(time, v, label="v (m/s)")
	axes.step(time, up, label="up")
	axes.step(time, down, label="down")
	axes.step(time, left, label="left")
	axes.step(time, right, label="right")
	plt.legend()
	axes.xaxis.set_major_formatter(DateFormatter("%Y-%m-%d %H:%M:%S"))
	fig.autofmt_xdate()
	plt.show()


v5()
