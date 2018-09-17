#!/bin/bash

db_dir="`dirname "$0"`"

db_dir="`dirname "$0"`"

exec "${db_dir}/generate_evr.sh" "$1" records
