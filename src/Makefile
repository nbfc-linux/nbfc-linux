# TODO: Check for libsensors and add `-DHAVE_SENSORS` and `-lsensors`

release: nbfc_service ec_probe nbfc
debug:   nbfc_service-debug ec_probe-debug nbfc-debug

confdir = /etc
sharedir = /usr/share
CCFLAGS = $(CFLAGS) -Wall -Wextra -Wfatal-errors -Wpedantic -lm
CFLAGS_MAIN = $(CCFLAGS) -DCONFDIR=\"$(confdir)\" -DDATADIR=\"$(sharedir)\"

nbfc_service-debug: \
	build.c \
	ec_debug.h ec_debug.c \
	ec_dummy.h ec_dummy.c \
	ec_linux.c ec_linux.h \
	ec_sys_linux.c ec_sys_linux.h \
	error.c error.h \
	fan.c fan.h \
	fs_sensors.c fs_sensors.h \
	generated/nbfc_service.help.h \
	generated/model_config.generated.c \
	generated/model_config.generated.h \
	info.c info.h \
	lm_sensors.c lm_sensors.h \
	macros.h \
	main.c \
	memory.c memory.h \
	model_config.c model_config.h \
	nbfc.h \
	nxjson.c nxjson.h \
	nxjson_utils.h \
	service.c service.h \
	service_config.c service_config.h \
	temperature_filter.c temperature_filter.h \
	temperature_threshold_manager.c temperature_threshold_manager.h \
	optparse/optparse.h optparse/optparse.c
	$(CC) $(CFLAGS_MAIN) -Og -g build.c -o nbfc_service-debug

nbfc_service: \
	build.c \
	ec_debug.h ec_debug.c \
	ec_dummy.h ec_dummy.c \
	ec_linux.c ec_linux.h \
	ec_sys_linux.c ec_sys_linux.h \
	error.c error.h \
	fan.c fan.h \
	fs_sensors.c fs_sensors.h \
	generated/nbfc_service.help.h \
	generated/model_config.generated.c \
	generated/model_config.generated.h \
	info.c info.h \
	lm_sensors.c lm_sensors.h \
	macros.h \
	main.c \
	memory.c memory.h \
	model_config.c model_config.h \
	nbfc.h \
	nxjson.c nxjson.h \
	nxjson_utils.h \
	service.c service.h \
	service_config.c service_config.h \
	temperature_filter.c temperature_filter.h \
	temperature_threshold_manager.c temperature_threshold_manager.h \
	optparse/optparse.h optparse/optparse.c
	$(CC) $(CFLAGS_MAIN) -Os -DNDEBUG -s build.c -o nbfc_service

ec_probe-debug: \
	ec_probe.c \
	ec_sys_linux.h ec_sys_linux.c \
	error.h error.c \
	generated/ec_probe.help.h \
	nbfc.h \
	memory.h memory.c \
	optparse/optparse.h optparse/optparse.c
	$(CC) $(CFLAGS_MAIN) -Og -g ec_probe.c -o ec_probe-debug

ec_probe: \
	ec_probe.c \
	ec_sys_linux.h ec_sys_linux.c \
	error.h error.c \
	generated/ec_probe.help.h \
	nbfc.h \
	memory.h memory.c \
	optparse/optparse.h optparse/optparse.c
	$(CC) $(CFLAGS_MAIN) -Os -DNDEBUG -s ec_probe.c -o ec_probe

nbfc-debug: \
	client.c \
	error.h error.c \
	generated/ec_probe.help.h \
	optparse/optparse.h optparse/optparse.c \
	nxjson.c reverse_nxjson.c nxjson.h \
	nbfc.h
	$(CC) $(CFLAGS_MAIN) -Og -g client.c -o nbfc-debug

nbfc: \
	client.c \
	error.h error.c \
	generated/ec_probe.help.h \
	optparse/optparse.h optparse/optparse.c \
	nxjson.c reverse_nxjson.c nxjson.h \
	nbfc.h
	$(CC) $(CFLAGS_MAIN) -Os -DNDEBUG -s client.c -o nbfc

generated/: .force
	mkdir -p generated
	../tools/config.py source > generated/model_config.generated.c
	../tools/config.py header > generated/model_config.generated.h
	argparse-tool printf ../tools/argparse-tool/nbfc_service.py -o generated/nbfc_service.help.h
	argparse-tool printf ../tools/argparse-tool/ec_probe.py     -o generated/ec_probe.help.h

clean:
	rm -f nbfc_service ec_probe nbfc_service-debug ec_probe-debug a.out nbfc nbfc-debug
	rm -f *.o
	(cd optparse; make clean)

clean_generated: clean
	rm -rf generated

.force:
	# Force building targets

#	git clone https://github.com/thestr4ng3r/nxjson
