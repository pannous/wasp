#!/bin/bash

BINARY="./cmake-build-debug-sanitize/wasp"
ARCH="arm64"

while read -r line; do
    if [[ $line =~ ([A-Z]+)\ ([^[:space:]]+)\ from\ ([^[:space:]]+) ]]; then
        kind="${BASH_REMATCH[1]}"
        addr1="${BASH_REMATCH[2]}"
        addr2="${BASH_REMATCH[3]}"

        # Resolve both addresses
        symbol1=$(atos -o "$BINARY" -arch "$ARCH" "$addr1")
        symbol2=$(atos -o "$BINARY" -arch "$ARCH" "$addr2")

        echo "$kind $addr1 ($symbol1) from $addr2 ($symbol2)"
    else
        echo "$line"
    fi
done < trace.txt