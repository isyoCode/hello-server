#!/bin/bash

# ============================================================================
# run_all_tests.sh - 运行所有单元测试和性能测试
# ============================================================================
# 使用方法: bash run_all_tests.sh
# ============================================================================

set -e

TESTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$TESTS_DIR")"

echo "════════════════════════════════════════════════════════════"
echo "       YoyoCppServer 单元测试和性能测试套件"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "项目路径: $PROJECT_ROOT"
echo "测试目录: $TESTS_DIR"
echo ""

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 测试计数
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# ============================================================================
# 函数: 编译并运行测试
# ============================================================================

run_test() {
    local test_name=$1
    local test_dir=$2
    local test_file=$3
    local compile_flags=$4

    echo ""
    echo "────────────────────────────────────────────────────────────"
    echo "📝 运行测试: $test_name"
    echo "────────────────────────────────────────────────────────────"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    cd "$test_dir" || exit 1

    # 获取可执行文件名（去掉 .cc/.cpp 后缀）
    local exe_name="${test_file%.*}"

    # 编译
    echo "编译中: $test_file"
    if g++ -std=c++20 "$test_file" -o "$exe_name" $compile_flags 2>&1 | head -20; then
        echo -e "${GREEN}✓ 编译成功${NC}"
    else
        echo -e "${RED}✗ 编译失败${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi

    # 运行
    echo ""
    echo "执行中: ./$exe_name"
    if ./"$exe_name"; then
        echo ""
        echo -e "${GREEN}✓ $test_name 通过${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        echo ""
        echo -e "${RED}✗ $test_name 失败${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
}

# ============================================================================
# 主程序
# ============================================================================

echo "开始运行测试..."
echo ""

# 测试 1: JSON 解析器
run_test "JSON 解析器测试" \
    "$TESTS_DIR/json" \
    "json_parser_test.cc" \
    "-I../../include"

# 测试 2: HTTP 解析器
# 注意: HTTP 解析器测试需要链接源代码
cd "$TESTS_DIR/http" && \
g++ -std=c++20 http_parser_test.cc ../../src/http/request_parser.cc ../../src/http/response_parser.cc \
    -o http_parser_test -I../../src -I../../include 2>&1 | head -10
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ HTTP 测试编译成功${NC}"
    ./http_parser_test
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ HTTP 解析器测试 通过${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}✗ HTTP 解析器测试 失败${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
else
    echo -e "${RED}✗ HTTP 测试编译失败${NC}"
    FAILED_TESTS=$((FAILED_TESTS + 1))
fi
TOTAL_TESTS=$((TOTAL_TESTS + 1))

# 测试 3: 线程池
run_test "线程池性能测试" \
    "$TESTS_DIR/threadpool" \
    "threadpool_test.cc" \
    "-I../../src -pthread"

# ============================================================================
# 测试总结
# ============================================================================

echo ""
echo "════════════════════════════════════════════════════════════"
echo "                     测试总结"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "总计运行: $TOTAL_TESTS 个测试"
echo -e "通过: ${GREEN}$PASSED_TESTS${NC}"
echo -e "失败: ${RED}$FAILED_TESTS${NC}"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo "════════════════════════════════════════════════════════════"
    echo -e "           ${GREEN}✓ 所有测试通过！${NC}"
    echo "════════════════════════════════════════════════════════════"
    exit 0
else
    echo "════════════════════════════════════════════════════════════"
    echo -e "           ${RED}✗ 有测试失败${NC}"
    echo "════════════════════════════════════════════════════════════"
    exit 1
fi
