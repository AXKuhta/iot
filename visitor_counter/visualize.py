import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

df = pd.read_csv("sample_data.csv")

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

	plt.figure(figsize=[16, 9], dpi=150)
	plt.step(df.time, x, label="x (m)")
	plt.step(df.time, y, label="y (m)")
	plt.step(df.time, v, label="v (m/s)")
	plt.step(df.time, zv, label="zv")
	plt.step(df.time, zx_r, label="zx_r")
	plt.step(df.time, zx_l, label="zx_l")
	plt.legend()
	plt.show()

v3()
