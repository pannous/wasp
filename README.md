# 🐝 Wasp = Wasm-Lisp

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

# Angle Language

[Angle](https://github.com/pannous/angle) is a new Programming Language using Wasp as data format, "Lisp with Hashes"

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


## Wasp Data Model

Wasp has a very simple and fully-typed data model. 
## [Documentation](https://github.com/pannous/wasp/wiki/)

- [Syntax specification](https://github.com/pannous/wasp/wiki/syntax)
- [Data model and API specification](https://github.com/pannous/wasp/wiki/data)
- [FAQ](https://github.com/pannous/wasp/wiki/FAQ)
- [Examples](https://github.com/pannous/wasp/wiki/Examples)



