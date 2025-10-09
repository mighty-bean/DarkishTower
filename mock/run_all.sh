#!/usr/bin/env bash
set -euo pipefail

# Resolve script directory so this script can be run from anywhere
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
pushd "$SCRIPT_DIR" > /dev/null

echo "Building lightweight mock..."
g++ -std=c++17 mock_main.cpp -o darkish_mock -lpthread
./darkish_mock

echo "Building closer-to-source mock..."
# Add -I. so the local mock/ directory (containing Arduino.h shim) is in the include path
g++ -std=c++17 mock_main_full.cpp -I. -o darkish_mock_full -lpthread
./darkish_mock_full

popd > /dev/null
