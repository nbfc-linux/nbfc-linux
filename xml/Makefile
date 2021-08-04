fetch_from_original_nbfc:
	#[ -e nbfc ] || git clone https://github.com/hirschmann/nbfc
	#mv nbfc/Configs/* .
	#rm -rf nbfc
	
	git clone https://github.com/UraniumDonut/nbfc-revive
	mv nbfc-revive/Configs/* .
	rm -rf nbfc-revive
	
	chmod -x *.xml

transform_to_json:
	../tools/config_to_json.py --out-dir ../share/nbfc/configs *
	../tools/config_to_json.py --out-dir ../share/nbfc/configs unmerged/*

