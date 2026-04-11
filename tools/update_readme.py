#!/usr/bin/env python3

import sys
import requests

OWNER = "nbfc-linux"
REPO  = "nbfc-linux"

def get_file_count(owner, repo, path):
    url = f"https://api.github.com/repos/{owner}/{repo}/contents/{path}"
    response = requests.get(url)
    response.raise_for_status()
    releases = response.json()
    return len(releases)

def replace_package_data(content):
    url = f"https://api.github.com/repos/{OWNER}/{REPO}/releases/latest"
    response = requests.get(url)
    response.raise_for_status()
    releases = response.json()

    mapping = {
        'debian-trixie':    'DEBIAN_TRIXIE',
        'debian-bookworm':  'DEBIAN_BOOKWORM',
        'ubuntu-noble':     'UBUNTU_NOBLE',
        'ubuntu-jammy':     'UBUNTU_JAMMY',
        'linux-mint-22':    'LINUX_MINT_22',
        'linux-mint-21':    'LINUX_MINT_21',
        'fedora':           'FEDORA',
        'opensuse':         'OPENSUSE',
        'arch-linux':       'ARCHLINUX',
    }

    TAG = releases['tag_name']
    content = content.replace('%TAG%', TAG)

    for release in releases['assets']:
        name = release['name']
        url  = release['browser_download_url']

        for file_prefix, replacement_prefix in mapping.items():
            if name.startswith(file_prefix):
                content = content.replace(f'%{replacement_prefix}_PACKAGE%', name)
                content = content.replace(f'%{replacement_prefix}_URL%', url)

    return content

def replace_config_file_count(content):
    original = get_file_count("hirschmann", "nbfc", "Configs")
    nbfc_linux = get_file_count("nbfc-linux", "configs", "1.0/configs")

    content = content.replace("%HIRSCHMANN_NUM_CONFIGS%", str(original))
    content = content.replace("%NBFC_LINUX_NUM_CONFIGS%", str(nbfc_linux))

    return content

with open(sys.argv[1]) as fh:
    content = fh.read()

content = replace_package_data(content)
content = replace_config_file_count(content)

print(content)
