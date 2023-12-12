from sklearn.linear_model import LinearRegression
import matplotlib.pyplot as plt
import numpy as np

x = [10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30]
y = [
	24733,
	23807,
	21412,
	18573,
	17172,
	18306,
	16535,
	15678,
	12687,
	12025,
	10761,
	10945,
	11073,
	11742,
	10905,
	10758,
	10303,
	9910,
	9273,
	9236,
	9490
]

# Усреднение!
# Исходные данные нужно разделить на 50
y = [val/50 for val in y]

# Подогнать полином третьей степени
poly_x = [[v**3, v**2, v] for v in x]

model = LinearRegression()
model.fit(poly_x, y)
print(model.score(poly_x, y))

y_pred = model.predict(poly_x)

plt.plot(x, y, label="Measured (avg of 50)")
plt.plot(x, y_pred, label="Model")
plt.ylabel("ADC value")
plt.xlabel("Distance (cm)")
plt.legend()
plt.show()
