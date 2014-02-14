# Copyright 2014 the Samizdat Authors (Dan Bornstein et alia).
# Licensed AS IS and WITHOUT WARRANTY under the Apache License,
# Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

#
# Helper library. Meant to be included via `.` command.
#


#
# OS detection
#

# OS flavor; either `bsd` or `linux`.
OS_FLAVOR=linux

if ! ls --version > /dev/null 2>&1; then
    OS_FLAVOR=bsd
fi


#
# Exported functions: General utilities
#

# Mangles an arbitrary string into a valid variable name.
function mangle-string {
    local s="$1"

    printf '__'

    while [[ ${s} != '' ]]; do
        # Emit a chunk of valid characters (if any).
        local chunk="${s%%[^A-Za-z0-9]*}"
        if [[ ${chunk} != '' ]]; then
            printf '%s' "${chunk}"
            s="${s:${#chunk}}"
        fi

        # Emit the next character (if any) as an escape.
        if [[ ${s} != '' ]]; then
            local c="${s:0:1}"
            printf '_%02x' "'${c}"
            s="${s:1}"
        fi
    done
}

# Gets the modification time of the given file as seconds since the Unix
# Epoch.
function mod-time {
    local file="$1"

    if [[ ${OS_FLAVOR} == bsd ]]; then
        stat -f %m "${file}"
    else
        stat --format=%Y "${file}"
    fi
}

# Makes an absolute path out of the given path, which might be either absolute
# or relative. This does not resolve symlinks but does flatten away `.` and
# `..` components.
function abs-path {
    local inDir='.'

    if [[ "$1" =~ --in-dir=(.*) ]]; then
        inDir="${BASH_REMATCH[1]}"
        shift
    fi

    local path="$1"

    if [[ ! ${path} =~ ^/ ]]; then
        if [[ ! ${inDir} =~ ^/ ]]; then
            inDir="${PWD}/${inDir}"
        fi
        path="${inDir}/${path}"
    fi

    local result=() at=0
    while [[ ${path} != '' ]]; do
        if [[ ${path} =~ ^// ]]; then
            path="${path:1}"
        elif [[ ${path} =~ ^/\./ ]]; then
            path="${path:2}"
        elif [[ ${path} == '/.' ]]; then
            path=''
        elif [[ ${path} =~ ^/\.\./ ]]; then
            if (( at > 0 )); then
                (( at-- ))
            fi
            path="${path:3}"
        elif [[ ${path} == '/..' ]]; then
            if (( at > 0 )); then
                (( at-- ))
                unset result[$at]
            fi
            path=''
        elif [[ ${path} == '/' ]]; then
            result[$at]=''
            path=''
            (( at++ ))
        else
            [[ ${path} =~ /([^/]*)(.*) ]]
            result[$at]="${BASH_REMATCH[1]}"
            path="${BASH_REMATCH[2]}"
            (( at++ ))
        fi
    done

    printf '/%s' "${result[@]}"
    printf '\n'
}

# Quotes each of the given arguments as strings.
function quote {
    printf '%q' "$1"
    shift

    if (( $# > 0 )); then
        printf ' %q' "$@"
    fi
}

# Unquotes the given string.
function unquote {
    local s="$1"

    if [[ $s == "''" ]]; then
        # Empty string.
        return
    fi

    if [[ $s =~ ^"$'"(.*)"'"$ ]]; then
        # Remove `$'...'` cladding.
        s="${BASH_REMATCH[1]}"
    fi

    if [[ ! $s =~ '\' ]]; then
        # Easy out: No escapes.
        printf '%s' "$s"
        return
    fi

    local i result=''

    for (( i = 0; i < ${#s}; i++ )); do
        local c="${s:$i:1}"
        if [[ $c == '\' ]]; then
            ((i++))
            c="${s:$i:1}"
            case "$c" in
                ('n') c=$'\n' ;;
                ('r') c=$'\r' ;;
                ('t') c=$'\t' ;;
                (' '|'"'|"'") ;; # Pass through as-is.
                (*)
                    echo "Unknown character escape: ${c}" 1>&2
                    exit 1
                    ;;
            esac
        fi
        result+="$c"
    done

    printf '%s' "${result}"
}

# Combines `unquote` and `abs-path`.
function unquote-abs {
    abs-path "$(unquote "$1")"
}


#
# Exported functions: Rule constructors
#

# Adds to `PREFIX`, `OPTS`, and `ARGS` based on the given raw arguments.
function parse-rule-args {
    while [[ $1 != '' ]]; do
        local opt="$1"
        if [[ ${opt} == '--' ]]; then
            shift
            break
        elif [[ ${opt} =~ ^--id=(.*) ]]; then
            PREFIX+=("id $(quote "${BASH_REMATCH[1]}")")
        elif [[ ${opt} =~ ^--req=(.*) ]]; then
            PREFIX+=("req $(quote "${BASH_REMATCH[1]}")")
        elif [[ ${opt} =~ ^--target=(.*) ]]; then
            PREFIX+=("target $(quote "${BASH_REMATCH[1]}")")
        elif [[ ${opt} =~ ^--assert=(.*) ]]; then
            PREFIX+=("assert ${BASH_REMATCH[1]}")
        elif [[ ${opt} =~ ^--cmd=(.*) ]]; then
            PREFIX+=("cmd ${BASH_REMATCH[1]}")
        elif [[ ${opt} =~ ^--moot=(.*) ]]; then
            PREFIX+=("moot ${BASH_REMATCH[1]}")
        elif [[ ${opt} =~ ^--msg=(.*) ]]; then
            PREFIX+=("msg ${BASH_REMATCH[1]}")
        elif [[ ${opt} =~ ^- ]]; then
            OPTS+=("${opt}")
        else
            break
        fi
        shift
    done

    ARGS+=("$@")
}

# Emits a rule with the current `PREFIX` and any additional lines as given.
function emit-rule {
    # Note: Let `PREFIX` be inherited but become local.
    local OPTS=() ARGS=() PREFIX=("${PREFIX[@]}")
    parse-rule-args "$@"

    if [[ ${#OPTS[@]} != 0 ]]; then
        echo "Unknown option: ${OPTS[0]}" 1>&2
        return 1
    fi

    if [[ ${#ARGS[@]} != 0 ]]; then
        echo "Invalid argument: ${ARGS[0]}" 1>&2
        return 1
    fi

    echo 'start'

    if [[ ${#PREFIX[@]} != 0 ]]; then
        printf '  %s\n' "${PREFIX[@]}"
    fi

    echo 'end'
}

# Implementation for `body` rule (arbitrary body lines).
function rule-body-body {
    if [[ ${#OPTS[@]} != 0 ]]; then
        echo "Unknown option: ${OPTS[0]}" 1>&2
        return 1
    fi

    emit-rule "${ARGS[@]}"
}

# Implementation for `copy` rule.
function rule-body-copy {
    local opt name fromDir toDir mode

    for opt in "${OPTS[@]}"; do
        if [[ ${opt} =~ ^--from-dir=(.*) ]]; then
            fromDir="${BASH_REMATCH[1]}"
        elif [[ ${opt} =~ ^--to-dir=(.*) ]]; then
            toDir="${BASH_REMATCH[1]}"
        elif [[ ${opt} =~ ^--chmod=(.*) ]]; then
            mode="${BASH_REMATCH[1]}"
        else
            echo "Unknown option: ${opt}" 1>&2
            return 1
        fi
    done

    if [[ ${fromDir} == '' ]]; then
        echo 'Missing option: --from-dir' 1>&2
        return 1
    elif [[ ${toDir} == '' ]]; then
        echo 'Missing option: --to-dir' 1>&2
        return 1
    fi

    # Make the directories absolute, if not already.
    fromDir="$(abs-path "${fromDir}")"
    toDir="$(abs-path "${toDir}")"

    for name in "${ARGS[@]}"; do
        if [[ ${name} =~ /$ ]]; then
            echo "Invalid file name (trailing slash): ${name}" 1>&2
            return 1
        elif [[ ${name} =~ ^/ ]]; then
            echo "Invalid file name (leading slash): ${name}" 1>&2
            return 1
        fi

        local targetFile="$(abs-path ${toDir}/${name})"
        local sourceFile="$(abs-path ${fromDir}/${name})"
        local targetDir="${targetFile%/*}"
        local chmodCmd=()

        if [[ ${mode} != '' ]]; then
            chmodCmd=(
                --cmd="chmod $(quote "${mode}" "${targetFile}")"
            )
        fi

        rule mkdir -- "${targetDir}"

        emit-rule \
            --target="${targetFile}" \
            --req="${targetDir}" \
            --req="${sourceFile}" \
            --msg="Copy: ${sourceFile}" \
            --cmd="cp $(quote "${sourceFile}" "${targetFile}")" \
            "${chmodCmd[@]}"
    done
}

# Implementation for `mkdir` rule. Ensures a given directory only ever has one
# rule emitted for it.
MKDIRS=()
function rule-body-mkdir {
    local name i

    if [[ ${#OPTS[@]} != 0 ]]; then
        echo "Unknown option: ${OPTS[0]}" 1>&2
        return 1
    fi

    for name in "${ARGS[@]}"; do
        name="$(abs-path "${name}")"
        if [[ ${name} =~ /$ ]]; then
            # Remove trailing slash.
            name="${name%/}"
        fi

        local already=0
        for (( i = 0; i < ${#MKDIRS[@]}; i++ )); do
            if [[ ${name} == ${MKDIRS[$i]} ]]; then
                already=1
                break
            fi
        done

        if (( already )); then
            continue
        fi

        MKDIRS+=("${name}")

        emit-rule \
            --target="${name}" \
            --moot="[[ -d $(quote "${name}") ]]" \
            --assert="[[ ! -e $(quote "${name}") ]]" \
            --cmd="mkdir -p $(quote "${name}")"
    done
}

# Implementation for `rm` rule.
function rule-body-rm {
    local opt name inDir='.'

    for opt in "${OPTS[@]}"; do
        if [[ ${opt} =~ ^--in-dir=(.*) ]]; then
            inDir="${BASH_REMATCH[1]}"
        else
            echo "Unknown option: ${opt}" 1>&2
            return 1
        fi
    done

    for name in "${ARGS[@]}"; do
        name="$(abs-path --in-dir="${inDir}" "${name}")"
        emit-rule \
            --moot="[[ ! -e $(quote "${name}") ]]" \
            --msg="Rm: ${name}" \
            --cmd="rm -rf $(quote "${name}")"
    done
}

# Emits a rule (or set of rules) of the indicated type, with given additional
# arguments. Every type accepts any number of `--id=`, `--req=`, `--target`,
# `--assert=`, `--cmd=`, `--moot`, and `--msg=` options. Beyond that,
# arguments are type-specific.
function rule {
    local type="$1"
    shift

    # These are used / modified by per-type rule constructors.
    local PREFIX=() OPTS=() ARGS=()

    parse-rule-args "$@"
    eval "rule-body-${type}"
}


#
# Directory setup. This has to be done after `abs-path` is defined.
#

BLUR_DIR="$(abs-path "${BASH_SOURCE[0]%/*}")"
