confdir = $(DESTDIR)/etc
bindir =  $(DESTDIR)/usr/bin
sharedir = $(DESTDIR)/usr/share

build: src/nbfc_service src/ec_probe #src/nbfc

install: build
	# Binaries
	mkdir -p $(bindir)
	install nbfc.py           $(bindir)/nbfc
	install src/nbfc_service  $(bindir)/nbfc_service
	install src/ec_probe      $(bindir)/ec_probe
#   install src/nbfc          $(bindir)/nbfc   #client written in c
	
	# /etc/systemd/system
	mkdir -p $(confdir)/systemd/system
	cp etc/systemd/system/nbfc_service.service $(confdir)/systemd/system/nbfc_service.service
	
	# /usr/share/nbfc/configs
	mkdir -p $(sharedir)/nbfc/configs
	cp -r share/nbfc/configs/* $(sharedir)/nbfc/configs
	
	# Documentation
	mkdir -p $(sharedir)/man/man1
	mkdir -p $(sharedir)/man/man5
	cp doc/ec_probe.1            $(sharedir)/man/man1
	cp doc/nbfc.1                $(sharedir)/man/man1
	cp doc/nbfc_service.1        $(sharedir)/man/man1
	cp doc/nbfc_service.json.5   $(sharedir)/man/man5
	
	# Completion
	mkdir -p $(sharedir)/zsh/site-functions
	cp completion/zsh/_nbfc                $(sharedir)/zsh/site-functions/
	cp completion/zsh/_nbfc_service        $(sharedir)/zsh/site-functions/
	cp completion/zsh/_ec_probe            $(sharedir)/zsh/site-functions/
	mkdir -p $(sharedir)/bash-completion/completions
	cp completion/bash/nbfc                $(sharedir)/bash-completion/completions/
	cp completion/bash/nbfc_service        $(sharedir)/bash-completion/completions/
	cp completion/bash/ec_probe            $(sharedir)/bash-completion/completions/
	mkdir -p $(sharedir)/fish/completions
	cp completion/fish/nbfc.fish           $(sharedir)/fish/completions/
	cp completion/fish/nbfc_service.fish   $(sharedir)/fish/completions/
	cp completion/fish/ec_probe.fish       $(sharedir)/fish/completions/

clean:
	rm -rf __pycache__ tools/argparse-tool/__pycache__
	(cd src; make clean)

# =============================================================================
# Binaries ====================================================================
# =============================================================================

src/nbfc_service:
	(cd src; make nbfc_service)

src/ec_probe:
	(cd src; make ec_probe)

src/nbfc:
	(cd src; make nbfc)

# =============================================================================
# Documentation ===============================================================
# =============================================================================

doc: .force
	mkdir -p doc
	
	$(ARGPARSE_TOOL) markdown ./tools/argparse-tool/ec_probe.py     -o doc/ec_probe.md
	$(ARGPARSE_TOOL) markdown ./tools/argparse-tool/nbfc_service.py -o doc/nbfc_service.md
	$(ARGPARSE_TOOL) markdown nbfc.py                               -o doc/nbfc.md
	
	./tools/config_to_md.py > doc/nbfc_service.json.md
	
	go-md2man < doc/ec_probe.md          > doc/ec_probe.1
	go-md2man < doc/nbfc.md              > doc/nbfc.1
	go-md2man < doc/nbfc_service.md      > doc/nbfc_service.1
	go-md2man < doc/nbfc_service.json.md > doc/nbfc_service.json.5

.force:
	# force building targets
