#!/usr/bin/env python3
import struct

floats = [0.0, 1.0, -1.0, 0.5, 2.0, 3.14159, 1e10, 1e-10, float('inf'), float('nan')]

print(f"{'Float32':<15} {'Hex':>12} {'Binary'}")
print("-" * 50)

for f in floats:
    packed = struct.pack('<f', f)
    hex_val = packed.hex()
    bits = int.from_bytes(packed, 'little')
    binary = f"{bits:032b}"
    sign = binary[0]
    exp = binary[1:9]
    mantissa = binary[9:]
    print(f"{f:<15} 0x{hex_val:<10} {sign} {exp} {mantissa}")
