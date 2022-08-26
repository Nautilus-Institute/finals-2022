import sys


def main():
    data = open(sys.argv[1], "rb").read()
    encrypted = [ ]
    for i, b in enumerate(data):
        if i % 2 == 0:
            enc = b ^ 0xca
        else:
            enc = b ^ 0x77
        enc = (enc - 9) & 0xff
        encrypted.append(enc)

    with open(sys.argv[2], "wb") as f:
        f.write(bytes(encrypted))


if __name__ == "__main__":
    main()
