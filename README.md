# 🐝 Wasp = Wasm-Lisp

Wasp is a modern programming language and 'post-lisp' data format, fixing the shortcomings of JSON, ECMA and [Lisp](https://github.com/google/schism).

The big difference to Lisp is that everything is map based, lists are flat maps like in JS : ['a','b'] == {0:'a' 1:'b'}.

This gives an universal exceptionless object data type, just like lists in lisp.

What do Lisp, ECMA and Wasm have in common anyway?
Unbeknownst to many, thanks to closures and ECMA Objects, modern JavaScript is getting ever closser to the original idea of Lisp:

«Data is Code and Code is Data»

# Angle Language

Angle is a new Programming Language as well as data format, "Lisp with Hashes"

In Lisp to create an object like foo:{bar:3} you have to resort to ugly reader macros like `#S(foo :bar 3)` 

 **Wasp**, is a new unified notation for both object and markup data. The notation is a superset of what can be represented by JSON, HTML and XML, but overcomes many limitations these popular data formats, yet still having a very clean syntax and simple data model.

- It has **clean syntax** with **fully-type** data model *(like JSON or even better)*
- It is **generic** and **extensible** *(like XML or even better)*
- It has built-in **mixed content** support *(like HTML5 or even better)*
- It supports **high-order** composition *(like S-expressions or even better)*

|                        | Wasp/[[Mark](https://github.com/henry-luo/mark)                           | JSON     | HTML | XML                            | S-expr                             | YAML                                  |
| ---------------------- | ------------------------------ | -------- | ---- | ------------------------------ | ---------------------------------- | ------------------------------------- |
| Clean syntax           | yes | yes| no   | yes | yes| yes|
| Fully-typed            | yes | yes| no   | no| yes| yes |
| Generic                | yes | yes| no   | yes | yes| yes |
| Mixed content support  | yes | hard     | yes | yes | hard | hard                                  |
| High-order composition | yes | possible | no   | yes | yes| possible                              |
| Wide adoption          | not yet | yes| yes | yes | limited                            | limited                               |

## Wasp Syntax

The major syntax extension Wasp makes to JSON is the introduction of a Wasp object. It is a JSON object extended with a type name and a list of content items, similar to element in HTML and XML.

For example, a HTML registration form:

```html
<form>
  <!--comment-->
  <div class="form-group">
    <label for="email">Email address:</label>
    <input type="email" id="email">
  </div>
  <div class="form-group">
    <label for="pwd">Password</label>
    <input type="password" id="pwd">
  </div>
  <button class='btn btn-info'>Submit</button>
</form>
```

Could be represented in Wasp/Angle as:

```text
form{                                  // object type-name 'form'
  //comment                          
  div{ class:"form-group"               // nested Wasp object
    label{ for:email                  // object with property 'for'
      "Email address:"                  // text needs to be quoted
    }
    input{ type:email id:email}     // object without any contents
  }
  div{ class:"form-group"
    label(for:pwd):"Password"
    input{ type:password id:"pwd"}    // comma is optional 
  }
  button{ class:['btn' 'btn-info']      // property with complex values
    'Submit'                            // text quoted with single quote
  }
}
```

## Wasp Data Model

Wasp has a very simple and fully-typed data model. 

Each Wasp object has 3 facets of data:

- **Type name**, which is mapped to `object.constructor.name` under JavaScript.
- **Properties**, which is a collection of key-value pairs, stored as normal JavaScript *named properties*.
- **Contents**, which is a list of content objects, stored as *indexed properties* inside the same JavaScript object.

Wasp utilizes a novel feature in JavaScript that a plain JS object is actually *array-like*, it can contain both named properties and indexed properties.

So each Wasp object is mapped to just **one** plain JavaScript object, which is more compact and efficient comparing to other JSON-based DOM models (e.g. [JsonML](http://www.jsonml.org/), [virtual-dom](https://github.com/Matt-Esch/virtual-dom), [MicroXML](https://dvcs.w3.org/hg/microxml/raw-file/tip/spec/microxml.html)), and more intuitive to used in JS.

Roughly speaking, data models of JSON, XML/HTML are subsets of Wasp data model, and Wasp data model is a subset of JS data model.

You can refer to the [data model spec](https://wasp.js.org/wasp-model.html) for details.

## Wasp vs. JSON

Wasp is a superset of JSON. It extends JSON notation with a type-name, and a list of content objects.

Comparing to JSON, Wasp has the following advantages:

- It has a type-name, which is important in identifying what the data represents; whereas JSON is actually an anonymous object, missing the type name.
- It can have nested content objects, which is common in all markup formats, and thus allows Wasp to convenient represent document-oriented data, which is awkward for JSON.
- It incorporates some syntax enhancements to JSON (e.g. allowing comments, property name without quotes, optional trailing comma or between properties and array values), and makes the format more friendly for human.

Some disadvantages of Wasp, comparing to JSON would be:

- It is no longer a subset of JavaScript in syntax, although a Wasp object is still a simple POJO in data model.
- It does not yet have wide support, like JSON, at the moment.

## Wasp vs. HTML

Comparing to HTML, Wasp has the following advantages:

- Wasp is a generic data format, whereas HTML is a specialized format for web content.
- It does not have whitespace ambiguity, as the text objects need to be quoted explicitly. Thus Wasp can be minified or prettified without worrying about changing the underlying content.
- Its properties can have complex values, like JSON, not just quoted string values as in HTML.
- It has a very clean syntax, whereas HTML5 parsing can be challenging even with HTML5 spec.
- Its objects are always properly closed; whereas HTML self-closing tag syntax is non-extensible and error-prone.
- The DOM produced under Wasp model, is just a hierarchy of POJO objects, which can be easily processed using the built-in JS functions or 3rd party libraries, making Wasp an ideal candidate for virtual DOM and other application usages.

## Wasp vs. XML

Comparing to XML, Wasp has the following advantages:

- Wasp properties can have complex object as value; whereas XML attribute values always need to be quoted and cannot have complex object as value, which is not flexible in syntax and data model.
- Wasp syntax is much cleaner than XML. It does not have whitespace ambiguity. It does not have all the legacy things like DTD.
- The data model produced by Wasp is fully typed, like JSON; whereas XML is only semi-typed without schema.

## Wasp vs. S-expressions

S-expression from Lisp gave rise to novel ideas like high-order composition, self-hosting program. The clean and flexible syntax of Wasp make it ideal for many such applications (e.g. [Wasp Template](https://github.com/henry-luo/wasp-template), a new JS template engine using Wasp for its template syntax).

The advantage of Wasp over S-expressions is that it takes a more modern, JS-first approach in its design, and can be more conveniently used in web and node.js environments.


## Usage

Install from NPM:

```
npm install angle --save
```

Then in your node script, use it as:

```js
const Wasp = require('angle');
var obj = Angle(`div:span:'Hello World!'`);  // using a shorthand
console.log("Greeting from Angle: " + obj);
```

To use the library in browser, you can include the `angle.js` into your html page, like:

```html
<script src='angle.js'></script>
<script>
var obj = Angle(`div:span:'Hello World!'`);  // using a shorthand
console.log("Greeting from Angle: " + obj);
</script>
```

Note: /dist/wasp.js has bundled wasp.mutate.js, wasp.convert.js and wasp.selector.js and all dependencies with it, and is meant to run in browser. The entire script is about 14K after gzip. It supports latest browsers, including Chrome, Safari, Firefox, Edge.

*If you just want the core functional API, without the sub-modules, you can also use wasp.core.js, which is only 7K after gzip. You can also refer to the package.json to create your own custom bundle with the sub-modules you need.* 

*If you need to support legacy browsers, like IE11, which do not have proper ES6 support, you need to use /dist/wasp.es5.js. IE < 11 are not supported.*

## Documentation

- [Syntax specification](https://wasp.js.org/wasp-syntax.html)
- [Data model and API specification](https://wasp.js.org/wasp-model.html)
- [FAQ](https://wasp.js.org/faq.html)
- Discussion about Wasp at [Hacker News](https://news.ycombinator.com/item?id=16308581)
- Examples:
  - You can take a look at all the [test scripts](https://github.com/henry-luo/wasp/tree/master/test), which also serve as basic demonstration of API usage.
  - [Wasp HTML example](https://plnkr.co/edit/DCgNxf?p=preview)
  - [Wasp conversion example](https://plnkr.co/edit/cMSCW3?p=preview)


## Differences to original Mark:
objects are created left of the bracket, so
`{html {body}}` in Mark is `html{body}` in Wasp / Angle.

Assignment and object creation
```
foo{bar:3}  #object creation
foo:{bar:3} #property assignment (nested objects)
foo={bar:3} #variable assignment (of calling context)
```

## Questions

should '=',':' be treated as operators or handled hardwiredly?
elegance vs efficiency


div{ class:"form-group"} vs div(class:"form-group") vs div{ class:{"form-group"}}
label(for:pwd):"Password"

Difference between Wasp and [Mark Notation](https://github.com/henry-luo/mark) :
Wasp  has the object/node type on the left side of braces:
Wasp: `html{ div{'hello world'}}` vs
Mark: `{html {div 'hello world'}}`

The main reason is that wasp is lisp based 
