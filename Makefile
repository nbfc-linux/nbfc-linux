confdir = $(DESTDIR)/etc
bindir =  $(DESTDIR)/usr/bin
sharedir = $(DESTDIR)/usr/share

CFLAGS_MAIN = $(CCFLAGS) -DCONFDIR=\"$(confdir)\" -DDATADIR=\"$(sharedir)\"

PROGS = src/nbfc_service src/ec_probe #src/nbfc

all: $(PROGS)

install: $(PROGS)
	# Binaries
	mkdir -p $(bindir)
	install nbfc.py           $(bindir)/nbfc
	install src/nbfc_service  $(bindir)/nbfc_service
	install src/ec_probe      $(bindir)/ec_probe
	#install src/nbfc          $(bindir)/nbfc   #client written in c
	
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

uninstall:
	# Binaries
	rm $(bindir)/nbfc
	rm $(bindir)/nbfc_service
	rm $(bindir)/ec_probe
	
	# /etc/systemd/system
	rm $(confdir)/systemd/system/nbfc_service.service
	
	# /usr/share/nbfc/configs
	rm -r $(sharedir)/nbfc
	
	# Documentation
	rm $(sharedir)/man/man1/ec_probe.1
	rm $(sharedir)/man/man1/nbfc.1
	rm $(sharedir)/man/man1/nbfc_service.1
	rm $(sharedir)/man/man5/nbfc_service.json.5
	
	# Completion
	rm $(sharedir)/zsh/site-functions/_nbfc
	rm $(sharedir)/zsh/site-functions/_nbfc_service
	rm $(sharedir)/zsh/site-functions/_ec_probe
	
	rm $(sharedir)/bash-completion/completions/nbfc
	rm $(sharedir)/bash-completion/completions/nbfc_service
	rm $(sharedir)/bash-completion/completions/ec_probe
	 
	rm $(sharedir)/fish/completions/nbfc.fish
	rm $(sharedir)/fish/completions/nbfc_service.fish
	rm $(sharedir)/fish/completions/ec_probe.fish

clean:
	rm -rf __pycache__ tools/argparse-tool/__pycache__
	rm -f $(PROGS) src/*.o

# =============================================================================
# Binaries ====================================================================
# =============================================================================

src/nbfc_service: \
	src/build.c \
	src/ec_debug.h src/ec_debug.c \
	src/ec_dummy.h src/ec_dummy.c \
	src/ec_linux.c src/ec_linux.h \
	src/ec_sys_linux.c src/ec_sys_linux.h \
	src/error.c src/error.h \
	src/fan.c src/fan.h \
	src/fs_sensors.c src/fs_sensors.h \
	src/generated/nbfc_service.help.h \
	src/generated/model_config.generated.c \
	src/generated/model_config.generated.h \
	src/info.c src/info.h \
	src/lm_sensors.c src/lm_sensors.h \
	src/macros.h \
	src/main.c \
	src/memory.c src/memory.h \
	src/model_config.c src/model_config.h \
	src/nbfc.h \
	src/nxjson.c src/nxjson.h \
	src/nxjson_utils.h \
	src/service.c src/service.h \
	src/service_config.c src/service_config.h \
	src/temperature_filter.c src/temperature_filter.h \
	src/temperature_threshold_manager.c src/temperature_threshold_manager.h \
	src/optparse/optparse.h src/optparse/optparse.c
	$(CC) $(CPPFLAGS) $(CFLAGS) src/build.c -o src/nbfc_service $(LDFLAGS) $(CFLAGS_MAIN)

src/ec_probe: \
	src/ec_probe.c \
	src/ec_sys_linux.h src/ec_sys_linux.c \
	src/error.h src/error.c \
	src/generated/ec_probe.help.h \
	src/nbfc.h \
	src/memory.h src/memory.c \
	src/optparse/optparse.h src/optparse/optparse.c
	$(CC) $(CPPFLAGS) $(CFLAGS) src/ec_probe.c -o src/ec_probe $(LDFLAGS) $(CFLAGS_MAIN)
src/nbfc: \
	src/client.c \
	src/error.h src/error.c \
	src/generated/ec_probe.help.h \
	src/optparse/optparse.h src/optparse/optparse.c \
	src/nxjson.c src/reverse_nxjson.c src/nxjson.h \
	src/nbfc.h
	$(CC) $(CPPFLAGS) $(CFLAGS) src/client.c -o src/nbfc $(LDFLAGS) $(CFLAGS_MAIN)

src/generated/: .force
	mkdir -p src/generated
	./tools/config.py source > src/generated/model_config.generated.c
	./tools/config.py header > src/generated/model_config.generated.h
	argparse-tool printf ./tools/argparse-tool/nbfc_service.py -o src/generated/nbfc_service.help.h
	argparse-tool printf ./tools/argparse-tool/ec_probe.py     -o src/generated/ec_probe.help.h


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
