complete -c nbfc -s h -l help -d 'show this help message and exit'
complete -c nbfc -l version -d "show program's version number and exit"
# command start
complete -c nbfc -f -n 'not __fish_seen_subcommand_from config help restart set start status stop' -d 'Start the service' -a start
complete -c nbfc -n '__fish_seen_subcommand_from start' -s h -l help -d 'show this help message and exit'
complete -c nbfc -n '__fish_seen_subcommand_from start' -s r -l readonly -d 'Start in read-only mode'
# command stop
complete -c nbfc -f -n 'not __fish_seen_subcommand_from config help restart set start status stop' -d 'Stop the service' -a stop
complete -c nbfc -n '__fish_seen_subcommand_from stop' -s h -l help -d 'show this help message and exit'
# command restart
complete -c nbfc -f -n 'not __fish_seen_subcommand_from config help restart set start status stop' -d 'Restart the service' -a restart
complete -c nbfc -n '__fish_seen_subcommand_from restart' -s h -l help -d 'show this help message and exit'
complete -c nbfc -n '__fish_seen_subcommand_from restart' -s r -l readonly -d 'Restart in read-only mode'
# command status
complete -c nbfc -f -n 'not __fish_seen_subcommand_from config help restart set start status stop' -d 'Show the service status' -a status
complete -c nbfc -n '__fish_seen_subcommand_from status' -s h -l help -d 'show this help message and exit'
complete -c nbfc -n '__fish_seen_subcommand_from status && not __fish_contains_opt fan service f s' -s a -l all -d 'Show service and fan status (default)'
complete -c nbfc -n '__fish_seen_subcommand_from status && not __fish_contains_opt all fan a f' -s s -l service -d 'Show service status'
complete -c nbfc -r -n '__fish_seen_subcommand_from status && not __fish_contains_opt all service a s' -s f -l fan -d 'Show fan status'
complete -c nbfc -r -n '__fish_seen_subcommand_from status' -s w -l watch -d 'Show status periodically'
# command config
complete -c nbfc -f -n 'not __fish_seen_subcommand_from config help restart set start status stop' -d 'List or apply configs' -a config
complete -c nbfc -n '__fish_seen_subcommand_from config' -s h -l help -d 'show this help message and exit'
complete -c nbfc -n '__fish_seen_subcommand_from config && not __fish_contains_opt apply recommend set a r s' -s l -l list -d 'List all available configs (default)'
complete -c nbfc -r -n '__fish_seen_subcommand_from config && not __fish_contains_opt apply list recommend a l r' -s s -l set -d 'Set a config' -F
complete -c nbfc -r -n '__fish_seen_subcommand_from config && not __fish_contains_opt list recommend set l r s' -s a -l apply -d 'Set a config and enable fan control' -F
complete -c nbfc -n '__fish_seen_subcommand_from config && not __fish_contains_opt apply list set a l s' -s r -l recommend -d 'List configs which may work for your device'
# command set
complete -c nbfc -f -n 'not __fish_seen_subcommand_from config help restart set start status stop' -d 'Control fan speed' -a set
complete -c nbfc -n '__fish_seen_subcommand_from set' -s h -l help -d 'show this help message and exit'
complete -c nbfc -n '__fish_seen_subcommand_from set && not __fish_contains_opt speed s' -s a -l auto -d "Set fan speed to 'auto'"
complete -c nbfc -r -n '__fish_seen_subcommand_from set && not __fish_contains_opt auto a' -s s -l speed -d 'Set fan speed to PERCENT'
complete -c nbfc -r -n '__fish_seen_subcommand_from set' -s f -l fan -d 'Fan index (zero based)'
# command help
complete -c nbfc -f -n 'not __fish_seen_subcommand_from config help restart set start status stop' -d 'Show help' -a help
complete -c nbfc -n '__fish_seen_subcommand_from help' -s h -l help -d 'show this help message and exit'
