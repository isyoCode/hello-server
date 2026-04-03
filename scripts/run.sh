#!/bin/bash

# YoyoCppServer run script
# Convenient wrapper to start the server

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EXECUTABLE="$PROJECT_ROOT/build/yoyo-server"

# Check if executable exists
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ Error: Server executable not found at $EXECUTABLE"
    echo ""
    echo "Please build the project first:"
    echo "  bash scripts/build.sh"
    exit 1
fi

# Check if library exists
SQLPOOL_LIB="$PROJECT_ROOT/lib/vendor/sqlpool/v1.0/libmysqlpool.so"
if [ ! -f "$SQLPOOL_LIB" ]; then
    echo "⚠️  Warning: SqlPool library not found at $SQLPOOL_LIB"
    echo "The server might fail to start."
    echo ""
fi

echo "================================"
echo "YoyoCppServer"
echo "================================"
echo ""
echo "Starting server..."
echo "Press Ctrl+C to stop"
echo ""

# Run the server
exec "$EXECUTABLE" "$@"
