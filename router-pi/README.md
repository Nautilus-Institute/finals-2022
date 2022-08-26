# router-pi

An ARM httpd service with memory corruption bugs, backdoors, race conditions, and a bunch of other features that do not always work (just like most routers' firmware).

Difficulty level: Easy

Shamelessly used code from the following projects:

- https://github.com/kokke/tiny-regex-c
- https://github.com/jart/cosmopolitan

## Intended vulnerabilities

1. The regex engine would skip the last character in a chunk when compiling regexes.
As a result, it fails to filter the input that is fed to `/ping`, leading to a command line injection.

2. There is a backdoor at `/remote_diagnose` that would allow the creation of arbitrary files on the file system and then sending the content back.
It has a path traversal bug that allow attackers to directly leak the flag.
Note that the teams should not completely remove this feature.
Rather, the intended fix is propery sanitizing the file name.

3. The user can set the DNS server for the service, and then perform a time synchronization against a remote NTP server.
If the time server is configured to be an IP address, the router will perform a reverse IP lookup.
The result of the reverse IP lookup will be copied onto the stack, leading to a stack buffer overflow.
If the time server is configured to be a domain name, the router will perform an IP resolution, and copy the resulting IP to the stack without checking its size, leading to a stack buffer overflow.

4. The service 

## Q & A

Q. Why do you enjoy writing router challenges?

A. Because this is probably the only way for me to write shitty code for a challenge without getting blamed.

