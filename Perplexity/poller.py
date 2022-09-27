import sys, os
import socket
import select
import random

def eprint(*args, **kwargs):
	print(*args, file=sys.stderr, **kwargs)

def GetData(s):
	data = b""

	while(1):
		if len(data):
			timeout = 0.5
		else:
			timeout = 1.5

		(r,w,e) = select.select([s], [], [], timeout)
		if len(r) == 0:
			return data

		new_data = s.recv(1024)
		if len(new_data) == 0:
			return data

		data += new_data
	return data

def GenRandomString(str_len):
	if str_len == -1:
		str_len = random.randint(3, 100)

	str = []
	while(len(str) < str_len):
		str.append(random.randint(1, 255))
		if str[-1] == 10:
			str[-1] += 1

	#pick a random place to be a jerk and do 0408/0508/0608 knowing they might try to ban that combo but only randomly
	if random.choice([0,1,2]) == 0:
		randoffset = random.randint(0, len(str) - 4) & ~3
		str[randoffset+2] = 4 + random.randint(0, 2)
		str[randoffset+3] = 8

	return bytes(str)

def GetDirections(s):
	s.send(b"describe\n")
	data = GetData(s)

	#parse up the directions
	Directions = {}
	data = data.split(b"\n")
	if b"You are in" not in data[0]:
		return {}

	data.pop(0)
	for entry in data:
		curline = entry.split()
		if len(curline) < 3:
			break
		curdir = curline[2]

		doortype = "room"
		if b"empty slots" in entry:
			doortype = "wheellock"
		elif b"closed door" in entry:
			s.send(b"open " + curdir + b" door\n")
		elif b"locked closed door" in entry:
			doortype = "lock"
		elif b"powered door" in entry:
			doortype = "power"
		elif b"teleporter" in entry:
			doortype = "teleporter"

		if doortype not in Directions:
			Directions[doortype] = []
		Directions[doortype].append(curdir)
	return Directions

def GetItems(s):
	s.send(b"look\n")
	data = GetData(s).split(b"\n")


	Items = []
	for entry in data:
		if b"large switch" in entry:
			Items.append("switch")
			if b"off position" in entry:
				s.send(b"flip switch\n")
		elif b"steam punk" in entry:
			Items.append("display")
			#print("Found display")
			if b"dark screen" in entry:
				s.send(b"activate display\n")

	return Items

def GetDisplayDetails(s):
	#s.send(b"view display\n")
	data = GetData(s).split(b"\n")

	Rooms = []
	for entry in data:
		curline = entry.split()
		if len(curline) < 2:
			continue
		if curline[0] == b"Room":
			Rooms.append(curline[1])

	return Rooms

def MoveDirection(s, Directions, RoomLocations):
	if len(RoomLocations) and "teleporter" in Directions:
		s.send(b"touch " + Directions["teleporter"][0] + b" teleporter keypad\n")
		s.send(random.choice(RoomLocations) + b"\n")
		s.send(b"activate " + Directions["teleporter"][0] + b" teleporter\n")
		data = GetData(s)
		if b"bright yellow fluxuating liquid" not in data:
			eprint("failed to activate teleporter")
			return -1
		s.send(Directions["teleporter"][0] + b"\n")
		#print("used teleporter")
	else:
		ret = random.choice(Directions["room"]) + b"\n"
		s.send(ret)
	GetData(s)
	return 0

def RunPoller(remote_ip, remote_port):
	s = socket.create_connection((remote_ip, remote_port))

	#make sure we got a response
	data = GetData(s)
	if b"Please provide your name:" not in data:
		eprint("Failed to get name")
		return -1

	s.send(GenRandomString(-1) + b"\n")
	GetData(s)

	#attempt to create a page then validate it
	s.send(b"in journal create page\n")
	GetData(s)

	s.send(b"in journal on page 1 write\n")
	GetData(s)

	rand_str = GenRandomString(250)
	s.send(rand_str + b"\n\n")
	#get the output
	data = GetData(s)

	#read the page
	s.send(b"in journal read page 1\n")
	data = GetData(s).split(b"\n")

	if len(data) != 3:
		eprint("Failed to read journal page data")
		return -1

	if rand_str != data[1]:
		eprint(f"Journal page data did not match: {len(rand_str)} != {len(data[1])}")
		return -1

	#crumple and uncrumple a page
	s.send(b"from journal take page 1\n")
	data = GetData(s).split(b"\n")
	s.send(b"crumple page 1\n")
	s.send(b"uncrumple page 1\n")
	data = GetData(s).split(b"\n")

	#page writing seems ok and not muddled, go validate wandering a bit
	RoomLocations = []
	for i in range(0, 100):
		Directions = GetDirections(s)
		Items = GetItems(s)
		if "display" in Items:
			RoomLocations = GetDisplayDetails(s)

		if MoveDirection(s, Directions, RoomLocations) == -1:
			eprint("Failed to move direction")
			return -1

	return 0

if __name__ == "__main__":
	HOST = os.environ["HOST"]
	PORT = os.environ["PORT"]
	SEED = os.environ["SEED"]
	random.seed(SEED)
	eprint("HOST={}".format(HOST))
	eprint("PORT={}".format(PORT))
	eprint("SEED={}".format(SEED))

	ret = -1
	try:
		ret = RunPoller(HOST, PORT)
	except BrokenPipeError as ex:
		ret = -1
	except ConnectionError as ex:
		ret = -1
	except Exception as ex:
		import traceback
		traceback.print_exc()
		ret = 1

	if ret == -1:
		print("bad")
	elif ret == 0:
		print("good")
	else:
		print("poller exception")

	sys.exit(ret)