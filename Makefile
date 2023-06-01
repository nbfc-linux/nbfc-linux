PREFIX   = /usr/local

bindir   = $(PREFIX)/bin
confdir  = $(PREFIX)/etc
sharedir = $(PREFIX)/share
sysddir  = $(PREFIX)/lib/systemd/system

ifeq ($(BUILD), debug)
	CFLAGS   = -Og -g
else
	CPPFLAGS = -DNDEBUG
	CFLAGS   = -Os -flto
	LDFLAGS  = -s
endif

override LDLIBS   += -lm
override CPPFLAGS += -DCONFDIR=\"$(confdir)\" -DDATADIR=\"$(sharedir)\"

CORE  = src/nbfc_service src/ec_probe
PROGS = $(CORE) src/nbfc

all: $(PROGS)

install-core: $(CORE)
	install -Dm 755 src/nbfc_service  $(DESTDIR)$(bindir)/nbfc_service
	install -Dm 755 src/ec_probe      $(DESTDIR)$(bindir)/ec_probe
	install -Dm 755 src/nbfc          $(DESTDIR)$(bindir)/nbfc

nbfc.py: nbfc.py.in
	sed 's:@CONFDIR@:'$(confdir)':; s:@DATADIR@:'$(sharedir)':' < $< >$@

install-configs:
	# /usr/local/etc/nbfc
	mkdir -p $(DESTDIR)$(confdir)/nbfc

	# /usr/local/share/nbfc/configs
	mkdir -p $(DESTDIR)$(sharedir)/nbfc/configs
	cp -r share/nbfc/configs/* $(DESTDIR)$(sharedir)/nbfc/configs

nbfc_service.service: etc/systemd/system/nbfc_service.service.in
	sed 's:@BINDIR@:'$(bindir)':' < $< >$@

install-systemd:    nbfc_service.service
	# /usr/local/lib/systemd/system
	install -Dm 644 nbfc_service.service     $(DESTDIR)$(sysddir)/nbfc_service.service

install-docs:
	install -Dm 644 doc/ec_probe.1           $(DESTDIR)$(sharedir)/man/man1/ec_probe.1
	install -Dm 644 doc/nbfc.1               $(DESTDIR)$(sharedir)/man/man1/nbfc.1
	install -Dm 644 doc/nbfc_service.1       $(DESTDIR)$(sharedir)/man/man1/nbfc_service.1
	install -Dm 644 doc/nbfc_service.json.5  $(DESTDIR)$(sharedir)/man/man5/nbfc_service.json.5

install-completion:
	mkdir -p $(DESTDIR)$(sharedir)/zsh/site-functions
	cp completion/zsh/_nbfc               $(DESTDIR)$(sharedir)/zsh/site-functions/
	cp completion/zsh/_nbfc_service       $(DESTDIR)$(sharedir)/zsh/site-functions/
	cp completion/zsh/_ec_probe           $(DESTDIR)$(sharedir)/zsh/site-functions/
	mkdir -p $(DESTDIR)$(sharedir)/bash-completion/completions
	cp completion/bash/nbfc               $(DESTDIR)$(sharedir)/bash-completion/completions/
	cp completion/bash/nbfc_service       $(DESTDIR)$(sharedir)/bash-completion/completions/
	cp completion/bash/ec_probe           $(DESTDIR)$(sharedir)/bash-completion/completions/
	mkdir -p $(DESTDIR)$(sharedir)/fish/completions
	cp completion/fish/nbfc.fish          $(DESTDIR)$(sharedir)/fish/completions/
	cp completion/fish/nbfc_service.fish  $(DESTDIR)$(sharedir)/fish/completions/
	cp completion/fish/ec_probe.fish      $(DESTDIR)$(sharedir)/fish/completions/

install: install-core install-configs install-systemd install-docs install-completion

uninstall:
	# Binaries
	rm -f $(DESTDIR)$(bindir)/nbfc
	rm -f $(DESTDIR)$(bindir)/nbfc_config
	rm -f $(DESTDIR)$(bindir)/nbfc_service
	rm -f $(DESTDIR)$(bindir)/ec_probe
	
	# /usr/local/lib/systemd/system
	rm -f $(DESTDIR)$(sysddir)/nbfc_service.service
	
	# /usr/local/share/nbfc/configs
	rm -rf $(DESTDIR)$(sharedir)/nbfc
	
	# /usr/local/etc/nbfc
	rm -rf $(DESTDIR)$(confdir)/nbfc
	
	# Documentation
	rm -f $(DESTDIR)$(sharedir)/man/man1/ec_probe.1
	rm -f $(DESTDIR)$(sharedir)/man/man1/nbfc.1
	rm -f $(DESTDIR)$(sharedir)/man/man1/nbfc_service.1
	rm -f $(DESTDIR)$(sharedir)/man/man5/nbfc_service.json.5
	
	# Completion
	rm -f $(DESTDIR)$(sharedir)/zsh/site-functions/_nbfc
	rm -f $(DESTDIR)$(sharedir)/zsh/site-functions/_nbfc_service
	rm -f $(DESTDIR)$(sharedir)/zsh/site-functions/_ec_probe
	
	rm -f $(DESTDIR)$(sharedir)/bash-completion/completions/nbfc
	rm -f $(DESTDIR)$(sharedir)/bash-completion/completions/nbfc_service
	rm -f $(DESTDIR)$(sharedir)/bash-completion/completions/ec_probe
	 
	rm -f $(DESTDIR)$(sharedir)/fish/completions/nbfc.fish
	rm -f $(DESTDIR)$(sharedir)/fish/completions/nbfc_service.fish
	rm -f $(DESTDIR)$(sharedir)/fish/completions/ec_probe.fish

clean:
	rm -rf __pycache__ tools/argparse-tool/__pycache__
	rm -f $(PROGS) src/*.o nbfc_service.service nbfc.py

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
	$(CC) $(CPPFLAGS) $(CFLAGS) src/build.c -o src/nbfc_service $(LDLIBS) $(LDFLAGS)

src/ec_probe: \
	src/ec_probe.c \
	src/ec_sys_linux.h src/ec_sys_linux.c \
	src/error.h src/error.c \
	src/generated/ec_probe.help.h \
	src/nbfc.h \
	src/memory.h src/memory.c \
	src/optparse/optparse.h src/optparse/optparse.c
	$(CC) $(CPPFLAGS) $(CFLAGS) src/ec_probe.c -o src/ec_probe $(LDLIBS) $(LDFLAGS)

src/nbfc: \
	src/client.c \
	src/error.h src/error.c \
	src/generated/ec_probe.help.h \
	src/optparse/optparse.h src/optparse/optparse.c \
	src/nxjson.c src/reverse_nxjson.c src/nxjson.h \
	src/nbfc.h
	$(CC) $(CPPFLAGS) $(CFLAGS) src/client.c -o src/nbfc $(LDLIBS) $(LDFLAGS)

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
