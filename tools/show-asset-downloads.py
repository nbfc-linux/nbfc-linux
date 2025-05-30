#!/usr/bin/env python3

import requests

def get_all_assets_downloads(repo):
    url = f"https://api.github.com/repos/nbfc-linux/{repo}/releases"
    total_downloads = 0

    try:
        response = requests.get(url)
        response.raise_for_status()
    except requests.RequestException as e:
        print(f"Error fetching data from GitHub: {e}")
        return

    releases = response.json()
    if not releases:
        print("No releases found.")
        return

    for release in releases:
        for asset in release.get("assets", []):
            name = asset.get("name")
            count = asset.get("download_count", 0)
            print(f"{name:60}: {count}")
            total_downloads += count

    print(f"Total downloads across all assets                           : {total_downloads}\n")

if __name__ == "__main__":
    get_all_assets_downloads('nbfc-linux')
    get_all_assets_downloads('nbfc-qt')
    get_all_assets_downloads('nbfc-gtk')
