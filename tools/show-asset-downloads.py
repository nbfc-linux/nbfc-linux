#!/usr/bin/env python3

import requests

GITHUB_API_URL = "https://api.github.com/repos/nbfc-linux/nbfc-linux/releases"

def get_all_assets_downloads():
    total_downloads = 0

    try:
        response = requests.get(GITHUB_API_URL)
        response.raise_for_status()
    except requests.RequestException as e:
        print(f"Error fetching data from GitHub: {e}")
        return

    releases = response.json()
    if not releases:
        print("No releases found.")
        return

    print("Download counts per asset:\n")

    for release in releases:
        for asset in release.get("assets", []):
            name = asset.get("name")
            count = asset.get("download_count", 0)
            print(f"{name}: {count}")
            total_downloads += count

    print("\nTotal downloads across all assets:", total_downloads)

if __name__ == "__main__":
    get_all_assets_downloads()
