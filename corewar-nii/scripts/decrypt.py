import sys


def main():
    data = open(sys.argv[1], "rb").read()
    decrypted = [ ]
    for i, b in enumerate(data):
        enc = (b + 9) & 0xff
        if i % 2 == 0:
            enc = enc ^ 0xca
        else:
            enc = enc ^ 0x77
        decrypted.append(enc)

    with open(sys.argv[2], "wb") as f:
        f.write(bytes(decrypted))


if __name__ == "__main__":
    main()
