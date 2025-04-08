version     = 0.2.9

PREFIX      = /usr

bindir			= $(PREFIX)/bin
confdir 		= $(PREFIX)/etc
datadir			= $(PREFIX)/share
sysddir 		= $(PREFIX)/lib/systemd/system
mandir 			= $(PREFIX)/share/man
man1dir     = $(mandir)/man1
man5dir     = $(mandir)/man5
runstatedir	= /var/run
orcdir   		= $(confdir)/init.d
systemvdir  = $(confdir)/init.d

INIT_SYSTEM = systemd # systemd, systemv, openrc

ifeq ($(BUILD), debug)
	CFLAGS   = -Og -g
else
	CPPFLAGS = -DNDEBUG
	CFLAGS   = -Wall -Os -flto
	LDFLAGS  = -s
endif

override LDLIBS   += -lm
override CPPFLAGS += \
	-DSYSCONFDIR=\"$(confdir)\"      \
	-DDATADIR=\"$(datadir)\"         \
	-DRUNSTATEDIR=\"$(runstatedir)\" \
	-DVERSION=\"$(version)\"

CORE  = src/nbfc_service src/nbfc src/ec_probe src/test_model_config nbfc-gui/nbfc-qt.py
DOC   = doc/ec_probe.1 doc/nbfc.1 doc/nbfc_service.1 doc/nbfc_service.json.5
SYSTEMD = etc/systemd/system/nbfc_service.service
OPEN_RC = etc/init.d/nbfc_service.openrc
SYSTEMV = etc/init.d/nbfc_service.systemv
BASH_COMPLETION = completion/bash/ec_probe completion/bash/nbfc completion/bash/nbfc_service
FISH_COMPLETION = completion/fish/ec_probe.fish completion/fish/nbfc.fish completion/fish/nbfc_service.fish
ZSH_COMPLETION = completion/zsh/_ec_probe completion/zsh/_nbfc completion/zsh/_nbfc_service

all: deprecation_warning $(CORE) $(DOC) $(SYSTEMD) $(OPEN_RC) $(SYSTEMV) $(BASH_COMPLETION) $(FISH_COMPLETION) $(ZSH_COMPLETION)

install-core: $(CORE)
	install -Dm 755 src/nbfc_service    $(DESTDIR)$(bindir)/nbfc_service
	install -Dm 755 src/ec_probe        $(DESTDIR)$(bindir)/ec_probe
	install -Dm 755 src/nbfc            $(DESTDIR)$(bindir)/nbfc
	install -Dm 755 nbfc-gui/nbfc-qt.py $(DESTDIR)$(bindir)/nbfc-qt

REPLACE_VARS = sed \
	-e 's|@BINDIR@|$(bindir)|g'           \
	-e 's|@DATADIR@|$(datadir)|g'         \
	-e 's|@SYSCONFDIR@|$(confdir)|g'      \
	-e 's|@RUNSTATEDIR@|$(runstatedir)|g' \
	-e 's|@VERSION@|$(version)|g'

nbfc.py: nbfc.py.in
	$(REPLACE_VARS) < $< > $@

nbfc-gui/nbfc-qt.py: nbfc-gui/about.py nbfc-gui/common.py nbfc-gui/fs_sensors.py nbfc-gui/nbfc_client.py nbfc-gui/qt.py
	(cd ./nbfc-gui; python3 ./include_files.py qt.py > nbfc-qt.py)
	chmod +x ./nbfc-gui/nbfc-qt.py

# Documentation ###############################################################
doc/ec_probe.1: doc/ec_probe.1.in
	$(REPLACE_VARS) < $< > $@

doc/nbfc.1: doc/nbfc.1.in
	$(REPLACE_VARS) < $< > $@

doc/nbfc_service.1: doc/nbfc_service.1.in
	$(REPLACE_VARS) < $< > $@

doc/nbfc_service.json.5: doc/nbfc_service.json.5.in
	$(REPLACE_VARS) < $< > $@

# BASH completion files #######################################################
completion/bash/ec_probe: completion/bash/ec_probe.in
	$(REPLACE_VARS) < $< > $@

completion/bash/nbfc: completion/bash/nbfc.in
	$(REPLACE_VARS) < $< > $@

completion/bash/nbfc_service: completion/bash/nbfc_service.in
	$(REPLACE_VARS) < $< > $@

# FISH completion files #######################################################
completion/fish/ec_probe.fish: completion/fish/ec_probe.fish.in
	$(REPLACE_VARS) < $< > $@

completion/fish/nbfc.fish: completion/fish/nbfc.fish.in
	$(REPLACE_VARS) < $< > $@

completion/fish/nbfc_service.fish: completion/fish/nbfc_service.fish.in
	$(REPLACE_VARS) < $< > $@

# ZSH completion files ########################################################
completion/zsh/_ec_probe: completion/zsh/_ec_probe.in
	$(REPLACE_VARS) < $< > $@

completion/zsh/_nbfc: completion/zsh/_nbfc.in
	$(REPLACE_VARS) < $< > $@

completion/zsh/_nbfc_service: completion/zsh/_nbfc_service.in
	$(REPLACE_VARS) < $< > $@

# Systemd service file ########################################################
etc/systemd/system/nbfc_service.service: etc/systemd/system/nbfc_service.service.in
	$(REPLACE_VARS) < $< > $@

# OpenRC service file #########################################################
etc/init.d/nbfc_service.openrc: etc/init.d/nbfc_service.openrc.in
	$(REPLACE_VARS) < $< > $@

# SystemV service file ########################################################
etc/init.d/nbfc_service.systemv: etc/init.d/nbfc_service.systemv.in
	$(REPLACE_VARS) < $< > $@

install-configs:
	# /usr/local/etc/nbfc
	mkdir -p $(DESTDIR)$(confdir)/nbfc

	# /usr/local/share/nbfc/configs
	mkdir -p $(DESTDIR)$(datadir)/nbfc/configs
	cp share/nbfc/model_support.json $(DESTDIR)$(datadir)/nbfc
	cp -r share/nbfc/configs/* $(DESTDIR)$(datadir)/nbfc/configs

install-systemd: etc/systemd/system/nbfc_service.service
	# /usr/local/lib/systemd/system
	install -Dm 644 etc/systemd/system/nbfc_service.service $(DESTDIR)$(sysddir)/nbfc_service.service

install-openrc: etc/init.d/nbfc_service.openrc
	# /usr/local/etc/init.d
	install -Dm 755 etc/init.d/nbfc_service.openrc		 $(DESTDIR)$(orcdir)/nbfc_service

install-systemv: etc/init.d/nbfc_service.systemv
	# /usr/local/etc/init.d
	install -Dm 755 etc/init.d/nbfc_service.systemv	 $(DESTDIR)$(systemvdir)/nbfc_service

install-docs: $(DOC)
	install -Dm 644 doc/ec_probe.1           $(DESTDIR)$(man1dir)/ec_probe.1
	install -Dm 644 doc/nbfc.1               $(DESTDIR)$(man1dir)/nbfc.1
	install -Dm 644 doc/nbfc_service.1       $(DESTDIR)$(man1dir)/nbfc_service.1
	install -Dm 644 doc/nbfc_service.json.5  $(DESTDIR)$(man5dir)/nbfc_service.json.5

install-completion: $(BASH_COMPLETION) $(FISH_COMPLETION) $(ZSH_COMPLETION)
	# ZSH
	install -Dm 644 completion/zsh/_nbfc               $(DESTDIR)$(datadir)/zsh/site-functions/_nbfc
	install -Dm 644 completion/zsh/_nbfc_service       $(DESTDIR)$(datadir)/zsh/site-functions/_nbfc_service
	install -Dm 644 completion/zsh/_ec_probe           $(DESTDIR)$(datadir)/zsh/site-functions/_ec_probe
	# BASH	
	install -Dm 644 completion/bash/nbfc               $(DESTDIR)$(datadir)/bash-completion/completions/nbfc
	install -Dm 644 completion/bash/nbfc_service       $(DESTDIR)$(datadir)/bash-completion/completions/nbfc_service
	install -Dm 644 completion/bash/ec_probe           $(DESTDIR)$(datadir)/bash-completion/completions/ec_probe
	# FISH	
	install -Dm 644 completion/fish/nbfc.fish          $(DESTDIR)$(datadir)/fish/vendor_completions.d/nbfc.fish
	install -Dm 644 completion/fish/nbfc_service.fish  $(DESTDIR)$(datadir)/fish/vendor_completions.d/nbfc_service.fish
	install -Dm 644 completion/fish/ec_probe.fish      $(DESTDIR)$(datadir)/fish/vendor_completions.d/ec_probe.fish

install: install-core install-configs install-$(INIT_SYSTEM) install-docs install-completion

uninstall:
	# Binaries
	rm -f $(DESTDIR)$(bindir)/nbfc
	rm -f $(DESTDIR)$(bindir)/nbfc_config
	rm -f $(DESTDIR)$(bindir)/nbfc_service
	rm -f $(DESTDIR)$(bindir)/ec_probe
	rm -f $(DESTDIR)$(bindir)/nbfc-qt

	# /usr/local/lib/systemd/system
	rm -f $(DESTDIR)$(sysddir)/nbfc_service.service

	# /usr/local/etc/init.d
	rm -f $(DESTDIR)$(orcdir)/nbfc_service

	# /usr/local/etc/init.d
	rm -f $(DESTDIR)$(systemvdir)/nbfc_service

	# /usr/local/share/nbfc/configs
	rm -rf $(DESTDIR)$(datadir)/nbfc

	# /usr/local/etc/nbfc
	rm -rf $(DESTDIR)$(confdir)/nbfc

	# Documentation
	rm -f $(DESTDIR)$(man1dir)/ec_probe.1
	rm -f $(DESTDIR)$(man1dir)/nbfc.1
	rm -f $(DESTDIR)$(man1dir)/nbfc_service.1
	rm -f $(DESTDIR)$(man5dir)/nbfc_service.json.5

	# Completion
	rm -f $(DESTDIR)$(datadir)/zsh/site-functions/_nbfc
	rm -f $(DESTDIR)$(datadir)/zsh/site-functions/_nbfc_service
	rm -f $(DESTDIR)$(datadir)/zsh/site-functions/_ec_probe

	rm -f $(DESTDIR)$(datadir)/bash-completion/completions/nbfc
	rm -f $(DESTDIR)$(datadir)/bash-completion/completions/nbfc_service
	rm -f $(DESTDIR)$(datadir)/bash-completion/completions/ec_probe

	rm -f $(DESTDIR)$(datadir)/fish/completions/nbfc.fish
	rm -f $(DESTDIR)$(datadir)/fish/completions/nbfc_service.fish
	rm -f $(DESTDIR)$(datadir)/fish/completions/ec_probe.fish

clean:
	rm -rf __pycache__
	rm -f $(CORE) nbfc.py src/*.o
	rm -f $(BASH_COMPLETION) $(FISH_COMPLETION) $(ZSH_COMPLETION)
	rm -f $(SYSTEMD) $(OPEN_RC) $(SYSTEMV)
	rm -f $(DOC)

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
	src/fan_temperature_control.h \
	src/fan_temperature_control.c \
	src/fs_sensors.c src/fs_sensors.h \
	src/generated/model_config.generated.c \
	src/generated/model_config.generated.h \
	src/help/nbfc_service.help.h \
	src/lm_sensors.c src/lm_sensors.h \
	src/macros.h \
	src/main.c \
	src/memory.c src/memory.h \
	src/model_config.c src/model_config.h \
	src/nbfc.h \
	src/nxjson.c src/nxjson.h \
	src/nxjson_utils.h \
	src/pidfile.c src/pidfile.h \
	src/protocol.c src/protocol.h \
	src/server.c src/server.h \
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
	src/help/ec_probe.help.h \
	src/nbfc.h \
	src/memory.h src/memory.c \
	src/optparse/optparse.h src/optparse/optparse.c
	$(CC) $(CPPFLAGS) $(CFLAGS) src/ec_probe.c -o src/ec_probe $(LDLIBS) $(LDFLAGS)

src/nbfc: \
	src/client.c \
	src/error.h src/error.c \
	src/help/ec_probe.help.h \
	src/optparse/optparse.h src/optparse/optparse.c \
	src/protocol.c src/protocol.h \
	src/nxjson.c src/reverse_nxjson.c src/nxjson.h \
	src/nbfc.h
	$(CC) $(CPPFLAGS) $(CFLAGS) src/client.c -o src/nbfc $(LDLIBS) $(LDFLAGS)

src/test_model_config: \
	src/error.c \
	src/generated/model_config.generated.h \
	src/generated/model_config.generated.c \
	src/memory.c \
	src/nxjson.c \
	src/program_name.c
	$(CC) $(CPPFLAGS) $(CFLAGS) src/test_model_config.c -o src/test_model_config $(LDLIBS) $(LDFLAGS)

src/generated/: .force
	mkdir -p src/generated
	./tools/config.py source > src/generated/model_config.generated.c
	./tools/config.py header > src/generated/model_config.generated.h

# =============================================================================
# Documentation ===============================================================
# =============================================================================

doc: doc/ec_probe.1 doc/nbfc.1 doc/nbfc_service.1 doc/nbfc_service.json.5
	pandoc -f man -t markdown doc/ec_probe.1 					> doc/ec_probe.1.md
	pandoc -f man -t markdown doc/nbfc.1 							> doc/nbfc.1.md
	pandoc -f man -t markdown doc/nbfc_service.1 			> doc/nbfc_service.1.md
	pandoc -f man -t markdown doc/nbfc_service.json.5 > doc/nbfc_service.json.5.md

	pandoc -f man -t html doc/ec_probe.1 					> doc/ec_probe.1.html
	pandoc -f man -t html doc/nbfc.1 							> doc/nbfc.1.html
	pandoc -f man -t html doc/nbfc_service.1 			> doc/nbfc_service.1.html
	pandoc -f man -t html doc/nbfc_service.json.5 > doc/nbfc_service.json.5.html

deprecation_warning:
	@echo
	@echo =======================================================================
	@echo This Makefile is deprecated. Use ./configure for future builds!
	@echo =======================================================================
	@echo

.force:
	# force building targets
