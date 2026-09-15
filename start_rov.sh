#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
INTERFACE_DIR="$ROOT_DIR/nautilus_interface"
MEDIAMTX_DIR="${NAUTILUS_MEDIAMTX_DIR:-$ROOT_DIR/../mediamtx}"
MEDIAMTX_BIN="$MEDIAMTX_DIR/mediamtx"
MEDIAMTX_CONFIG="${NAUTILUS_MEDIAMTX_CONFIG:-$MEDIAMTX_DIR/mediamtx.yml}"
LOG_DIR="${NAUTILUS_LOG_DIR:-$ROOT_DIR/logs}"
UART_PORT="${NAUTILUS_UART_PORT:-/dev/ttyACM0}"
BAUDRATE="${NAUTILUS_BAUDRATE:-9600}"
HTTP_PORT="${NAUTILUS_HTTP_PORT:-8000}"
WEBSOCKET_PORT="${NAUTILUS_WS_PORT:-8765}"
RAW_VIDEO_PORT="${NAUTILUS_RAW_VIDEO_PORT:-8555}"
RTSP_PORT="${NAUTILUS_RTSP_PORT:-8554}"
STARTUP_TIMEOUT="${NAUTILUS_STARTUP_TIMEOUT:-30}"

child_pids=()
shutting_down=false

log() {
    printf '[%s] %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$*"
}

fail() {
    log "ERROR: $*" >&2
    exit 1
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || fail "required command not found: $1"
}

cleanup() {
    local exit_status=$?
    local pid

    if [[ "$shutting_down" == true ]]; then
        return "$exit_status"
    fi
    shutting_down=true

    log "stopping ROV services"
    for pid in "${child_pids[@]}"; do
        if kill -0 "$pid" 2>/dev/null; then
            kill -TERM "$pid" 2>/dev/null || true
        fi
    done

    for pid in "${child_pids[@]}"; do
        wait "$pid" 2>/dev/null || true
    done

    return "$exit_status"
}

trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

wait_for_port() {
    local host="$1"
    local port="$2"
    local description="$3"
    local deadline=$((SECONDS + STARTUP_TIMEOUT))

    while (( SECONDS < deadline )); do
        if (echo >/dev/tcp/"$host"/"$port") 2>/dev/null; then
            log "$description is ready on $host:$port"
            return 0
        fi
        sleep 1
    done

    fail "timed out waiting for $description on $host:$port"
}

start_service() {
    local name="$1"
    shift
    local log_file="$LOG_DIR/$name.log"

    log "starting $name"
    "$@" >"$log_file" 2>&1 &
    child_pids+=("$!")
    log "$name started (pid ${child_pids[-1]})"
}

require_command python3
require_command rpicam-vid
require_command ffmpeg
[[ -x "$MEDIAMTX_BIN" ]] || fail "MediaMTX executable not found: $MEDIAMTX_BIN"
[[ -f "$MEDIAMTX_CONFIG" ]] || fail "MediaMTX config not found: $MEDIAMTX_CONFIG"
[[ -d "$INTERFACE_DIR/web" ]] || fail "web directory not found: $INTERFACE_DIR/web"

python3 -c 'import serial, websockets' \
    || fail "Python dependencies missing; install pyserial and websockets"

mkdir -p "$LOG_DIR"

pushd "$INTERFACE_DIR" >/dev/null
start_service interface \
    python3 main.py \
    --uart-port "$UART_PORT" \
    --baudrate "$BAUDRATE" \
    --http-port "$HTTP_PORT" \
    --websocket-port "$WEBSOCKET_PORT" \
    --disable-console
popd >/dev/null

start_service camera \
    rpicam-vid \
    -t 0 \
    --bitrate 2000000 \
    --profile baseline \
    --width 1920 \
    --height 1080 \
    --inline \
    --listen \
    -o "tcp://127.0.0.1:$RAW_VIDEO_PORT"

wait_for_port 127.0.0.1 "$RAW_VIDEO_PORT" "camera stream"

start_service mediamtx "$MEDIAMTX_BIN" "$MEDIAMTX_CONFIG"
wait_for_port 127.0.0.1 "$RTSP_PORT" "MediaMTX RTSP server"

start_service ffmpeg \
    ffmpeg \
    -i "tcp://127.0.0.1:$RAW_VIDEO_PORT" \
    -c:v copy \
    -f rtsp \
    -an \
    -rtsp_transport tcp \
    "rtsp://127.0.0.1:$RTSP_PORT/cam"

log "ROV services running: HTTP :$HTTP_PORT, WebSocket :$WEBSOCKET_PORT, RTSP :$RTSP_PORT/cam"

while true; do
    if wait -n "${child_pids[@]}"; then
        fail "a ROV service exited unexpectedly"
    else
        fail "a ROV service exited unexpectedly"
    fi
done
