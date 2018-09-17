decimal_to_hex_digit() {
  case "$1" in
    10)
      echo -n "a"
      ;;
    11)
      echo -n "b"
      ;;
    12)
      echo -n "c"
      ;;
    13)
      echo -n "d"
      ;;
    14)
      echo -n "e"
      ;;
    15)
      echo -n "f"
      ;;
    *)
      echo -n "$1"
      ;;
  esac
}

hex_digit_to_decimal() {
  case "$1" in
    [Aa])
      echo -n "10"
      ;;
    [Bb])
      echo -n "11"
      ;;
    [Cc])
      echo -n "12"
      ;;
    [Dd])
      echo -n "13"
      ;;
    [Ee])
      echo -n "14"
      ;;
    [Ff])
      echo -n "15"
      ;;
    *)
      echo -n "$1"
      ;;
  esac
}

decimal_to_hex() {
  local decimal_number="$1"
  local min_digits=0
  if [ $# -ge 2 ]; then
    min_digits="$2"
  fi
  local hex_number=""
  while [ "${decimal_number}" -gt 0 ]; do
    local remainder=$(( ${decimal_number} % 16 ))
    decimal_number=$(( ${decimal_number} / 16 ))
    hex_number=$( decimal_to_hex_digit ${remainder} )${hex_number}
  done
  while [ ${#hex_number} -lt "${min_digits}" ]; do
    hex_number=0${hex_number}
  done
  echo "0x${hex_number}"
}

hex_to_decimal() {
  local hex_number="$1"
  local min_digits=0
  if [ $# -ge 2 ]; then
    min_digits="$2"
  fi
  local decimal_number=0
  hex_number="${hex_number#0[Xx]}"
  while [ -n "${hex_number}" ]; do
    local first_hex_digit=${hex_number:0:1}
    hex_number=${hex_number:1}
    decimal_number=$(( 16 * ${decimal_number} + $( hex_digit_to_decimal ${first_hex_digit} ) ))
  done
  while [ ${#decimal_number} -lt "${min_digits}" ]; do
    decimal_number=0${decimal_number}
  done
  echo "${decimal_number}"
}

substitute_template_variables() {
  local done=false
  # Splitting the substitution strings with cut takes a considerable amount of
  # time. For this reason, we do it outside the loop where we iterate over all
  # lines of input.
  local -a substitution_names=()
  local -a substitution_values=()
  local substitution
  for substitution in "$@"; do
    substitution_names+=( "$( echo "${substitution}" | cut -d = -f 1 )" )
    substitution_values+=( "$( echo "${substitution}" | cut -d = -f 2- )" )
  done
  while [ ${done} = false ] ; do
    local line
    IFS="" read -r line || done=true
    local i
    for i in ${!substitution_names[@]}; do
      local name="${substitution_names[${i}]}"
      local value="${substitution_values[${i}]}"
      line="${line//@${name}@/${value}}"
    done
    echo -n "${line}"
    if [ ${done} = false ]; then
      echo
    fi
  done
}

fanout() {
  local pv_name="$1"
  local fout_num=0
  local target_pv_index=1
  local -a write_all_pvs
  shift
  write_all_pvs=("$@")
  cat <<EOF
record(fanout, "${pv_name}") {
EOF
  for target_pv_name in "${write_all_pvs[@]}"; do
    if [ ${target_pv_index} -eq 7 ]; then
      fout_num=$(( ${fout_num} + 1 ))
      cat <<EOF
  field(FLNK, "${pv_name}:Fout${fout_num}")
}

record(fanout, "${pv_name}:Fout${fout_num}") {
EOF
      target_pv_index=1
    fi
    cat <<EOF
  field(LNK${target_pv_index}, "${target_pv_name}")
EOF
    target_pv_index=$(( ${target_pv_index} + 1 ))
  done
  cat <<EOF
}

EOF
}
