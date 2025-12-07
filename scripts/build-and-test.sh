#!/bin/bash
# Quick test runner - builds and runs tests
# Usage: ./scripts/test-quick.sh [preset]
# Example: ./scripts/test-quick.sh debug

set -e

PRESET="${1:-debug}"

echo "🔨 Building (${PRESET})..."
cmake --preset "${PRESET}" > /dev/null 2>&1 || true
cmake --build --preset "${PRESET}"

echo ""
echo "🧪 Running tests..."
ctest --preset "${PRESET}"

echo ""
echo "✅ Done!"
