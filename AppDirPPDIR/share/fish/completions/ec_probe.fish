complete -c ec_probe -s h -l help -d 'show this help message and exit'
complete -c ec_probe -x -s e -l embedded-controller -d 'Specify embedded controller to use' -a 'ec_linux ec_sys_linux'
# command dump
complete -c ec_probe -f -n 'not __fish_seen_subcommand_from dump monitor read write' -d 'Dump all EC registers' -a dump
complete -c ec_probe -n '__fish_seen_subcommand_from dump' -s h -l help -d 'show this help message and exit'
# command read
complete -c ec_probe -f -n 'not __fish_seen_subcommand_from dump monitor read write' -d 'Read a byte from a EC register' -a read
complete -c ec_probe -n '__fish_seen_subcommand_from read' -s h -l help -d 'show this help message and exit'
complete -c ec_probe -x -n '__fish_seen_subcommand_from read && test (__fish_number_of_cmd_args_wo_opts) = 2' -d 'Register source' -a '(seq 0 255)'
# command write
complete -c ec_probe -f -n 'not __fish_seen_subcommand_from dump monitor read write' -d 'Write a byte to a EC register' -a write
complete -c ec_probe -n '__fish_seen_subcommand_from write' -s h -l help -d 'show this help message and exit'
complete -c ec_probe -x -n '__fish_seen_subcommand_from write && test (__fish_number_of_cmd_args_wo_opts) = 2' -d 'Register destination' -a '(seq 0 255)'
complete -c ec_probe -x -n '__fish_seen_subcommand_from write && test (__fish_number_of_cmd_args_wo_opts) = 3' -d 'Value to write' -a '(seq 0 255)'
# command monitor
complete -c ec_probe -f -n 'not __fish_seen_subcommand_from dump monitor read write' -d 'Monitor all EC registers for changes' -a monitor
complete -c ec_probe -n '__fish_seen_subcommand_from monitor' -s h -l help -d 'show this help message and exit'
complete -c ec_probe -r -n '__fish_seen_subcommand_from monitor' -s i -l interval -d 'Monitored timespan'
complete -c ec_probe -r -n '__fish_seen_subcommand_from monitor' -s t -l timespan -d 'Set poll intervall'
complete -c ec_probe -r -n '__fish_seen_subcommand_from monitor' -s r -l report -d 'Save all readings as a CSV file' -F
complete -c ec_probe -n '__fish_seen_subcommand_from monitor' -s c -l clearly -d 'Blanks out consecutive duplicate readings'
complete -c ec_probe -n '__fish_seen_subcommand_from monitor' -s d -l decimal -d 'Output readings in decimal format instead of hexadecimal format'
