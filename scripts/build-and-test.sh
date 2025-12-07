#!/bin/bash
# Quick test runner - builds and runs tests
# Usage: ./scripts/build-and-test.sh [preset]
# Example: ./scripts/build-and-test.sh v0-debug
#          ./scripts/build-and-test.sh v1-debug
#          ./scripts/build-and-test.sh both-debug

set -e

PRESET="${1:-v1-debug}"

echo "🔨 Building (${PRESET})..."
cmake --preset "${PRESET}" > /dev/null 2>&1 || true
cmake --build --preset "${PRESET}"

echo ""
echo "🧪 Running tests..."
ctest --preset "${PRESET}"

echo ""
echo "✅ Done!"
