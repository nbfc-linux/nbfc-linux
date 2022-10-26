complete -c nbfc_service -s h -l help -d 'show this help message and exit'
complete -c nbfc_service -s r -l readonly -d 'Start in read-only mode'
complete -c nbfc_service -s f -l fork -d 'Switch process to background after sucessfully started'
complete -c nbfc_service -s d -l debug -d 'Enable tracing of reads and writes of the embedded controller'
complete -c nbfc_service -r -s c -l config-file -d 'Use alternative config file (default /etc/nbfc/nbfc.json)' -F
complete -c nbfc_service -r -s s -l state-file -d 'Write state to an alternative file (default /var/run/nbfc_service.state.json)' -F
complete -c nbfc_service -x -s e -l embedded-controller -d 'Specify embedded controller to use' -a 'dummy ec_linux ec_sys_linux'
