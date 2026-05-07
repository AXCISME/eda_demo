#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
NPROC="$(nproc)"

usage() {
  cat <<EOF
用法: ./build.sh [命令] [选项]

命令:
  native        编译 x86_64 原生版本（默认）
  arm           交叉编译 aarch64 版本
  all           同时编译 native 和 arm
  clean         删除所有 build 目录

选项:
  -j N          指定并行编译任务数（默认 ${NPROC}）
  -h, --help    显示此帮助信息

示例:
  ./build.sh                      # 等同于 ./build.sh native
  ./build.sh arm                  # 交叉编译 aarch64
  ./build.sh arm -j 8             # 指定 8 线程
  ./build.sh all
  ./build.sh clean
EOF
  exit 0
}

build_native() {
  echo "=== 编译 x86_64 ==="
  cmake -B "${SCRIPT_DIR}/build" -S "${SCRIPT_DIR}"
  cmake --build "${SCRIPT_DIR}/build" --parallel "${NPROC}"
}

build_arm() {
  echo "=== 编译 aarch64 ==="
  cmake -B "${SCRIPT_DIR}/build_arm" -S "${SCRIPT_DIR}" \
    -DCMAKE_TOOLCHAIN_FILE="${SCRIPT_DIR}/toolchain-aarch64.cmake"
  cmake --build "${SCRIPT_DIR}/build_arm" --parallel "${NPROC}"
}

clean() {
  echo "=== 清理 build 目录 ==="
  rm -rf "${SCRIPT_DIR}/build" "${SCRIPT_DIR}/build_arm"
  echo "已删除 build/ build_arm/"
}

# 默认命令
CMD="native"

while [[ $# -gt 0 ]]; do
  case "$1" in
    native|arm|all|clean)
      CMD="$1"
      shift
      ;;
    -j)
      NPROC="$2"
      shift 2
      ;;
    -h|--help)
      usage
      ;;
    *)
      echo "未知参数: $1"
      usage
      ;;
  esac
done

case "${CMD}" in
  native)  build_native ;;
  arm)     build_arm ;;
  all)     build_native && build_arm ;;
  clean)   clean ;;
esac
