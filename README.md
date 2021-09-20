# 🐝 Wasp : Wasm Programming Language  

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
fibonacci number = if number<2 : 0 else fibonacci(number - 1) + fibonacci it - 2
```

Note how number simulataneously acts as type and variable name.  
Note how the unique argument `number` can be accessed via `it` keyword and brackets are optional

[Auto typed](https://github.com/pannous/wasp/wiki/type-inference) fibonacci in Angle is
```
fib := if it<2 : 0 else fib(it-1) + fib it - 2
```

Angle runs as wasm file inside browsers an as small lambdas in edge computing.
Angle programms which dont make use of the standard api can be extremely small <1kb,  
just like handwritten wast but with much more comfort.

The native Wasp/Angle binary contains a small **WebView** connector making use of the host's browser component (Edge or WebKit).
Since the Angle language can be compiled from wasm to wasm, this gives a whole self sufficient programming environment in less than 200kb,  
with similar power to [electron](https://www.electronjs.org/) and QT. 

Until a smart way is found to write directly to the WebViews canvas, angle ships with a low overhead **[graphics](https://github.com/pannous/wasp/wiki/graphics)** adapter (using SDL) for fast fullscreen painting. 


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

|                        | [Wasp](https://github.com/pannous/wasp/wiki)/[Mark](https://github.com/henry-luo/mark)                           | JSON     | HTML | XML                            | S-expr                             | YAML                                  |
| ---------------------- | ------------------------------ | -------- | ---- | ------------------------------ | ---------------------------------- | ------------------------------------- |
| Clean syntax           | yes | yes| no   | yes | yes| yes|
| Fully-typed            | yes | yes| no   | no| yes| yes |
| Generic                | yes | yes| no   | yes | yes| yes |
| Mixed content support  | yes | hard     | yes | yes | hard | hard                                  |
| High-order composition | yes | possible | no   | yes | yes| possible                              |
| Wide adoption          | not yet | yes| yes | yes | limited                            | limited                               |


Wasp has a simple and fully-typed data model. 
## [Documentation](https://github.com/pannous/wasp/wiki/)

- [Data model](https://github.com/pannous/wasp/wiki/data)
- [FAQ](https://github.com/pannous/wasp/wiki/FAQ)
- [Examples](https://github.com/pannous/wasp/wiki/Examples)



