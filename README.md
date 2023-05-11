# 𓆤 Wasp : Wasm Programming Language

 **[Wasp](https://github.com/pannous/wasp/wiki)** is a new unified notation for both markup/object data and code.  
 Wasp is the foundation layer of the higher order programming language [angle](https://github.com/pannous/angle).
 
«Data is Code and Code is Data»

## [Wasp Syntax](https://github.com/pannous/wasp/wiki/syntax)

For example, a HTML registration form:

```html
<form>
  <!--comment-->
  <div class="form-group">
    <label for="email">Email address:</label>
    <input type="email" id="email">
  </div>
  <button class='btn btn-info'>Submit</button>
</form>
```
Can be represented in Wasp as:
```text
form{                                 
  //comment                          
  div{ class:"form-group"             
    label{ for:email                  
      "Email address:"                
    }
    input{ type:email id:email}     
  }
  button{ class:['btn' 'btn-info']  
    'Submit'                        
  }
}
```

The wasp runtime is available as
* native **binary** for Mac, Linux and Windows
* small standalone **webassembly** file (~50kb), with or without  
* a wasm **compiler** (~100kb) in wasm:

# Angle Language

[Angle](https://github.com/pannous/wasp/wiki/angle) is a new Programming Language using Wasp as data format,  
"Lisp with Hashes" and optional braces.

Hello World in Angle is  
`"Hello World"`  

The deep formal reason why this is a valid program is that the last object in a block is its return value  
and the last result in the root block of the main file is printed.

Fibonacci in Angle is
```
fibonacci number = if number<2 : 1 else fibonacci(number - 1) + fibonacci it - 2
```

Note how number simulataneously acts as type and variable name.  
Note how the unique argument `number` can be accessed via `it` keyword and some brackets are optional

[Auto typed](https://github.com/pannous/wasp/wiki/type-inference) fibonacci in Angle is

```
fib := if it<2 : 1 else fib(it-1) + fib it - 2
```

Angle runs as wasm file inside browsers an as small lambdas in edge computing.
Angle programms **compiled to wasm**, which dont make use of the standard api can be extremely small <1kb, just like
handwritten [wast](https://www.richinfante.com/2020/01/03/webassembly-examples) but with much more comfort.

## built-in linker

Like in swift, all files within the source file's directory are parsed and available without extra `use` / `include`
keyword. However other projects and source files can be included dynamically or statically aot at compile time.

## built-in component model

The webassembly [component model](https://github.com/WebAssembly/component-model) is a first class citizen of
angle: `wit` files and syntax can be included directly:

```
flags virtues{ 
  fast
  safe
}

virtues wasp_goal = fast + safe
```

No need for any bindgen, unless interfacing with components of other languages.
In fact we hop to promote wit to a universal header file format.

Wasp and Angle are **free of dependencies** (other than gcc, if the runtime is to be compiled from scratch).
Only some optional features can make use of external dependencies:

The natives runtimes can be configured to ship with a JIT wasm runtime (wasmtime, wasmedge, wasm3, wasmer,
wasm-micro-runtime and V8!)

The **native** Wasp/Angle binary contains a small **WebView** connector making use of the host's browser component (Edge
or WebKit).
Since the Angle language can be compiled from wasm to wasm, this gives a whole self sufficient programming environment
in less than 200kb,  
with similar power to [electron](https://www.electronjs.org/) and QT.

Note: The full wasp_compiler.wasm is currently 2MB but can be shrunk back close to the 70kb of wasp_runtime.wasm once
the lazy external references are removed again.

Until a smart way is found to write directly to the WebViews canvas, native angle ships with a low
overhead **[graphics](https://github.com/pannous/wasp/wiki/graphics)** adapter (using SDL) for fast fullscreen painting
in native wasp bundles.

## [Documentation](https://github.com/pannous/wasp/wiki/)

- [Syntax specification](https://github.com/pannous/wasp/wiki/syntax)
- [Features](https://github.com/pannous/wasp/wiki/features)
- [Inventions](https://github.com/pannous/wasp/wiki/Inventions)
- [Examples](https://github.com/pannous/wasp/wiki/Examples)

## Wasp Data Model


**Wasp**, is a new unified notation for both object and markup data. The notation is a superset of what can be represented by JSON, HTML and XML, but overcomes many limitations these popular data formats, yet still having a very clean syntax and simple data model.

- It has **clean syntax** with **fully-type** data model *(like JSON or even better)*
- It is **generic** and **extensible** *(like XML or even better)*
- It has built-in **mixed content** support *(like HTML5 or even better)*
- It supports **high-order** composition *(like S-expression or even better)*

|                        | [Wasp](https://github.com/pannous/wasp/wiki)/[Mark](https://github.com/henry-luo/wasp) | JSON     | HTML | XML | S-expr  | YAML     |
|------------------------|----------------------------------------------------------------------------------------|----------|------|-----|---------|----------|
| Clean syntax           | yes                                                                                    | yes      | no   | yes | yes     | yes      |
| Fully-typed            | yes                                                                                    | yes      | no   | no  | yes     | yes      |
| Generic                | yes                                                                                    | yes      | no   | yes | yes     | yes      |
| Mixed content support  | yes                                                                                    | hard     | yes  | yes | hard    | hard     |
| High-order composition | yes                                                                                    | possible | no   | yes | yes     | possible |
| Wide adoption          | not yet                                                                                | yes      | yes  | yes | limited | limited  |


Wasp has a simple and fully-typed data model. 
## [Documentation](https://github.com/pannous/wasp/wiki/)

- [Data model](https://github.com/pannous/wasp/wiki/data)
- [FAQ](https://github.com/pannous/wasp/wiki/FAQ)
- [Examples](https://github.com/pannous/wasp/wiki/Examples)



