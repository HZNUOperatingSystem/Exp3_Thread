#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ONNX_BASE="${SCRIPT_DIR}/../third_party/onnxruntime"

uname_s=$(uname -s)

if [[ "$uname_s" == "Darwin" ]]; then
    PLATFORM="macos-arm64"
    LDFLAGS="-L${ONNX_BASE}/${PLATFORM}/lib -lonnxruntime.1.24.4"
    LDFLAGS="${LDFLAGS} -Wl,-rpath,${ONNX_BASE}/${PLATFORM}/lib"
else
    PLATFORM="linux-x64"
    LDFLAGS="-L${ONNX_BASE}/${PLATFORM}/lib -lonnxruntime"
    LDFLAGS="${LDFLAGS} -Wl,-rpath,${ONNX_BASE}/${PLATFORM}/lib"
fi

INCLUDES="-I${ONNX_BASE}/${PLATFORM}/include"

# Output based on requested format
case "${1:-flags}" in
    cflags)
        echo "${INCLUDES}"
        ;;
    ldflags)
        echo "${LDFLAGS}"
        ;;
    flags)
        echo "${INCLUDES} ${LDFLAGS}"
        ;;
    platform)
        echo "${PLATFORM}"
        ;;
    libpath)
        echo "${ONNX_BASE}/${PLATFORM}/lib"
        ;;
    *)
        echo "Usage: $0 [cflags|ldflags|flags|platform|libpath]" >&2
        exit 1
        ;;
esac
