#!/bin/bash

REPO='https://github.com/UraniumDonut/nbfc-revive'

OUTDIR='nbfc-revive'

IGNORE=(
	'HP Victus 16-e0xxx.xml'
)

set -e

type git &>/dev/null || {
	echo 'Program 'git' not found!' >&2
	exit 1
}

type dos2unix &>/dev/null || {
	echo 'Program 'dos2unix' not found!' >&2
	exit 1
}

is_ignored_file() {
	local filename="$(basename "$1")" ignored_file

	for ignored_file in "${IGNORE[@]}"; do
		[[ "$filename" == "$ignored_file" ]] && return 0
	done

	return 1
}

set -x

rm -rf "$OUTDIR"
git clone "$REPO" "$OUTDIR"
chmod -x "$OUTDIR/Configs/"*.xml
dos2unix "$OUTDIR/Configs/"*.xml

set +x 

for file in "$OUTDIR/Configs/"*.xml; do
	if is_ignored_file "$file"; then
		echo "Skipping $file ..."
	else
		echo "Moving $file ..."
		mv "$file" .
	fi
done

rm -rf "$OUTDIR"
