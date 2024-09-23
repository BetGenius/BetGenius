# bash programmable completion for betgeniusd(1) and betgenius-qt(1)
# Copyright (c) 2012-2022 The Bitcoin Core developers
# Copyright (c) 2024 The Betgenius Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

_betgeniusd() {
    local cur prev words=() cword
    local betgeniusd

    # save and use original argument to invoke betgeniusd for -help
    # it might not be in $PATH
    betgeniusd="$1"

    COMPREPLY=()
    _get_comp_words_by_ref -n = cur prev words cword

    case "$cur" in
        -conf=*|-pid=*|-loadblock=*|-rpccookiefile=*|-wallet=*)
            cur="${cur#*=}"
            _filedir
            return 0
            ;;
        -datadir=*)
            cur="${cur#*=}"
            _filedir -d
            return 0
            ;;
        -*=*)	# prevent nonsense completions
            return 0
            ;;
        *)

            # only parse -help if sensible
            if [[ -z "$cur" || "$cur" =~ ^- ]]; then
                local helpopts
                helpopts=$($betgeniusd -help 2>&1 | awk '$1 ~ /^-/ { sub(/=.*/, "="); print $1 }' )
                COMPREPLY=( $( compgen -W "$helpopts" -- "$cur" ) )
            fi

            # Prevent space if an argument is desired
            if [[ $COMPREPLY == *= ]]; then
                compopt -o nospace
            fi
            return 0
            ;;
    esac
} &&
complete -F _betgeniusd betgeniusd betgenius-qt

# Local variables:
# mode: shell-script
# sh-basic-offset: 4
# sh-indent-comment: t
# indent-tabs-mode: nil
# End:
# ex: ts=4 sw=4 et filetype=sh
