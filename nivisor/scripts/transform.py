from os import O_RDWR
import lief
import sys
import struct

RODATA = 0
DATA = 1
TEXT = 2

INTERP_MASK  = 0x200
VERSION_MASK = 0x100

MAGIC = 0x45554c42
MAX_RELOCS = 1024
SIZEOF_RELOC = 25

lief_section_mapping = {
        ".text" : TEXT,
        ".data" : DATA,
        ".rodata" : RODATA
        }

PC_RELOCS = [
    lief.ELF.RELOCATION_X86_64.PC8.value,
    lief.ELF.RELOCATION_X86_64.PC16.value,
    lief.ELF.RELOCATION_X86_64.PC32.value,
    lief.ELF.RELOCATION_X86_64.PC64.value,
    lief.ELF.RELOCATION_X86_64.PLT32.value,
    ]



def handle_pc_reloc(r, sections):

    patch_offset = r.address
    try:
        patch_section = lief_section_mapping[r.section.name]
    except KeyError:
        print("Ignoring some shit from %s" % (r.section.name))
        return 0
    # _PC is always S + A - P
    # S: Relocation entry’s correspondent symbol value.
    # A: Addend of Elfxx_Rela entries.
    # P: The section offset or address of the storage unit being relocated. (.offset)
    if (r.has_symbol == False):
        print("Fuck")
        import sys;sys.exit()
    S = r.symbol.value
    A = r.addend
    #P = r.offset
    # we don't care about P because we'll do the section relative stuff in the loader
    P = 0
    value_offset = S + A - P
    #value_section = lief_section_mapping
    print(list(map(lambda x: x.name, sections)))
    print("r.symbol.shndx %d\n" % r.symbol.shndx)
    print("SECTION NAME %s" % sections[r.symbol.shndx].name)
    try:
        value_section = lief_section_mapping[sections[r.symbol.shndx].name]
    except KeyError:
        print("Hope you didn't care about " + r.symbol.name)
        return 0

    size = r.size >> 3
    #def __init__(self, patch_offset, patch_section, value_offset, value_section, size):
    new_dr = BlueReloc(patch_offset, patch_section, value_offset, value_section, size)
    return new_dr



class DummySection():
    def __init__(self):
        self.size = 0
        self.content = None

class BlueReloc:
    def __init__(self, patch_offset, patch_section, value_offset, value_section, size):
        self.patch_offset = patch_offset
        self.patch_section = patch_section
        self.value_offset = value_offset
        self.value_section = value_section
        self.size = size
        self.structify()

    def structify(self):
        '''
    struct __attribute__((__packed__)) reloc {
      // TODO: Can this also be negative?
      int64_t patch_offset;
      enum SECTION_TYPE patch_section;
      int64_t value_offset;
      enum SECTION_TYPE value_section;
      uint8_t size;
    };
        '''
        data = b''
        data += struct.pack("<q", self.patch_offset)
        data += struct.pack("<I", self.patch_section)
        data += struct.pack("<q", self.value_offset)
        data += struct.pack("<I", self.value_section)
        data += struct.pack("B", self.size)
        self.data = data
        

class BlueInfo:
    def __init__(self, linker=None, version=None):
        self.linker = linker
        self.version = version
        self.flags = 0

        if self.linker:
            self.flags |= INTERP_MASK

        if self.version:
            self.flags |= VERSION_MASK

    def structify(self):
        data = b''
        if (self.linker):
            data += self.linker.encode()
        data = data.ljust(128, b'\x00')
        
        if (self.version):
            data += struct.pack("<I", self.version)
        else:
            data += struct.pack("<I", 0)

        return data


class Blue:
    def __init__(self, fname, linker=None, version=None):
        self.binary = lief.parse(fname)
        self.text = DummySection()
        self.rodata = DummySection()
        self.data = DummySection()
        self.bss = DummySection()
        self.info = BlueInfo(linker, version)
        self.flags = self.info.flags

        for s in self.binary.sections:
            if s.name == '.text':
                self.text = s
            elif s.name == '.rodata':
                self.rodata = s
            elif s.name == '.data':
                self.data = s
            elif s.name == '.bss':
                self.bss = s

        self.relocs = self.binary.relocations
        self.blue_relocs = []

    def find_main(self):
        for symbol in self.binary.symbols:
            if (symbol.name == 'main'):
                self.main = symbol
                return

        raise(Exception("Main not found"))
        

    def transform_relocs(self):
        for r in self.relocs:
            if r.type in PC_RELOCS:
                new_dr = handle_pc_reloc(r, self.binary.sections)
                if (new_dr):
                    self.blue_relocs.append(new_dr)
            else:
                print("Unsupported reloc")

    def output(self, fname):
        '''
        struct dong {
          uint32_t magic;
          uint64_t text_size;
          uint64_t rodata_size;
          uint64_t data_size;
          uint64_t num_relocs;
          struct reloc relocs[MAX_RELOCS];
        '''
        data = struct.pack(">I", MAGIC)
        data += struct.pack("<I", self.flags)
        data += struct.pack("<Q", self.text.size)
        data += struct.pack("<Q", self.rodata.size)
        data += struct.pack("<Q", self.data.size)
        data += struct.pack("<Q", self.main.value)
        data += struct.pack("<Q", len(self.blue_relocs))

        for dr in self.blue_relocs:
            data += dr.data

        remaining_relocs = MAX_RELOCS - len(self.blue_relocs)
        data += (b'\x00' * SIZEOF_RELOC) * remaining_relocs

        data += self.info.structify()
        
        if (self.text.content):
            data += self.text.content.tobytes()
        if (self.rodata.content):
            data += self.rodata.content.tobytes()
        if (self.data.content):
            data += self.data.content.tobytes()

        f = open(fname, 'wb')
        f.write(data)
        f.close()

            
linker = None
if (len(sys.argv) == 4):
    linker = sys.argv[3]

d = Blue(sys.argv[1], linker)
d.find_main()
d.transform_relocs()
d.output(sys.argv[2])
