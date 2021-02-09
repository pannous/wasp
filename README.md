# 🐝 Wasp = Wasm-Lisp

 **Wasp** is a new unified notation for both markup/object data and code.  
 Wasp is the foundation layer of the higher order programming language [angle](https://github.com/pannous/angle).
 
«Data is Code and Code is Data»

## Wasp Syntax

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

# Angle Language

[Angle](https://github.com/pannous/angle) is a new Programming Language using Wasp as data format, "Lisp with Hashes"

In Lisp to create an object like foo:{bar:3} you have to resort to ugly reader macros like `#S(foo :bar 3)` 

Wasp is a modern programming language and 'post-lisp' data format, fixing the shortcomings of JSON (no comments, verbose quoted keys…) , ECMA (breaking JS semantics) and [Lisp](https://github.com/google/schism) (maps second order citizens).  

The big difference to Lisp is that everything is map based, lists are flat maps like in JS : ['a','b'] == {0:'a' 1:'b'}.

This gives an universal exceptionless object data type, just like lists in lisp.

What do Lisp, ECMA and Wasm have in common anyway?
Unbeknownst to many, thanks to closures and ECMA Objects, modern JavaScript is getting ever closser to the original idea of Lisp:

The Wasp syntax was influenced by [Mark](https://github.com/henry-luo/mark)

 **Wasp**, is a new unified notation for both object and markup data. The notation is a superset of what can be represented by JSON, HTML and XML, but overcomes many limitations these popular data formats, yet still having a very clean syntax and simple data model.

- It has **clean syntax** with **fully-type** data model *(like JSON or even better)*
- It is **generic** and **extensible** *(like XML or even better)*
- It has built-in **mixed content** support *(like HTML5 or even better)*
- It supports **high-order** composition *(like S-expressions or even better)*

|                        | Wasp/[Mark](https://github.com/henry-luo/mark)                           | JSON     | HTML | XML                            | S-expr                             | YAML                                  |
| ---------------------- | ------------------------------ | -------- | ---- | ------------------------------ | ---------------------------------- | ------------------------------------- |
| Clean syntax           | yes | yes| no   | yes | yes| yes|
| Fully-typed            | yes | yes| no   | no| yes| yes |
| Generic                | yes | yes| no   | yes | yes| yes |
| Mixed content support  | yes | hard     | yes | yes | hard | hard                                  |
| High-order composition | yes | possible | no   | yes | yes| possible                              |
| Wide adoption          | not yet | yes| yes | yes | limited                            | limited                               |


## Wasp Data Model

Wasp has a very simple and fully-typed data model. 
## [Documentation](../wiki/)

- [Syntax specification](../wiki/syntax)
- [Data model and API specification](../wiki/data)
- [FAQ](../wiki/FAQ)
- [Examples](../wiki/Examples)

## Differences to original Mark:
objects are created left of the bracket, so
`{html {body}}` in Mark is `html{body}` in Wasp / Angle.

Assignment and object creation
```
foo{bar:3}  #object creation
foo:{bar:3} #property assignment (nested objects)
foo={bar:3} #variable assignment (of calling context)
```

Difference between Wasp and [Mark Notation](https://github.com/henry-luo/mark) :
Wasp  has the object/node type on the left side of braces:
Wasp: `html{ div{'hello world'}}` vs
Mark: `{html {div 'hello world'}}`

The later 'polish-notation' (or sexp/lisp-notation with braces) can be used via compile flag (#pragma?).

div{ class:"form-group" } vs div(class="form-group") vs div{ class:{"form-group"}} ?
Should be [identical](identities) under most circumstances

[[Todos]]
* replace wasm-micro-runtime: The codebase is cancer
