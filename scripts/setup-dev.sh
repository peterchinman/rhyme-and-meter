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

# Initial configuration (v0 debug preset)
echo ""
echo "📋 Configuring v0 debug build..."
if cmake --preset v0-debug; then
    echo "✅ V0 debug preset configured"
else
    echo "❌ Failed to configure v0 debug preset"
    exit 1
fi

# Create symlink to compile_commands.json for IntelliSense
echo ""
echo "📋 Setting up IntelliSense..."
if [ -f "build/v0-debug/compile_commands.json" ]; then
    ln -sf build/v0-debug/compile_commands.json compile_commands.json
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
echo "  Build v0:    cmake --build --preset v0-debug"
echo "  Test v0:     ctest --preset v0-debug"
echo "  Build v1:    cmake --build --preset v1-debug"
echo "  Test v1:     ctest --preset v1-debug"
echo "  Build both:  cmake --build --preset both-debug"
echo ""

