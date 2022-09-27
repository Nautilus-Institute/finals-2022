import sys, os
import struct

BLOCK_SIZE = 16

elffile = open(sys.argv[1], "r+b")

#get the section offset
elffile.seek(0x20, os.SEEK_SET)
section_offset = struct.unpack("<I", elffile.read(4))[0]

#get section table details
elffile.seek(0x2e, os.SEEK_SET)
(e_shentsize, e_shnum, e_shstrndx) = struct.unpack("<HHH", elffile.read(6))

#go through each section
elffile.seek(section_offset, os.SEEK_SET)

#read all sections into a table
sections = []
for i in range(0, e_shnum):
	(sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size, sh_link, sh_info, sh_addralign, sh_entsize) = struct.unpack("<IIIIIIIIII", elffile.read(10 * 4))
	sections.append({"sh_name": sh_name, "sh_type": sh_type, "sh_flags": sh_flags, "sh_addr": sh_addr, "sh_offset": sh_offset, "sh_size": sh_size, "sh_link": sh_link, "sh_info": sh_info, "sh_addralign": sh_addralign, "sh_entsize": sh_entsize})

#get the section strings
elffile.seek(sections[e_shstrndx]["sh_offset"], os.SEEK_SET)
shstr = elffile.read(sections[e_shstrndx]["sh_size"])

#find .strtab for normal strings
for entry in sections:
	if (entry["sh_type"] == 3):
		nulloffset = shstr.find(0, entry["sh_name"])
		if shstr[entry["sh_name"]:nulloffset] == b".strtab":
			elffile.seek(entry["sh_offset"], os.SEEK_SET)
			strtab = elffile.read(entry["sh_size"])
			break

#find .symtab section
for entry in sections:
	if entry["sh_type"] == 2:
		break

#find _ZTV13DirectionDoor for the vtable section
DirectionDoorSection = []
for i in range(1, int(entry["sh_size"] / 0x10)):
	elffile.seek(entry["sh_offset"] + (i * 0x10), os.SEEK_SET)
	(st_name, st_value, st_size, st_info, st_other, st_shndx) = struct.unpack("<IIIBBH", elffile.read(0x10))

	if(st_shndx > len(sections)):
		continue

	objnulloffset = strtab.find(0, st_name)
	if strtab[st_name:objnulloffset] == b"_ZTV13DirectionDoor":
		DirectionDoorSection = [st_shndx, st_value, st_size]
		print("Found DirectionDoor VTable: %x bytes long" % (st_size))
		break

#now find UserInputBuffer and rewrite it's st_shndx and value
if len(DirectionDoorSection):
	for i in range(i+1, int(entry["sh_size"] / 0x10)):
		elffile.seek(entry["sh_offset"] + (i * 0x10), os.SEEK_SET)
		(st_name, st_value, st_size, st_info, st_other, st_shndx) = struct.unpack("<IIIBBH", elffile.read(0x10))

		if(st_shndx > len(sections)):
			continue

		objnulloffset = strtab.find(0, st_name)
		if strtab[st_name:objnulloffset] == b"UserInputBuffer":
			print("Rewriting UserInputBuffer to be after DirectionDoor vtable")
			st_shndx = DirectionDoorSection[0]
			st_value = DirectionDoorSection[1] + DirectionDoorSection[2]

			print("st_shndx", st_shndx, "st_value", hex(st_value))
			elffile.seek(entry["sh_offset"] + (i * 0x10) + 4, os.SEEK_SET)
			outdata = struct.pack("<IIBBH", st_value, st_size, st_info, st_other, st_shndx)
			elffile.write(outdata)

			#alter the original section size
			elffile.seek(section_offset + (st_shndx * (10*4)) + (5*4), os.SEEK_SET)
			elffile.write(struct.pack("<I", st_value + st_size))
			break

elffile.close()