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

sfp_module() {
  local sfp_num="$1"
  local sfp_base_addr="$( hex_to_decimal $2 )"
  local sfp_nominal_bit_rate_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 12 )) 4 )"
  local sfp_vendor_name_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 20 )) 4 )"
  local sfp_vendor_id_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 36 )) 4 )"
  local sfp_vendor_part_number_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 40 )) 4 )"
  local sfp_vendor_part_number_revision_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 56 )) 4 )"
  local sfp_vendor_serial_number_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 68 )) 4 )"
  local sfp_vendor_manufacturing_date_code_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 84 )) 4 )"
  local sfp_temp_h_alarm_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 256 )) 4 )"
  local sfp_temp_l_alarm_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 258 )) 4 )"
  local sfp_temp_h_warning_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 260 )) 4 )"
  local sfp_temp_l_warning_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 262 )) 4 )"
  local sfp_vcc_h_alarm_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 264 )) 4 )"
  local sfp_vcc_l_alarm_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 266 )) 4 )"
  local sfp_vcc_h_warning_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 268 )) 4 )"
  local sfp_vcc_l_warning_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 270 )) 4 )"
  local sfp_tx_bias_h_alarm_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 272 )) 4 )"
  local sfp_tx_bias_l_alarm_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 274 )) 4 )"
  local sfp_tx_bias_h_warning_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 276 )) 4 )"
  local sfp_tx_bias_l_warning_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 278 )) 4 )"
  local sfp_tx_power_h_alarm_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 280 )) 4 )"
  local sfp_tx_power_l_alarm_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 282 )) 4 )"
  local sfp_tx_power_h_warning_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 284 )) 4 )"
  local sfp_tx_power_l_warning_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 286 )) 4 )"
  local sfp_rx_power_h_alarm_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 288 )) 4 )"
  local sfp_rx_power_l_alarm_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 290 )) 4 )"
  local sfp_rx_power_h_warning_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 292 )) 4 )"
  local sfp_rx_power_l_warning_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 294 )) 4 )"
  local sfp_temp_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 352 )) 4 )"
  local sfp_vcc_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 354 )) 4 )"
  local sfp_tx_bias_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 356 )) 4 )"
  local sfp_tx_power_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 358 )) 4 )"
  local sfp_rx_power_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 360 )) 4 )"
  local sfp_status_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 366 )) 4 )"
  local sfp_alarm_flags_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 368 )) 4 )"
  local sfp_warning_flags_addr="$( decimal_to_hex $(( ${sfp_base_addr} + 372 )) 4 )"
  cat "${db_dir}/template-sfp.inc.${extension}" | substitute_template_variables \
    SFP_NUM="${sfp_num}" \
    SFP_NOMINAL_BIT_RATE_ADDR="${sfp_nominal_bit_rate_addr}" \
    SFP_VENDOR_NAME_ADDR="${sfp_vendor_name_addr}" \
    SFP_VENDOR_ID_ADDR="${sfp_vendor_id_addr}" \
    SFP_VENDOR_PART_NUMBER_ADDR="${sfp_vendor_part_number_addr}" \
    SFP_VENDOR_PART_NUMBER_REVISION_ADDR="${sfp_vendor_part_number_revision_addr}" \
    SFP_VENDOR_SERIAL_NUMBER_ADDR="${sfp_vendor_serial_number_addr}" \
    SFP_VENDOR_MANUFACTURING_DATE_CODE_ADDR="${sfp_vendor_manufacturing_date_code_addr}" \
    SFP_TEMP_H_ALARM_ADDR="${sfp_temp_h_alarm_addr}" \
    SFP_TEMP_L_ALARM_ADDR="${sfp_temp_l_alarm_addr}" \
    SFP_TEMP_H_WARNING_ADDR="${sfp_temp_h_warning_addr}" \
    SFP_TEMP_L_WARNING_ADDR="${sfp_temp_l_warning_addr}" \
    SFP_VCC_H_ALARM_ADDR="${sfp_temp_h_alarm_addr}" \
    SFP_VCC_L_ALARM_ADDR="${sfp_temp_l_alarm_addr}" \
    SFP_VCC_H_WARNING_ADDR="${sfp_temp_h_warning_addr}" \
    SFP_VCC_L_WARNING_ADDR="${sfp_temp_l_warning_addr}" \
    SFP_TX_BIAS_H_ALARM_ADDR="${sfp_tx_bias_h_alarm_addr}" \
    SFP_TX_BIAS_L_ALARM_ADDR="${sfp_tx_bias_l_alarm_addr}" \
    SFP_TX_BIAS_H_WARNING_ADDR="${sfp_tx_bias_h_warning_addr}" \
    SFP_TX_BIAS_L_WARNING_ADDR="${sfp_tx_bias_l_warning_addr}" \
    SFP_TX_POWER_H_ALARM_ADDR="${sfp_tx_power_h_alarm_addr}" \
    SFP_TX_POWER_L_ALARM_ADDR="${sfp_tx_power_l_alarm_addr}" \
    SFP_TX_POWER_H_WARNING_ADDR="${sfp_tx_power_h_warning_addr}" \
    SFP_TX_POWER_L_WARNING_ADDR="${sfp_tx_power_l_warning_addr}" \
    SFP_RX_POWER_H_ALARM_ADDR="${sfp_rx_power_h_alarm_addr}" \
    SFP_RX_POWER_L_ALARM_ADDR="${sfp_rx_power_l_alarm_addr}" \
    SFP_RX_POWER_H_WARNING_ADDR="${sfp_rx_power_h_warning_addr}" \
    SFP_RX_POWER_L_WARNING_ADDR="${sfp_rx_power_l_warning_addr}" \
    SFP_TEMP_ADDR="${sfp_temp_addr}" \
    SFP_VCC_ADDR="${sfp_vcc_addr}" \
    SFP_TX_BIAS_ADDR="${sfp_tx_bias_addr}" \
    SFP_TX_POWER_ADDR="${sfp_tx_power_addr}" \
    SFP_RX_POWER_ADDR="${sfp_rx_power_addr}" \
    SFP_STATUS_ADDR="${sfp_status_addr}" \
    SFP_ALARM_FLAGS_ADDR="${sfp_alarm_flags_addr}" \
    SFP_WARNING_FLAGS_ADDR="${sfp_warning_flags_addr}"
}
