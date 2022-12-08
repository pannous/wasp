#!/usr/local/bin/python3
from tutorial_01 import bindings

calculator = bindings.calculator()
print("2+2 = ", calculator.add(2.0, 2.0))