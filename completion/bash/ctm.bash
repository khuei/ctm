#!/usr/bin/env bash

_ctm_completion() {
	COMPREPLY=($(compgen -W "addr refresh list read version help" "${COMP_WORDS[1]}"))
	if [[ "${COMP_WORDS[1]}" = "addr" ]]; then
		COMPREPLY=($(compgen -W "create new current select delete" "${COMP_WORDS[2]}"))
	fi
}

complete -F _ctm_completion ctm
