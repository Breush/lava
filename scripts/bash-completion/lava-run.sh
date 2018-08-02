# Script for autocompleting lava run.sh script
#
# Add "source path/to/this/script/lava-run.sh" to your .bashrc

_lava_run()
{
    COMPREPLY=()

    local WORD="${COMP_WORDS[COMP_CWORD]}"

    if (( $COMP_CWORD == 1 )); then

        # Find program indentified by the make alias
        local MAKE="make"
        if [ `uname -o` == "Msys" ]; then
            MAKE="mingw32-make"
        fi

        # Find a make target that match the name
        local TARGETS=$(${MAKE} help | grep -P '^   (?!all|clean|lava-)' | grep "$WORD")

        COMPREPLY=($TARGETS)

    elif (( $COMP_CWORD == 2 )); then

        COMPREPLY=( $(compgen -W "debug" -- $WORD) )
  
    fi

    return 0
}

complete -o nospace -F _lava_run ./scripts/run.sh
