#!/usr/bin/env python3

import sys
import requests

OWNER = "nbfc-linux"
REPO  = "nbfc-linux"
URL   = f"https://api.github.com/repos/{OWNER}/{REPO}/releases/latest"

with open(sys.argv[1]) as fh:
    content = fh.read()

response = requests.get(URL)
response.raise_for_status()
releases = response.json()

DEBIAN = None
FEDORA = None
ARCHLINUX = None
OPENSUSE = None

TAG = releases['tag_name']

for release in releases['assets']:
    name = release['name']
    url  = release['browser_download_url']

    if name.endswith('.deb'):
        DEBIAN = (name, url)

    elif name.startswith('fedora-'):
        FEDORA = (name, url)

    elif name.startswith('opensuse-'):
        OPENSUSE = (name, url)

    elif name.endswith('pkg.tar.zst'):
        ARCHLINUX = (name, url)

content = content.replace('%TAG%', TAG)
content = content.replace('%ARCHLINUX_PACKAGE%', ARCHLINUX[0])
content = content.replace('%ARCHLINUX_URL%',     ARCHLINUX[1])
content = content.replace('%DEBIAN_PACKAGE%',    DEBIAN[0])
content = content.replace('%DEBIAN_URL%',        DEBIAN[1])
content = content.replace('%FEDORA_PACKAGE%',    FEDORA[0])
content = content.replace('%FEDORA_URL%',        FEDORA[1])
content = content.replace('%OPENSUSE_PACKAGE%',  OPENSUSE[0])
content = content.replace('%OPENSUSE_URL%',      OPENSUSE[1])

print(content)
