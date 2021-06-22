#!/bin/bash

db_dir="`dirname "$0"`"

exec "${db_dir}/generate_evg.sh" "$1" autosave_request_file
