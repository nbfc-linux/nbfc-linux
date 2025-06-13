#!/usr/bin/env python3

import sys
import requests
import hashlib

OWNER = "nbfc-linux"
REPO  = "nbfc-linux"
LATEST_URL = f"https://api.github.com/repos/{OWNER}/{REPO}/releases/latest"

with open(sys.argv[1]) as fh:
    content = fh.read()

response = requests.get(LATEST_URL)
response.raise_for_status()
releases = response.json()

TAG = releases['tag_name']

TARBALL_URL = f"https://github.com/{OWNER}/{REPO}/archive/refs/tags/{TAG}.tar.gz"

response = requests.get(TARBALL_URL)
response.raise_for_status()

SHA256 = hashlib.sha256(response.content).hexdigest()

content = content.replace('%TAG%', TAG)
content = content.replace('%SHA256%', SHA256)

print(content, end='')
