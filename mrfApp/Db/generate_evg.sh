#!/bin/bash

db_dir="`dirname "$0"`"

. "${db_dir}/generate_lib.sh"

device_type="$1"
mode="$2"

form_factor=""
series=""

case "${device_type}" in
  vme-evg-230)
    form_factor="vme"
    series="230"
    ;;
  *)
    echo "Unsupported device type: ${device_type}" >&2
    ;;
esac

extension=""
case "${mode}" in
  records)
    extension=db
    ;;
  autosave_request_file)
    extension=req
    ;;
  *)
    echo "Unsupported mode: ${mode}" >&2
    exit 1
    ;;
esac

fp_input() {
  local fp_in_num="$1"
  local fp_in_addr=$( decimal_to_hex $(( $( hex_to_decimal 0x0500 ) + 4 * ${fp_in_num} )) 4 )
  input "FPIn${fp_in_num}" "FP input ${fp_in_num}" ${fp_in_addr}
}

fp_output() {
  local fp_out_num="$1"
  local fp_out_addr=$( decimal_to_hex $(( $( hex_to_decimal 0x0400 ) + 2 * ${fp_out_num} )) 4 )
  output "FPOut${fp_out_num}" "front-panel output ${fp_out_num}" ${fp_out_addr}
}

input() {
  local input_name="$1"
  local input_description="$2"
  local input_addr="$3"
  local input_installed_macro=""
  local input_installed_description=""
  if [ $# -ge 5 ]; then
    input_installed_macro="$4"
    input_installed_description="$5"
  fi
  cat "${db_dir}/evg-template-input.inc.${extension}" \
    | substitute_template_variables \
    INPUT_NAME="${input_name}" \
    INPUT_DESCRIPTION="${input_description}" \
    INPUT_ADDR="${input_addr}"
  if [ -n "${input_installed_macro}" ]; then
    cat "${db_dir}/evg-template-input-installed.inc.${extension}" \
    | substitute_template_variables \
    INPUT_NAME="${input_name}" \
    INPUT_INSTALLED_MACRO="${input_installed_macro}" \
    INPUT_INSTALLED_DESCRIPTION="${input_installed_description}"
  fi
  description "\$(P)\$(R)${input_name}:Description" "Description for ${input_description}"
}

multiplexed_counter() {
  local mxc_num="$1"
  local mxc_addr=$( decimal_to_hex $(( $( hex_to_decimal 0x0180 ) + 8 * ${mxc_num} )) 4 )
  local mxc_prescaler_addr=$( decimal_to_hex $(( $( hex_to_decimal 0x0184 ) + 8 * ${mxc_num} )) 4 )
  cat "${db_dir}/evg-template-mxc.inc.${extension}" \
    | substitute_template_variables \
    MXC_NUM="${mxc_num}" \
    MXC_ADDR="${mxc_addr}" \
    MXC_PRESCALER_ADDR="${mxc_prescaler_addr}"
  description "\$(P)\$(R)MXC${mxc_num}:Description" "Description for MXC ${mxc_num}"
}

output() {
  local output_name="$1"
  local output_description="$2"
  local output_addr="$3"
  local output_installed_macro=""
  local output_installed_description=""
  if [ $# -ge 5 ]; then
    output_installed_macro="$4"
    output_installed_description="$5"
  fi
  cat "${db_dir}/evg-template-output.inc.${extension}" | substitute_template_variables OUTPUT_NAME="${output_name}" OUTPUT_DESCRIPTION="${output_description}" OUTPUT_ADDR="${output_addr}"
  if [ -n "${output_installed_macro}" ]; then
    cat "${db_dir}/evg-template-output-installed.inc.${extension}" | substitute_template_variables OUTPUT_NAME="${output_name}" OUTPUT_INSTALLED_MACRO="${output_installed_macro}" OUTPUT_INSTALLED_DESCRIPTION="${output_installed_description}"
  fi
  description "\$(P)\$(R)${output_name}:Description" "Description for ${output_description}"
}

univ_input() {
  local univ_in_num="$1"
  local univ_in_addr=$( decimal_to_hex $(( $( hex_to_decimal 0x0540 ) + 4 * ${univ_in_num} )) 4 )
  local univ_in_module_first_num=$(( ${univ_in_num} / 2 * 2 ))
  local univ_in_module_second_num=$(( ${univ_in_module_first_num} + 1 ))
  input \
    "UnivIn${univ_in_num}" \
    "univ. input ${univ_in_num}" \
    ${univ_in_addr} \
    "UNIV_IN_${univ_in_module_first_num}_${univ_in_module_second_num}_INSTALLED=\${UNIV_IN_INSTALLED=0}" \
    "Universal input module ${univ_in_module_first_num}/${univ_in_module_second_num}"
}

univ_output() {
  local univ_out_num="$1"
  local univ_out_addr=$( decimal_to_hex $(( $( hex_to_decimal 0x0440 ) + 2 * ${univ_out_num} )) 4 )
  local univ_out_module_first_num=$(( ${univ_out_num} / 2 * 2 ))
  local univ_out_module_second_num=$(( ${univ_out_module_first_num} + 1 ))
  output \
    "UnivOut${univ_out_num}" \
    "universal output ${univ_out_num}" \
    ${univ_out_addr} \
    "UNIV_OUT_${univ_out_module_first_num}_${univ_out_module_second_num}_INSTALLED=\$(UNIV_OUT_INSTALLED=0)" \
    "Universal output module ${univ_out_module_first_num}/${univ_out_module_second_num}"
}

tb_input() {
  local tb_in_num="$1"
  local tb_in_addr=$( decimal_to_hex $(( $( hex_to_decimal 0x0600 ) + 4 * ${tb_in_num} )) 4 )
  local tb_in_module_first_num=$(( ${tb_in_num} / 2 * 2 ))
  local tb_in_module_second_num=$(( ${tb_in_module_first_num} + 1 ))
  input \
    "TBIn${tb_in_num}" \
    "TB u. input ${tb_in_num}" \
    ${tb_in_addr} \
    "TB_UNIV_IN_${tb_in_module_first_num}_${tb_in_module_second_num}_INSTALLED=\${TB_UNIV_IN_INSTALLED=0}" \
    "TB univ. input module ${tb_in_module_first_num}/${tb_in_module_second_num}"
}

declare -a write_all_pvs;

cat "${db_dir}/evg-common.inc.${extension}"
write_all_pvs+=("\$(P)\$(R)Intrnl:WriteAll:Common")

# The record type that is used for descriptions depends on a compile-time
# option, so we cannot include the description records in evg-common.inc.db and
# generate them here instead.
for i in `seq 0 7`; do
  description "\$(P)\$(R)DBus:B${i}:Description" "Description for dist. bus bit ${i}"
done
for i in `seq 0 7`; do
  description "\$(P)\$(R)Event:EvTrig${i}:Description" "Description for event trigger ${i}"
done

for i in `seq 0 7`; do
  multiplexed_counter ${i}
  write_all_pvs+=("\$(P)\$(R)Intrnl:WriteAll:MXC${i}")
done

for i in `seq 0 3`; do
  univ_output ${i}
  write_all_pvs+=("\$(P)\$(R)UnivOut${i}:Map")
done

for i in `seq 0 1`; do
  fp_input ${i}
  write_all_pvs+=("\$(P)\$(R)Intrnl:WriteAll:FPIn${i}")
done

for i in `seq 0 3`; do
  univ_input ${i}
  write_all_pvs+=("\$(P)\$(R)Intrnl:WriteAll:UnivIn${i}")
done

if [ "${form_factor}" = "vme" ]; then
  cat "${db_dir}/evg-vme.inc.${extension}"
  write_all_pvs+=("\$(P)\$(R)Intrnl:WriteAll:VME")
  for i in `seq 0 3`; do
    fp_output ${i}
    write_all_pvs+=("\$(P)\$(R)FPOut${i}:Map")
  done
  for i in `seq 0 15`; do
    tb_input ${i}
    write_all_pvs+=("\$(P)\$(R)Intrnl:WriteAll:TBIn${i}")
  done
fi

# EVGs only have a single SFP module, so we use the empty string for the number.
sfp_module "" "0x1200"

if [ "${mode}" = "records" ]; then
  cat <<EOF
# Write all settings to the hardware.

EOF
  fanout "\$(P)\$(R)Intrnl:WriteAll" "${write_all_pvs[@]}"
  cat <<EOF
record(bo, "\$(P)\$(R)WriteAll") {
  field(DESC, "Send all settings to the hardware")
  field(FLNK, "\$(P)\$(R)Intrnl:WriteAll")
  field(ZNAM, "Write all")
  field(ONAM, "Write all")
}

EOF
fi
