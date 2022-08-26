# assembler for corewar-nii

import sys

import keystone


def main():
    input_file = sys.argv[1]
    output_file = sys.argv[2]

    ks = keystone.Ks(keystone.KS_ARCH_X86, keystone.KS_MODE_32)
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
            encoding, count = ks.asm(line)
        except keystone.KsError as ex:
            print("Assembling failed: ", ex)
            raise
        assembled += bytes(encoding)

    with open(output_file, "wb")  as f:
        f.write(assembled)


if __name__ == "__main__":
    main()
