#!/bin/bash

db_dir="`dirname "$0"`"

device_type="$1"

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

cat "${db_dir}/evg-common.inc.req"

if [ "${form_factor}" = "vme" ]; then
  cat "${db_dir}/evg-vme.inc.req"
fi
