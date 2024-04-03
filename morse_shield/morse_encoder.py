from morse_state_machine import Receiver

class Encoder():
	def __init__(self):
		self.sequence = 0
		self.count = 0
		self.bit_pattern = 0
		self.bit_count = 0

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

		self.alpha1 =  {v: k for k, v in alpha1.items()}
		self.alpha2 =  {v: k for k, v in alpha2.items()}
		self.alpha3 =  {v: k for k, v in alpha3.items()}
		self.alpha4 =  {v: k for k, v in alpha4.items()}

	# Пробел: bit_pattern = 0b111, bit_count = 3
	def enqueue(self, letter):
		if self.bit_count:
			return False # BUSY

		if letter == ' ':
			self.bit_pattern = 0b11
			self.bit_count = 2
			return True
		elif letter in self.alpha1:
			self.sequence = self.alpha1[letter]
			self.count = 1
		elif letter in self.alpha2:
			self.sequence = self.alpha2[letter]
			self.count = 2
		elif letter in self.alpha3:
			self.sequence = self.alpha3[letter]
			self.count = 3
		elif letter in self.alpha4:
			self.sequence = self.alpha4[letter]
			self.count = 4
		else:
			raise Exception("Unk letter")

		self.next_pattern()

		return True

	# symbol это mark или space
	def next_pattern(self):
		if self.count == 0:
			return

		self.count -= 1
		symbol = self.sequence & (1 << self.count)

		if symbol:
			self.bit_pattern = 0b1000
			self.bit_count = 4
		else:
			self.bit_pattern = 0b10
			self.bit_count = 2

		# Добавление отступа после символа
		if self.count == 0:
			self.bit_pattern += 0b11 << self.bit_count
			self.bit_count += 2

	# Держим высокий уровень, пока очередь битов пустая
	# Т.е. это IDLE
	def level(self):
		if self.bit_count == 0:
			return "1"

		bit = self.bit_pattern & 1
		self.bit_pattern = self.bit_pattern >> 1
		self.bit_count -= 1

		if self.bit_count == 0:
			self.next_pattern()

		return str(bit)

encoder = Encoder()
msg = "THE QUICK BROWN FOX JUMPS OVER THIRTEEN LAZY DOGS"
i = 0

recv = Receiver()

recv.advance(encoder.level())
recv.advance(encoder.level())
recv.advance(encoder.level())
recv.advance(encoder.level())
recv.advance(encoder.level())

while i < len(msg):
	letter = msg[i]

	if encoder.enqueue(letter):
		i += 1

	level = encoder.level()
	recv.advance(level)


recv.advance(encoder.level())
recv.advance(encoder.level())
recv.advance(encoder.level())
recv.advance(encoder.level())
recv.advance(encoder.level())
recv.advance(encoder.level())
recv.advance(encoder.level())
recv.advance(encoder.level())
recv.advance(encoder.level())
recv.advance(encoder.level())
