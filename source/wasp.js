// wasp ABI

class Types {
    // todo : sync with NodeTypes.h
    static Red = 0xFF0000
    static Green = 0x00FF00
    static Blue = 0x0000FF
}

class Node {
    // name=""
    // type=0
    // value
    // children=[]

    // needs int8 buffer to be initialized from linear wasm memory
    constructor(address) {
        this.value = buffer[address + 0]
        this.type = buffer[address + 8]
        this.name = buffer[address + 16]
        this.children = buffer[address + 24] // linked list of Node todo
    }

    constructor(value, type) {
        this.value = value
        this.type = type
        if (type == Types.node) constructor(value)// as address
    }

    unbox() {
        if (type === Types.strings) return this.value
        if (type == Types.longs) return this.value
        if (type == Types.reals) return this.value
        console.error("missing unboxing for Node type ", this.type)
        return this.value
    }
}