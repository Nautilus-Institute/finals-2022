import hashlib
from operator import truediv
import struct

class NautilusRand:
	def __init__(self, Filename:str=None, Data=None):
		if Filename:
			Data = open(Filename, "rb").read()

		SHAHash = hashlib.sha512(Data).digest()
		for i in range(0, 1000):
			SHAHash = hashlib.sha512(SHAHash).digest()

		self._InitWELL512a(SHAHash)

	def _InitWELL512a(self, data):
		self._STATE = list(struct.unpack("<" + "I"*16, data))
		self._state_i = 0;

	def _MAT0POS(self,t,v):
		return (v^((v >> t) & 0xffffffff))

	def _MAT0NEG(self,t,v):
		return (v^((v << -t) & 0xffffffff))

	def _MAT3NEG(self,t,v):
		return ((v << -t) & 0xffffffff)

	def _MAT4NEG(self,t,b,v):
		return (v ^ ((v<<(-(t))) & b)) & 0xffffffff

	def _WELLRNG512a(self):
		z0    = self._STATE[(self._state_i+15) & 0x0000000f]
		z1    = self._MAT0NEG (-16,self._STATE[self._state_i]) ^ self._MAT0NEG (-15, self._STATE[(self._state_i+13) & 0x0000000f])
		z2    = self._MAT0POS (11, self._STATE[(self._state_i+9) & 0x0000000f])
		self._STATE[self._state_i] = z1 ^ z2
		self._STATE[(self._state_i+15) & 0x0000000f] = self._MAT0NEG (-2,z0)     ^ self._MAT0NEG(-18,z1)    ^ self._MAT3NEG(-28,z2) ^ self._MAT4NEG(-5,0xda442d24,self._STATE[self._state_i])
		self._state_i = (self._state_i + 15) & 0x0000000f
		return self._STATE[self._state_i]

	def GetRandData(self, datalen:int):
		result = b""
		while len(result) < (datalen & ~7):
			result += struct.pack("<Q", self.GetRandVal())

		datalen -= len(result)
		if datalen:
			RandVal = self.GetRandVal()
			if datalen >= 4:
				result += struct.pack("<I", RandVal & 0xffffffff)
				RandVal >>= 32
				datalen -= 4
			if datalen >= 2:
				result += struct.pack("<H", RandVal & 0xffff)
				RandVal >>= 16
				datalen -= 2
			if datalen:
				result += struct.pack("<B", RandVal & 0xff)

		return result

	def GetRandVal(self):
		result = (self._WELLRNG512a() & 0xffffffff)
		return result | ((self._WELLRNG512a() & 0xffffffff) << 32)

if __name__ == "__main__":
	RandExpectedResult = [0xf773127ce8f5a2e1,0xeb42129a140b4d18,0x3819bdbf3a840691,0xbfb5b3754d337b1d,0x3f71a5c9c9dd9ce8,0xb33092ed5197efcc,0xb8f8d7c1d7ac01db,0x7faed22e938379de]
	RandExpectedResult2 = [0x39978df7749403fd,0x4d521311536f0227,0xe4d9d8ad0ba0d9ae,0x19a9e732b1201625,0x0ad5d28cc91e36c4,0x673b48523df37201,0xea16ecbd241301a8,0x000000213793b313]

	Rnd = NautilusRand(Data = b"testing")
	Result = Rnd.GetRandData(int(512 / 8))
	ResultVal = struct.unpack("<" + "Q"*8, Result)
	Failed = False
	for i in range(0, len(RandExpectedResult)):
		if RandExpectedResult[i] != ResultVal[i]:
			print("Rand invalid")
			output = ""
			for i in RandExpectedResult:
				output += "0x%016x-" % (i)
			print("Expected:", output[0:-1])

			output = ""
			for i in ResultVal:
				output += "0x%016x-" % (i)
			print("Got:     ", output[0:-1])
			Failed = True
			break

	if not Failed:
		print("Rand valid")

	Result = Rnd.GetRandData(int(512 / 8) - 3) + bytes([0,0,0])
	ResultVal = struct.unpack("<" + "Q"*8, Result)
	Failed = False
	for i in range(0, len(RandExpectedResult2)):
		if RandExpectedResult2[i] != ResultVal[i]:
			print("Partial Rand invalid")
			output = ""
			for i in RandExpectedResult2:
				output += "0x%016x-" % (i)
			print("Expected:", output[0:-1])

			output = ""
			for i in ResultVal:
				output += "0x%016x-" % (i)
			print("Got:     ", output[0:-1])
			Failed = True
			break

	if not Failed:
		print("Partial Rand valid")

