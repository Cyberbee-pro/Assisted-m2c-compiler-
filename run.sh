#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BINARY="${ROOT_DIR}/m2c_bin"

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
    echo "Usage: $0 <input.cym2c> [output_binary_name]"
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_BINARY="${2:-m2c_program}"

"${BINARY}" "${INPUT_FILE}" -o "${OUTPUT_BINARY}"
