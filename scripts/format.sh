#!/bin/bash

PARENT_DIR="$(readlink -f "$(dirname -- "${BASH_SOURCE[0]}")/..")"

cd ${PARENT_DIR}

python3 scripts/format.py "$@"
