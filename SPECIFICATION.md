#Angle specification

## data=code format
```angle
cat{
    length:7cm
    hair-color:{r=1 g=0 b=0}
    form{
        dimensions=(3, length*2)
    }
    kill{print '🕱'}
}
```

The data format is a superset to (a subset) of emascript,json,css,mark,…

While it looks very different to lisp, it is in fact a very similar

*Data calculous* "lisp with builtin assignment and hashes"

With very few axioms and primitives:

##  Operators:
* `:` pair : turn two nodes into a pair, `a:3` (hashed+symbolic). almost identical to:
* `=` value : turn two nodes into a variable pair, `a=3`
* `,` list : turn two nodes into a list. Append if one is a list. 'cons' in lisp
* `;` list : end expressions/statements, list inside a block. if 1 : 0 ;
* `.` `#` `of` `in` `'s` selection: garden of house == house.garden == house's garden xs.1 xs#1
* `|` pipe/then : output of last command as input for next command. `ls ~ | sort`
* ` ` space acts as comma/concatenator in lists (a b c) == (a,b,c) == a->b->c
* ` ` newline, dedent: acts as comma in lists or closing 'bracket' if matching block start
usual math operators `add` `plus` `times` `power` `^` … and logical `and` `or` `xor` `not`

Question: substraction|concatenation|range  hair-color vs a-b vs 2001-2020

for static content `,` `;` ` ` are identical
{1 2 3} == {1,2,3} == {1;2;3}

## brackets: lists maps blocks 
for static content [],(),{} are identical

`[1,2,3]==(1,2,3)=={1,2,3}`
`[a:3]==(a:3)=={a:3}`

If attatched to objects, [],(),{} have different semantics:

`[export]square(a:int){a*a}`

### brackets
* […] attributes meta modifiers decorators annotations adjectives adverbs
* (…) groups parameters signatures attributes lists head
* {…} body blocks maps lambdas closures children nodes
* «…» other brackets behave like ()/{} until specified
* x[…] index patterns selectors matches 

brackets: content of () is evaluated, {} is deferred (block/lambda)

### optional order
[a]b == b[a] == a b  if a is modifier/operator
a + b == + a b   if feasible 

### optional brackets
f a b = f(a b) if f is a method/operator/functor

All kinds of brackets are often optional thanks to operator prescedence
`a:=1 2 3` == `a=(1,2,3)`
`[extern]void()` == `extern void`

Each line is treated as a group by default, unless escaped with \
so in lisp `(add (1 2))` is just `add 1 2` in angle

Brackets have different semantics depending on their context, this is NOT reflected in the data structure, only in its evaluation: 

[…]object vs object[…]
[attributes] are commonly written on the left side of objects [indices] on the right site.
 e.g. `[mutable]list xs` but `xs[1]=7`

Question: what about xs[mutable]=7?

### Indexing
the preferred way to index properties in objects to use the 'of' / 'in' notation
`show color of cat` or dot notation
`show cat.color`
list items are indexed from 0 in ordinary [] notation `items[0]`
list items are accessed from 1 in selector # notation `item#1`

#### value indexing | symbolic indexing

x=color
cat{color:red}
cat.color // red
cat.x // ø
cat#x // red
cat[x] // red
cat#color // red unless color is a variable
cat[color] // red unless color is a variable

Question: how is value indexing done in english? `property=color; get this property from cat`

<!-- cat$x // red later -->

car(list)=list#1
cdr(list)=rest of list=list…


### Evaluation time

To make sure an object is evaluated, invoke it with `!` or `()`
a=7
x={a+1}
print x  // {a+1}
print x! // 8
a=8
print x! // 9

parameters are evaluated if given in () and deferred if given as {}
print x  // 9
print(x) // 9
print{x} // {a+1}
print[x] // print has no property {a+1}

objects are evaluated if given in () and deferred if given as {}
y=(a+1)  // 8

NOTE: Everything inside {blocks} is deferred, including (groups)
a=7;x={print(a+1)};a=1;x() // 2
For instant template evaluation inside {blocks} use `$` :
a=7;x={print($a+1)};a=1;x() // 8

NOTE: Everything inside (groups) is evaluated, including {blocks}
z={x} print(z) // ({a+1}) (8+1) (9) 9

### Blocks
block is a builtin data type similar to 'object' (inspired by ruby)

Every group […] (…) {…} «…» is a block at first until special semantics are given.

A block is basically a deferred multiline lambda

f a b = f(a b) if f is a functor
example functor:
if.signature == condition:block then:block [optional]otherwise:block
if(a=b){go!}{stop!}

`if` could be implemented within the language:
if(block condition, block body, block otherwise){
}

Having blocks as first class citizens makes a lot of code cleaner:
square(x){x^2}
{it^2}[1 2 3] == [ 1 4 9]

blocks can be applied to objects from left or from right
{a*a}(a=7) == 49

blocks can be applied/evaluated via `!`
a=6;{a*a}! #36

blocks can be applied/evaluated via ()
ATTENTION
(){} has different semantics to {}()
the first is a deferred list [group(),block()]  
the second is an applied list [block(),group()] where the group is provided as context for the block

example:
(x){x*x}  a lambda (anonymous function)
{x*x}(x=5) #25 evaluated on the spot

This is particularily important because normaly () is just the empty object ø, but
{go}() will EXECUTE the block containing the statement 'go'; if 'go' was declared as function before, it will return the result of 'go'

b=7
x={a=b}
x.a # b (here:7)
y=x!
y=={a=7}
b=5
x.a # b (here:5)
y=={a=7} # unchanged because b was resolved


blocks can be applied to lists from left or from right
{it^2}[1 2 3]

### Overlap with javascript

While some blocks might look identical to javascript, the underlying logic is much more unified. Read the rest of this document for details.


`a={b:1,c:d()}`
Has near identical behavior in javascript and angle:
a variable('node'/'object') with name 'a' is created and has the properties b and c. while b is static, c recieves the dynamic value of d() at construction.

### Differences to json
json sucks, angle is more like ecmascript:

### Differences to javascript

`a={b:1,c:d()}` legal angle and javascript 
`a:{b=1,c{d()}}` legal angle 

Attention:


objects can be constructed with a{} a={} a:{} mostly equivalently 
in javascript '=' is used for variable setters and ':' is used for fields

DANGER a,b=c,d TWO READINGS 1. {a;b=c;d} 2. (a,b)=(c,d)

Groups, blocks, lists
() Node *params == group attributes meta modifiers decorators annotations
{} Node *children == block body content 'set':setters!=Set? [[Setter Problem]]
[] Node *list == selector pattern match
a[b]=c == a.b=c
a[[b]]=c (a that b)=c  a@b a~b


Diffenence of maps and constructors
   ` color:{r=1 g=0 b=0}` creates object called 'color' with r,g,b fields
   ` color{r=1 g=0 b=0}`  constructs instance of color class with r,g,b parameters
These are EQUIVALENT unless there is a constructor, e.g. 
`color{construct(r,g,b){this.rgb=r+g*2^8+b*2^16}}`

This is an example of class name as variable name. In this context color is both a variable with name 'color' and of type Color.

## Setter Problem
`cats=[cat{color=brown},cat{color=yellow}]` ok, two cats
`cats={cat{color=brown};cat{color=yellow}}` danger, cat property gets reassigned! really?
`cats(){color=green;sleep 1;color=blue}` ok, two setters in sequence

Then these two objects should still behave very similarily.
Especially the constructor arguments should be unmodified and are automatically accessible:
color.r==1 no matter what.


Assignment and object creation
foo{bar:3}  #object creation
foo(bar:3)  #object creation if (implicit) constructor, else function call
foo:{bar:3} #property assignment (nested objects)
foo={bar:3} #variable assignment (of calling context)

foo:=(x y){x+y} # function creation
:foo(x y){x+y} # function creation
to foo(x y){x+y} # function creation
