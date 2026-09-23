#!/bin/bash
# Real-request probe of the /upload guard (waspy/upload_guard.py).
# Runs ON the server against the live gunicorn app:
#   ssh pannous.com 'bash -s' < probes/test_upload_guard.sh
set -u

BASE_URL="${BASE_URL:-http://localhost:8008}"
WASPY_DIR="${WASPY_DIR:-/root/wasp/waspy}"
TOKEN="$(cat "$WASPY_DIR/.upload-token")"
VALID_WASM="$WASPY_DIR/lambdas/test.wasm"
TRAVERSAL_TARGET="/tmp/upload-guard-pwned.wasm"
FAILURES=0

upload_status() { curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/upload" "$@"; }

expect() {
  local label="$1" expected="$2" actual="$3"
  if [ "$actual" = "$expected" ]; then echo "ok   $label ($actual)"; else echo "FAIL $label: expected $expected, got $actual"; FAILURES=$((FAILURES + 1)); fi
}

rm -f "$TRAVERSAL_TARGET" "$WASPY_DIR/lambdas/guardprobe.wasm"

expect "no token" 401 "$(upload_status -F "file=@$VALID_WASM" -F name=guardprobe)"
expect "wrong token" 401 "$(upload_status -H "Authorization: Bearer wrong" -F "file=@$VALID_WASM" -F name=guardprobe)"
expect "traversal filename, no name" 400 "$(upload_status -H "Authorization: Bearer $TOKEN" -F "file=@$VALID_WASM;filename=../../../../tmp/upload-guard-pwned.wasm")"
expect "traversal in name" 400 "$(upload_status -H "Authorization: Bearer $TOKEN" -F "file=@$VALID_WASM" -F "name=../../../tmp/upload-guard-pwned")"
expect "not wasm" 400 "$(upload_status -H "Authorization: Bearer $TOKEN" -F "file=@/etc/hostname" -F name=guardprobe)"
expect "traversal filename, valid name" 302 "$(upload_status -H "Authorization: Bearer $TOKEN" -F "file=@$VALID_WASM;filename=../../../../tmp/upload-guard-pwned.wasm" -F name=guardprobe)"
expect "file stayed inside lambdas/" yes "$([ -f "$WASPY_DIR/lambdas/guardprobe.wasm" ] && [ ! -e "$TRAVERSAL_TARGET" ] && echo yes || echo no)"
expect "token as form field (editor)" 302 "$(upload_status -F "token=$TOKEN" -F "file=@$VALID_WASM" -F name=guardprobe)"

rm -f "$WASPY_DIR/lambdas/guardprobe.wasm"
echo "failures: $FAILURES"
exit "$FAILURES"
