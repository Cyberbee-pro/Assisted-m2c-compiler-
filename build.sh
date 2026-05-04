#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

g++ -std=c++17 \
    "${ROOT_DIR}/main.cpp" \
    "${ROOT_DIR}/lexer/source/"*.cpp \
    "${ROOT_DIR}/syntaxer/source/"*.cpp \
    "${ROOT_DIR}/generators/source/"*.cpp \
    -I "${ROOT_DIR}/lexer/include" \
    -I "${ROOT_DIR}/syntaxer/include" \
    -I "${ROOT_DIR}/generators/include" \
    -Wall -Wextra -pedantic \
    -o "${ROOT_DIR}/m2c_bin"

echo "Build Success: ${ROOT_DIR}/m2c_bin"
