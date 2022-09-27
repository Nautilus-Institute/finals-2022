import sys, os

def rotl(value, distance):
	return ((value << distance) | (value >> (64 - distance))) & 0xffffffffffffffff

def ConvertRoomIDToMem(RoomID):
	OrigRoomChars = ['G','Q','0','W','B','L','J','2','X','R','T','5','4','H','8','N','3']

	FirstCharIndex = OrigRoomChars.index(RoomID[0])
	if(FirstCharIndex == -1):
		return -1

	RoomChars = OrigRoomChars[FirstCharIndex:] + OrigRoomChars[0:FirstCharIndex]

	RoomCode = []
	
	#start creating the key value
	for x in range(1, len(RoomID)):
		#find the index
		i = RoomChars.index(RoomID[x])
		if(i == -1):
			return -1

		RoomCode.append(i)
		i = (i + 1) % len(RoomChars)
		RoomChars = RoomChars[i:] + RoomChars[0:i]

	#calculate
	KeyValue = 0
	i = 0
	while(RoomCode):
		KeyValue *= len(RoomChars)
		PopVal = RoomCode.pop()
		KeyValue += PopVal
		print(i, hex(KeyValue), hex(PopVal))
		i += 1

	KeyValue = (KeyValue ^ rotl(0xb236fdfa7ea1e492, FirstCharIndex)) & 0xffffffffffffffff
	print(RoomID, "=", hex(KeyValue))
	return KeyValue

ConvertRoomIDToMem(sys.argv[1])

