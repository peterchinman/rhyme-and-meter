#!/bin/bash
# Development setup script
# Sets up pre-commit hooks and checks dependencies

set -e

echo "🚀 Setting up development environment..."
echo ""

# Make git hooks executable
echo "📋 Setting up pre-commit hooks..."
chmod +x .git/hooks/pre-commit
echo "✅ Pre-commit hook installed"

# Make scripts executable
echo ""
echo "📋 Making scripts executable..."
chmod +x scripts/*.sh
echo "✅ Scripts are executable"

# Check for cmake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -1)
    echo "✅ ${CMAKE_VERSION}"
else
    echo "❌ cmake is not installed"
    exit 1
fi

# Initial configuration (debug preset)
echo ""
echo "📋 Configuring debug build..."
if cmake --preset debug; then
    echo "✅ Debug preset configured"
else
    echo "❌ Failed to configure debug preset"
    exit 1
fi

# Create symlink to compile_commands.json for IntelliSense
echo ""
echo "📋 Setting up IntelliSense..."
if [ -f "build/debug/compile_commands.json" ]; then
    ln -sf build/debug/compile_commands.json compile_commands.json
    echo "✅ compile_commands.json symlink created"
else
    echo "⚠️  compile_commands.json not found (will be created after first build)"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ Development environment ready!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Quick commands:"
echo "  Build:       cmake --build --preset debug"
echo "  Test:        ctest --preset debug"
echo "  Quick test:  ./scripts/test-quick.sh"
echo ""

