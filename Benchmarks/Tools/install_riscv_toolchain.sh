#!/usr/bin/env bash
# Installs Raspberry Pi's prebuilt riscv32-unknown-elf-gcc toolchain (matches
# the vendored pico-sdk 2.1.1 exactly, including Hazard3's Zcb/Zcmp support)
# to ~/.pico-sdk/toolchain/15 - a local, non-sudo install, nothing added to
# git. build.sh calls this automatically if the toolchain is missing.
set -euo pipefail

DEST="${RISCV_TOOLCHAIN_BIN:-$HOME/.pico-sdk/toolchain/15}"
DEST="${DEST%/bin}"  # allow being passed either the toolchain root or its bin/

if [ -x "$DEST/bin/riscv32-unknown-elf-gcc" ]; then
    echo "RISC-V toolchain already installed at $DEST/bin"
    exit 0
fi

case "$(uname -s)" in
    Linux)  os=lin ;;
    Darwin) os=mac ;;
    *) echo "error: unsupported OS $(uname -s) - see https://github.com/raspberrypi/pico-sdk-tools/releases/tag/v2.1.1-3 for a manual download" >&2; exit 1 ;;
esac

case "$(uname -m)" in
    x86_64|amd64) arch=x86_64 ;;
    arm64|aarch64) arch=$([ "$os" = mac ] && echo arm64 || echo aarch64) ;;
    *) echo "error: unsupported arch $(uname -m)" >&2; exit 1 ;;
esac

asset="riscv-toolchain-15-${arch}-${os}.tar.gz"
url="https://github.com/raspberrypi/pico-sdk-tools/releases/download/v2.1.1-3/${asset}"

echo "Installing RISC-V toolchain to $DEST"
echo "  <- $url"
mkdir -p "$DEST"
tmp="$(mktemp)"
trap 'rm -f "$tmp"' EXIT
curl -fL -o "$tmp" "$url"
tar -xzf "$tmp" -C "$DEST"

if [ ! -x "$DEST/bin/riscv32-unknown-elf-gcc" ]; then
    echo "error: extracted archive but riscv32-unknown-elf-gcc still not found under $DEST/bin" >&2
    exit 1
fi

echo "Installed: $("$DEST/bin/riscv32-unknown-elf-gcc" --version | head -1)"
