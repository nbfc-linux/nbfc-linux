#!/bin/bash

OUTDIR='../share/nbfc/configs'

IGNORE=(
	'HP Victus 16-e0175AX.xml'
	'HP Victus 16-e0xxx.xml'
	'Lenovo Yoga Slim7.xml'
)

is_ignored_file() {
	local filename="$(basename "$1")" ignored_file

	for ignored_file in "${IGNORE[@]}"; do
		[[ "$filename" == "$ignored_file" ]] && return 0
	done

	return 1
}

for file in *.xml unmerged/*.xml; do
	if is_ignored_file "$file"; then
		echo "Skipping $file ..."
	else
		echo "Converting $file ..."
		../tools/config_to_json.py --out-dir "$OUTDIR" "$file"
	fi
done
