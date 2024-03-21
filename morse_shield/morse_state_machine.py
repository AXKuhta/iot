
class Decoder():
	def __init__(self):
		alpha1 = {
			0b0: 'E',
			0b1: 'T'
		}

		alpha2 = {
			0b01: 'A',
			0b00: 'I',
			0b11: 'M',
			0b10: 'N'
		}

		alpha3 = {
			0b100: 'D',
			0b110: 'G',
			0b101: 'K',
			0b111: 'O',
			0b010: 'R',
			0b000: 'S',
			0b001: 'U',
			0b011: 'W',
		}

		alpha4 = {
			0b1000: 'B',
			0b1010: 'C',
			0b0010: 'F',
			0b0000: 'H',
			0b0111: 'J',
			0b0100: 'L',
			0b0110: 'P',
			0b1101: 'Q',
			0b0001: 'V',
			0b1001: 'X',
			0b1011: 'Y',
			0b1100: 'Z'
		}

		self.tables = [None, alpha1, alpha2, alpha3, alpha4]
		self.value = 0
		self.count = 0

	def dot(self):
		self.value = (self.value << 1)
		self.count += 1

	def dash(self):
		self.value = (self.value << 1) + 1
		self.count += 1

	def flush(self):
		table = self.tables[self.count]
		print(table[self.value], end="")
		self.value = 0
		self.count = 0

class Receiver():
	def __init__(self):
		self.decoder = Decoder()
		self.mode = "DISCONNECT" # ["IDLE", "SPACE", "MARK", "DISCONNECT"]
		self.count = 0
		self.log = []

	def transition_to_idle(self):
		self.mode = "IDLE"
		self.count = 0
		self.log = []

	def transition_to_space(self):
		self.mode = "SPACE"
		self.count = 0

	def transition_to_mark(self):
		self.mode = "MARK"
		self.count = 0

	def transition_to_disconnect(self):
		self.mode = "DISCONNECT"
		self.count = 0

	def advance(self, input):
		if input == "1":
			if self.mode == "IDLE":
				self.count += 1
				return

			# 2 TU HIGH			FLUSH LETTER
			# >2 TU HIGH		FLUSH WORD
			if self.mode == "SPACE":
				self.count += 1
				if self.count > 1:
					print("\nEvent: transaction end", self.log)
					self.transition_to_idle()
				elif self.count == 1:
					self.decoder.flush()

			if self.mode == "MARK":
				self.log.append(f"{self.count} mark")
				if self.count == 1:
					self.decoder.dot()
				elif self.count == 2:
					self.decoder.dash()
				else:
					raise Exception("Invalid mark")
				self.transition_to_space()

			if self.mode == "DISCONNECT":
				print("Event: line connected")
				self.transition_to_idle()

		elif input == "0":
			if self.mode == "IDLE":
				print("Event: transaction start")
				self.transition_to_mark()

			if self.mode == "SPACE":
				self.log.append(f"{self.count+1} space")
				self.transition_to_mark()

			if self.mode == "MARK":
				self.count += 1
				if self.count > 4:
					print("Event: line disconnected")
					self.transition_to_disconnect()

			if self.mode == "DISCONNECT":
				self.count += 1

		else:
			raise Exception("Advance with 1 or 0 str")

def test_a():
	recv = Receiver()
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("0")
	recv.advance("1")
	recv.advance("0")
	recv.advance("0")
	recv.advance("1")
	recv.advance("1")
	recv.advance("0")
	recv.advance("0")
	recv.advance("1")
	recv.advance("0")
	recv.advance("0")
	recv.advance("1")
	recv.advance("0")
	recv.advance("1")
	recv.advance("0")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")

def test_b():
	recv = Receiver()
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("0")
	recv.advance("1")
	recv.advance("0")
	recv.advance("0")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("0")
	recv.advance("0")
	recv.advance("1")
	recv.advance("0")
	recv.advance("0")
	recv.advance("1")
	recv.advance("0")
	recv.advance("1")
	recv.advance("0")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")

def test_c():
	recv = Receiver()
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("0")
	recv.advance("1")
	recv.advance("0")
	recv.advance("0")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("0")
	recv.advance("0")
	recv.advance("1")
	recv.advance("0")
	recv.advance("0")
	recv.advance("1")
	recv.advance("0")
	recv.advance("1")
	recv.advance("0")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")

def test_d():
	recv = Receiver()

	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")

	text = "- .... . / --.- ..- .. -.-. -.- / -... .-. --- .-- -. / ..-. --- -..- / .--- ..- -- .--. ... / --- ...- . .-. / - .... .. .-. - . . -. / .-.. .- --.. -.-- / -.. --- --. ..."

	for c in text:
		if c == "-":
			recv.advance("0")
			recv.advance("0")
			recv.advance("1")

		elif c == ".":
			recv.advance("0")
			recv.advance("1")

		elif c == " ":
			recv.advance("1")

		elif c == "/":
			recv.advance("1")
			recv.advance("1")

		else:
			raise Exception("Unhandled")

	recv.advance("1")
	recv.advance("1")
	recv.advance("1")
	recv.advance("1")


test_a()
test_b()
test_c()
test_d()
