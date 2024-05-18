from itertools import permutations

states = [0, 1, 2, 3, 4, 5, 6, 7]

for a, b in permutations(states, 2):
	print(a, b)

for a, b, c in permutations(states, 3):
	print(a, b, c)

for a, b, c, d in permutations(states, 4):
	print(a, b, c, d)
