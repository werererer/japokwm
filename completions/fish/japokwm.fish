# japokwm(1) completion

complete -f -c japokwm
complete -c japokwm -s h -l help --description "Show help message and quit."
complete -c japokwm -s c -l config --description "Specifies a config file." -r
complete -c japokwm -s p -l path --description "Check the validity of the config file, then exit." -r
complete -c japokwm -s v -l version --description "Show the version number and quit."

