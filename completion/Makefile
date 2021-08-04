ARGPARSE_TOOL = argparse-tool

completion: .force
	mkdir -p bash fish zsh
	
	$(ARGPARSE_TOOL) zsh  ../nbfc.py -o zsh/_nbfc
	$(ARGPARSE_TOOL) fish ../nbfc.py -o fish/nbfc.fish
	$(ARGPARSE_TOOL) bash ../nbfc.py -o bash/nbfc
	
	$(ARGPARSE_TOOL) zsh  nbfc_service.py -o zsh/_nbfc_service
	$(ARGPARSE_TOOL) fish nbfc_service.py -o fish/nbfc_service.fish
	$(ARGPARSE_TOOL) bash nbfc_service.py -o bash/nbfc_service
	
	$(ARGPARSE_TOOL) zsh  ec_probe.py -o zsh/_ec_probe
	$(ARGPARSE_TOOL) fish ec_probe.py -o fish/ec_probe.fish
	$(ARGPARSE_TOOL) bash ec_probe.py -o bash/ec_probe

clean:
	rm -rf bash fish zsh

.force:
	# force building targets
