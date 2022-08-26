# assembler for corewar-ni

import sys
import struct
import enum
from typing import Optional


class Opcode(enum.Enum):
    DAT = 0
    MOV = 1
    ADD = 2
    SUB = 3
    MUL = 4
    DIV = 5
    MOD = 6
    JMP = 7
    JMZ = 8
    JMN = 9
    DJN = 10
    SPL = 11
    CMP = 12
    SEQ = 13
    SNE = 14
    SLT = 15
    LDP = 16
    STP = 17
    NOP = 18


class Mode(enum.Enum):
    MEM = 0
    CONST = 1
    RELATIVE = 2


class Operand:
    def __init__(self, mode, value):
        self.mode: Mode = mode
        self.value: int = value


class Instr:
    def __init__(self, opcode, op0, op1: Optional[Operand]):
        self.opcode: Opcode = opcode
        self.op0: Operand = op0
        self.op1: Optional[Operand] = op1

    def assemble(self) -> bytes:
        opcode = bytes([self.opcode.value])
        a = struct.pack("<h", self.op0.value)
        a += bytes([self.op0.mode.value])
        if self.op1 is None:
            b = b"\x00\x01\x02"
        else:
            b = struct.pack("<h", self.op1.value)
            b += bytes([self.op1.mode.value])
        return opcode + a + b


def parse_operand(operand: str) -> Operand:
    if operand.startswith("#"):
        mode = Mode.CONST
        operand = operand[1:]
    elif operand.startswith("@"):
        mode = Mode.RELATIVE
        operand = operand[1:]
    else:
        mode = Mode.MEM
    v = int(operand)
    return Operand(mode, v)


def parse(line: str) -> Instr:
    if ";" in line:
        line = line[:line.find(";")]

    if " " not in line:
        raise RuntimeError("Each line should have at least one space")
    opcode = line[:line.find(" ")]
    opcode = Opcode[opcode.upper()]

    line = line[line.find(" ") + 1:]
    if "," not in line:
        # single operand
        parsed_a = parse_operand(line.strip(" "))
        parsed_b = None
    else:
        # double operand
        a, b = line.split(",")
        a = a.strip(" ")
        b = b.strip(" ")

        parsed_a = parse_operand(a)
        parsed_b = parse_operand(b)

    return Instr(opcode, parsed_a, parsed_b)


def main():
    input_file = sys.argv[1]
    output_file = sys.argv[2]

    with open(input_file, "r") as f:
        lines = f.read().split("\n")

    assembled = b""
    for line in lines:
        line = line.strip(" ")
        if not line:
            continue
        if line.startswith("#") or line.startswith(";"):
            # comment
            continue
        try:
            encoded = parse(line).assemble()
        except Exception as ex:
            print("Assembling failed: ", ex)
            raise
        assembled += bytes(encoded)

    with open(output_file, "wb")  as f:
        f.write(assembled)


if __name__ == "__main__":
    main()
