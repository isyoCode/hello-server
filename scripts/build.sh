#!/bin/bash

# YoyoCppServer build script
# Compiles the C++17 network framework with MySQL and OpenSSL support

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

echo "================================"
echo "YoyoCppServer Build Script"
echo "================================"
echo ""
echo "Project Root: $PROJECT_ROOT"
echo "Build Directory: $BUILD_DIR"
echo ""

# Compiler flags
CFLAGS="-std=c++20 -Wall -Wextra -O2"
INCLUDE_FLAGS="-I$PROJECT_ROOT/src -I$PROJECT_ROOT/include -I/usr/include/mysql"

# Library paths - using local vendor libraries
SQLPOOL_LIB_PATH="$PROJECT_ROOT/lib/vendor/sqlpool/v1.0"
SQLPOOL_LIB="$SQLPOOL_LIB_PATH/libmysqlpool.so"

# Verify SqlPool library exists
if [ ! -f "$SQLPOOL_LIB" ]; then
    echo "❌ Error: SqlPool library not found at $SQLPOOL_LIB"
    echo "Please ensure the library file is placed in the correct location."
    exit 1
fi

LIB_FLAGS="-L$SQLPOOL_LIB_PATH -lmysqlpool -lmysqlclient -lssl -lcrypto"

# Source files
CORE_SOURCES="$PROJECT_ROOT/src/core/Epoll.cc \
               $PROJECT_ROOT/src/core/Eventloop.cc \
               $PROJECT_ROOT/src/core/Socket.cc \
               $PROJECT_ROOT/src/core/TcpConnection.cc \
               $PROJECT_ROOT/src/core/TcpServer.cc \
               $PROJECT_ROOT/src/core/Channel.cc"

HTTP_SOURCES="$PROJECT_ROOT/src/http/request.cc \
              $PROJECT_ROOT/src/http/request_parser.cc \
              $PROJECT_ROOT/src/http/response.cc \
              $PROJECT_ROOT/src/http/response_parser.cc"

ROUTER_SOURCES="$PROJECT_ROOT/src/router/Router.cc"

HANDLER_SOURCES="$PROJECT_ROOT/src/handlers/Handler.cc"

DATABASE_SOURCES="$PROJECT_ROOT/src/database/UserService.cc"

UTILS_SOURCES="$PROJECT_ROOT/src/utils/timer.cc"

THREADPOOL_SOURCES="$PROJECT_ROOT/src/threadpool/threadpool_test.cc"

# Main entry point
MAIN_SOURCE="$PROJECT_ROOT/examples/basic_server.cc"

# Compile command
echo "Compiling..."
g++ $CFLAGS \
    $INCLUDE_FLAGS \
    $CORE_SOURCES \
    $HTTP_SOURCES \
    $ROUTER_SOURCES \
    $HANDLER_SOURCES \
    $DATABASE_SOURCES \
    $UTILS_SOURCES \
    $MAIN_SOURCE \
    $LIB_FLAGS \
    -o "$BUILD_DIR/yoyo-server"

echo ""
echo "================================"
echo "Build completed successfully!"
echo "================================"
echo ""
echo "Executable: $BUILD_DIR/yoyo-server"
echo ""
echo "To run the server:"
echo "  cd $PROJECT_ROOT"
echo "  export LD_LIBRARY_PATH=$PROJECT_ROOT/lib/vendor/sqlpool/v1.0:\$LD_LIBRARY_PATH"
echo "  $BUILD_DIR/yoyo-server"
echo ""
