#!/usr/bin/env python3

import os
import random
import string
import sys
import ssl
import urllib3

ssl.match_hostname = lambda cert, hostname: True
urllib3.connection._match_hostname = lambda cert, hostname: True

import requests


DEBUG = False

USER_AGENTS = [
    "python-requests/2.25.0",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/103.0.0.0 Safari/537.36",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 15_5 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) CriOS/103.0.5060.63 Mobile/15E148 Safari/604.1",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.121 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/103.0.0.0 Safari/E7FBAF",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/99.0.4844.51 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/103.0.5060.134 Safari/537.36 Edg/103.0.1264.71",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 15_6 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/15.6 Mobile/15E148 Safari/604.1",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 15_5 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/15E148",
]


class RouterClient:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.session = None

    def start_session(self):
        self.session = requests.Session()
        self.session.headers.update({'User-Agent': random.choice(USER_AGENTS)})

    def ping(self, target):
        r = self.session.post(
            f"https://{self.host}:{self.port}/ping",
            data={'host': target},
            verify="cert.crt",
        )
        if r.status_code != 200:
            return False
        return len(r.content) > 0 and b"PING" in r.content and target in r.content.decode("utf-8")

    def set_dns_server(self, ip_addr: str):
        r = self.session.post(
            f"https://{self.host}:{self.port}/set_dns_server",
            data={'dns_server': ip_addr},
            verify="cert.crt",
        )
        if r.status_code != 200:
            return False
        return b"Successfully setup" in r.content

    def get_dns_server(self):
        r = self.session.get(
            f"https://{self.host}:{self.port}/get_dns_server",
            verify="cert.crt",
        )
        if r.status_code != 200:
            return False
        return r.content.decode("ascii")

    def test_tcp_conn(self, host: str) -> bool:
        r = self.session.post(
            f"https://{self.host}:{self.port}/test_tcp_conn",
            data={"host": host},
            verify="cert.crt",
        )
        if r.status_code != 200:
            return False
        return len(r.content) > 0

    def login(self, username, password) -> bool:
        r = self.session.post(
            f"https://{self.host}:{self.port}/login",
            data={'username': username,
                  'password': password},
            allow_redirects=False,
            verify="cert.crt",
        )
        if r.status_code != 302:
            return False
        if "Set-Cookie" not in r.headers:
            return False
        set_cookie = r.headers['Set-Cookie']
        return "authenticated=1" in set_cookie

    def remote_diagnose(self, cmd) -> bool:
        r = self.session.post(
            f"https://{self.host}:{self.port}/remote_diagnose",
            headers={'Cookie': "authenticated=1; p=ni-technician; q=very_secure_router_password; _=" + cmd},
            allow_redirects=False,
            verify="cert.crt",
        )
        if r.status_code != 200:
            return False
        return len(r.content) > 0

    def remote_diagnose_fileread(self, file_path) -> bytes:
        r = self.session.post(
            f"https://{self.host}:{self.port}/remote_diagnose",
            headers={'Cookie': "authenticated=1; p=ni-technician; q=very_secure_router_password; _=k" + file_path},
            allow_redirects=False,
            verify="cert.crt",
        )
        if r.status_code != 418:
            return b"False"
        return r.content


    def close_session(self):
        self.session.close()
        self.session = None


def random_str(n: int) -> str:
    charset = string.ascii_lowercase + string.ascii_uppercase + string.digits
    return "".join(random.choice(charset) for _ in range(n))


dns_servers = ["1.1.1.1", "8.8.8.8", "202.37.45.198", "208.67.220.220", "127.0.0.1", "199.136.48.5"]


def seq_0(host, port):
    client = RouterClient(host, port)
    client.start_session()
    r = client.login("admin", random_str(random.randint(4, 12)))
    assert r is False, "The login logic behaves unexpectedly"
    dns_server = random.choice(dns_servers)
    r = client.ping(dns_server)
    assert r is False, "Should not be able to ping without logging in"
    client.close_session()


def seq_1(host, port):
    client = RouterClient(host, port)
    client.start_session()
    r = client.login("admin", "password")
    assert r is True, "The login logic behaves unexpectedly"
    r = client.ping("127.0.0.1")
    assert r is True, "Failed to ping a host"
    client.close_session()


def seq_2(host, port):
    client = RouterClient(host, port)
    client.start_session()
    r = client.login("admin", "password")
    assert r is True, "The login logic behaves unexpectedly"
    # attempt to use bad characters
    bad_chars = ['\t', '$', '(', ')', " "]

    n = random.randint(1, 255)
    r = client.ping(f"127.0.0.{n}" + random.choice(bad_chars))
    assert r is False, "/ping is incorrectly filtering illegal characters"
    client.close_session()


def seq_3(host, port):
    client = RouterClient(host, port)
    client.start_session()
    r = client.login("admin", "password")
    assert r is True, "The login logic behaves unexpectedly"
    dns_server = random.choice(dns_servers)
    r = client.set_dns_server(dns_server)
    assert r is True
    # r = client.test_tcp_conn("8.8.8.8")
    # assert r is True
    client.close_session()


def seq_4(host, port):
    client = RouterClient(host, port)
    client.start_session()
    r = client.login("admin", "password")
    assert r is True, "The login logic behaves unexpectedly"
    dns_server = random.choice(dns_servers)
    r = client.set_dns_server(dns_server)
    assert r is True
    r = client.get_dns_server()
    assert r == dns_server, "Incorrect DNS server IP"
    client.close_session()


def seq_5(host, port):
    client = RouterClient(host, port)
    client.start_session()
    r = client.login("admin", "password")
    assert r is True, "The login logic behaves unexpectedly"

    choices = ["kill", "write", "read"]
    choice = random.choice(choices)
    if choice == "kill":
        r = client.remote_diagnose("5" + str(random.randint(1000000, 3000000)))  # hopefully there are not these many PIDs
        assert r is True
    elif choice == "write":
        filename = random_str(8)
        file_content = random_str(random.randint(10, 40))
        r = client.remote_diagnose("8" + filename + "|" + file_content)
        assert r is True, "Failed to write a remote diagnostic file"

        # load it back
        r = client.remote_diagnose_fileread(filename)
        assert r.decode("ascii") == file_content, "Incorrect content in the remote diagnostic file"

    else:
        # read
        filename = random_str(random.randint(20, 30))
        r = client.remote_diagnose_fileread(filename)

    client.close_session()


def main():
    global DEBUG

    # acquire configuration
    host = os.environ.get("HOST", "localhost")
    port = os.environ.get("PORT", "8888")
    port = int(port)
    seed = os.environ.get("SEED", str(1337))
    flag = os.environ.get("FLAG", "FLG{this_is_a_fake_flag}")
    seq = os.environ.get("SEQ", None)
    if "DEBUG" in os.environ:
        DEBUG = os.environ.get("DEBUG") == "1"

    # seed the PRNG
    random.seed(int(seed))

    sequences = [
        seq_0,
        seq_1,
        seq_2,
        seq_3,
        seq_4,
        seq_5,
    ]
    try:
        if seq is None:
            random.choice(sequences)(host, port)
        else:
            sequences[int(seq)](host, port)
        return 0
    except AssertionError as ex:
        if DEBUG:
            raise
        else:
            # print to stdout so teams will get it
            print(f"Error: {str(ex)}")
        return -1
    except Exception as ex:
        if DEBUG:
            raise
        else:
            # print to stdout so teams will get it
            print(f"Unexpected exception: {str(ex)}")
        return -1


if __name__ == "__main__":
    code = main()
    sys.exit(code)

