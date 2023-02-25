//
// Created by pannous on 18.05.20.
//
#define _main_

#include "Wasp.h"
#include "Angle.h"
#import "wasm_helpers.h" // IMPORT so that they don't get mangled!
#include "Node.h"
#include "wasm_emitter.h"
#include "Map.h"
#include "wasm_runner.h"


Node interpret(Node &n) {
#ifdef WASI
    return NUL;
#endif
    return n.interpret();
}

Node If(Node condition, Node then) {
    Node ok0 = condition.interpret();
    bool ok = (bool) ok0;
    if (condition.name == "0")ok = false;// hack
    if (ok)return then.interpret();
    else return False;
}

Node While(Node n) {
    // todo: Move this to the parser:
    if (n.length == 0 and !n.value.data)
        error("no if condition given");
    if (n.length == 1 and !n.value.data)
        error("no if block given");
    Node &condition = n.first();
    Node then = n.last();
    Node result;
    while (condition.interpret())
        result = then.interpret();
    return result;
}

bool recursive = true;// whats that?

Node Node::interpret(bool expectOperator /* = true*/) {
    if (length == 0)return constants(*this);
    if (length == 1) {
        if (kind == expression and name.empty())
            return first().interpret(true);
        if (kind == operators or name == "=" or name == ":=" or isFunction(*this))
            return apply_op(NIL, *this, *children);
        else return constants(children[0]);
    }
    if (kind == operators and length == 2) {
        return apply_op(first(), *this, last());
    }
    if (length > 1)
        if (kind == operators or precedence(*this)) {
            Node &right = this->values();
            right.kind = expression;
            right = right.interpret();
            this->clear();
            this->setType(operators, false);// restore
            if (right == this)
                error("no value for operator");
            return apply_op(NIL, *this, right);
        }

    if (length == 2 and children[1].kind == expression) {
        length = 1;
        return this->merge(children[1]).interpret();// stop hacking
    }
//	if (type != expression and type != key)
//		return *this;
    float max = 0; // do{
    float min = 999999;
    Node right;
    Node left;
    Node unknown_symbols;
    for (Node &node: *this) {// foreach
        float p = precedence(node);
        if (p > max) max = p;
        if (p < min and p != 0) min = p;
        if (p == 0 and node.kind == reference)
            unknown_symbols.add(node);
    }
    if (max == 0) {
        if (!name.empty() or length > 1) {
            breakpoint_helper // ok need not always have known operators
            info(String("Info: No operator in : ") + serialize());
            if (unknown_symbols.length > 0 and expectOperator)
                error("unknown symbol "s + unknown_symbols.serialize());
        }
        if (children)
            for (int i = 0; i < length; ++i) {
                Node child = children[i];
                Node evaled = child.interpret();
                children[i] = evaled;
            }
        return *this;
    }
    Node *op = 0;
    Node *inner = 0;// NIL;
//	Node *prev = 0;
    for (Node &n: *this) {
        float p = precedence(n);
        if (p == min and not inner) inner = &n;
//		else if (not inner) prev = &n;
        if (p == max and not op) {
            op = &n;
        } else if (op)
            right.add(n);
        else
            left.add(n);
    }
//	const Node &arg = inner.next ? *inner.next : NIL;
//	replace ... apply_op(prev, inner, arg);

    if (op->children and right.isEmpty()) {
        right = op->children[0];
    }// DONT work around bugs like this!!
//		remove(&op);// fucks up pointers?
    if (recursive and op) {
        right = right.flat();
        Node left1 = left.flat();
        Node result = apply_op(left1, *op, right);// <<<<<<<<<
        if (isFunction(*op) and not left.isEmpty()) {
            result = left.add(result).interpret();
        }
        return result;
    }
//	};// while (max > 0);
    return *this;
}

// todo? a if b == if b : a
// todo? a unless b == unless b : a
// todo? a while b == while b : a
// todo? a until b == until b : a
// todo? UNIFY:
Node If(Node n) {
    if (n.length == 0)return If(n, n.values());
    if (n.length == 0 and !n.value.data)
        error("no if condition given");
    if (n.length == 1 and !n.value.data)
        error("no if block given");
    Node &condition = n.children[0];
    Node then = n[1];

    if (n.has("then")) {
        condition = n.to("then");
        then = n.from("then");
    }

    if (condition.value.data and !condition.next)
        then = condition.values();
    if (condition.next and condition.next->name == "else")
        then = condition.values();

    // todo: UNMESS how?
    if (n.has(":") /*before else: */) {
        condition = n.to(":");
        if (condition.has("else"))
            condition = condition.to("else");// shouldn't happen?
        then = n.from(":");
    } else if (condition.has(":")) {// as child
        then = condition.from(":");
        condition = condition.interpret();
    }
    if (then.has("then"))
        then = n.from("then");
    if (then.has("else"))
        then = then.to("else");
//	if(condition.name=="condition")
//		condition = condition.values();

    Node condit = condition.interpret();
    bool condition_fulfilled = (bool) condit;
    if (condition.kind == reals or condition.kind == longs)
        condition_fulfilled =
                ((!condition.name or empty(condition.name)) and condition.value.data) or condition.name != "0";
    else if (condition.value.data and condition.kind == objects) // or ...
        error("If statements need a space after colon");
    if (condition_fulfilled) {
        if (then.name == "then") {
            if (then.value.data or then.children) // then={} as arg
                return interpret(then.values());
            return interpret(n[2]);
        }
        return interpret(then);
    } else {
        if (n.has("else"))
            return interpret(n.from("else"));
        if (n.length == 3 and not n.has(":"))
            return interpret(n[2]);// else
        else
            return False;
    }
}

Node do_call(Node left, Node op0, Node right) {
    String op = op0.name;
    if (op == "id")return right;// identity
//	if (op == "square")return Node(square(right.numbere()));
    if (op == "square")return Node(right.numbere() * right.numbere());// don't test
    if (op == "√")return Node(sqrt1(right.numbere()));
    if (op == "printf") {
        print(right);
        return right;
    }
    error("Unregistered function "s + op);
	return ERROR;
}

Node matchPattern(Node object, Node pattern0) {
//	[1 2 3]#1 == 1 == [1 2 3][0]
    Node pattern = pattern0;
    if (pattern0.kind == expression)
        pattern = pattern0.interpret(); // [1 2 3][3-2]==2
    if (object.isNil())return pattern;
    if (pattern.kind == longs)return object[(int) pattern.numbere()];
    if (pattern.kind == strings)return object[pattern.value.string];
    if (pattern.kind == reference and pattern.isEmpty())
        return object[pattern.name];
    if (pattern.kind == patterns) {
        if (pattern.isEmpty())return object; // empty pattern returns object ok?
        return matchPattern(object, pattern.first());
    }
    // todo: by name ok?
    todo("proper matches, referenceIndices... in matchPattern");
    return object[pattern.name];
}


Node Node::apply_op(Node left, Node op0, Node right) {

    if (debug) {
        trace("apply_op\n");
        left.print();
        op0.print();
        right.print();
    }
    if (left.kind == reference) {
        if (globals.has(left.name))
            left = *globals[left.name].value;
    }

    if (right.kind == reference) {
        if (globals.has(right.name))
            right = *globals[right.name].value;
    }

//	if(right.length==0 and op0.param){
//		warn("using param for args");
//		right = *op0.param;
//	}
    String &op = op0.name;
    if (!isFunction(op, true)) // 1 + square 2  => "1+" kept dangling
        left = left.interpret(false);
    bool lazy = (op == "or") and (bool) left;
    lazy = lazy || (op == "and" and not(bool) left);
    lazy = lazy || (op == "#");// length and index
    lazy = lazy || (op == "if");
    lazy = lazy || (op == "while");
//	lazy = lazy || arg#n is block

    if (!lazy)
        right = right.interpret(false);

    if (isFunction(op, true))
        return do_call(left, op0, right);

    if (op == ".") {
        return matchPattern(left, right);
    }

    if (op == "of" or op == "in") {
        return matchPattern(right, left);
    }

    if (op == "#") {
        if (left.length == 0) // length operator #{a b c} == 3
            return Node(right.length);// or right["size"] or right["count"]  or right["length"]
        else {  // index operator [a b c]#2 == b
            int64 index = right.value.longy;
            if (index <= 0)
                error("index<=0 ! Angle index operator # starts from 1. So [a b c]#2 == b. Use [] operator for zero based indexing");
            if (index > left.length)error("Index out of range: %d > %d !"s % index % left.length);
            return left.children[index - 1];
        }
    }


    if (op == "not" or op == "¬" or op == "!") {
        // todo: what if left is present?
        Node x = right.interpret();
        return x.isEmpty() ? True : False;
    }

    if (op == "as") {
//		4.4 as int is 4
    }
    if (op == "peek") { // todo module memory.peek = memory.get = memory[index]
//		poke 65536, 42  set/write memory
//		peek 65536 #42  get/read  memory
    }
    if (op == "√") { // why String( on mac?
        if (right.kind == reals)
            left.addSmart(Node(sqrt1(right.value.real)));
        if (right.kind == longs)
            left.addSmart(Node(sqrt1(right.value.longy)).setType(reals));
        return left.interpret();
    }

//	if(!is_KNOWN_operator(op0))return call(left, op0, right);

    if (op == "‖") {
        if (right.isEmpty() and left.kind == longs) return Node(abs(left.value.longy));
        if (left.isEmpty() and right.kind == longs) return Node(abs(right.value.longy));
        if (right.isEmpty() and left.kind == reals) return Node(abs(left.value.real));
        if (left.isEmpty() and right.kind == reals) return Node(abs(right.value.real));
        error("missing value for ‖");
    }
    if (op == "|") {// bitwise or OR pipe!
        if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
        if (left.kind == longs and right.kind == longs) return Node((int64) (left.value.longy | right.value.longy));
        todo("PIPE a|b");
        // pipe todo
    }

    if (op == "&") {// todo
        if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
        if (left.kind == bools or right.kind == bools)
            return left.value.data and right.value.data ? True : False;
        return Node(left.value.longy & right.value.longy);
    }

    if (op == "xor" or op == "^|" or op == "⊻") {
        if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
        if (left.kind == bools or right.kind == bools) {
            return left.value.longy ^ right.value.longy ? True : False;
        }
        return Node(left.value.longy ^ right.value.longy);
    }

    if (op == "and" or op == "&&" or op == "∧" or op == "⋀") {
        if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
        if (left.kind == bools or right.kind == bools) return left.value.data and right.value.data ? True : False;
        if (left.value.longy)return right;
        return Node(left.value.longy and right.value.longy);// todo just False?
    }
/*
 `or`/`else` ARE NOT IDENTICAL:
`if 1 then 0 else 2 == 0`
`1 and 0 or 2 == 2`  !!!
*/
    if (op == "or" or op == "||" or op == "∨" or op == "⋁") {
        ::print("YA OR!!");
        if (left.isEmpty())return right;
        if (left.kind == bools) return left.value.longy == 1 ? True : right;
//		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());// eek no!!
        if (!left.isEmpty() and left != NIL and left != False) return left;
        return right;
    }

    if (op == "==" or op == "equals" or op == "≡" or op == "﹦" or op == "﹦" or op == "≟") { // ≝
        return left == right ? True : False;
    }

    if (op == "!=" or op == "=/" or op == "^=" or op == "≠" or op == "is not" or op == "isn't" or op == "isn't") {
        return left != right ? True : False;
    }
    if (op == "<" or op == "less" or op == "lt") {
        if (left.kind == strings or right.kind == strings) return Node(left.string() < right.string());
        if (left.kind == reals and right.kind == reals) return Node(left.value.real < right.value.real);
        if (left.kind == reals and right.kind == longs) return Node(left.value.real < right.value.longy);
        if (left.kind == longs and right.kind == reals) return Node(left.value.longy < right.value.real);
        if (left.kind == longs and right.kind == longs) return Node(left.value.longy < right.value.longy);
    }

    if (op == "<=" or op == "le" or op == "≤") {
        if (left.kind == strings or right.kind == strings) return Node(left.string() <= right.string());
        if (left.kind == reals and right.kind == reals) return Node(left.value.real <= right.value.real);
        if (left.kind == reals and right.kind == longs) return Node(left.value.real <= right.value.longy);
        if (left.kind == longs and right.kind == reals) return Node(left.value.longy <= right.value.real);
        if (left.kind == longs and right.kind == longs) return Node(left.value.longy <= right.value.longy);
    }

    if (op == ">=" or op == "ge" or op == "≥") {
        if (left.kind == strings or right.kind == strings) return Node(left.string() >= right.string());
        if (left.kind == reals and right.kind == reals) return Node(left.value.real >= right.value.real);
        if (left.kind == reals and right.kind == longs) return Node(left.value.real >= right.value.longy);
        if (left.kind == longs and right.kind == reals) return Node(left.value.longy >= right.value.real);
        if (left.kind == longs and right.kind == longs) return Node(left.value.longy >= right.value.longy);
    }

    if (op == ">" or op == "gt") {
        if (left.kind == strings or right.kind == strings) return Node(left.string() > right.string());
        if (left.kind == reals and right.kind == reals) return Node(left.value.real > right.value.real);
        if (left.kind == reals and right.kind == longs) return Node(left.value.real > right.value.longy);
        if (left.kind == longs and right.kind == reals) return Node(left.value.longy > right.value.real);
        if (left.kind == longs and right.kind == longs) return Node(left.value.longy > right.value.longy);
    }

    if (op == "+" or op == "add" or op == "plus") {
        if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
        if (left.kind == reals and right.kind == reals) return Node(left.value.real + right.value.real);
        if (left.kind == reals and right.kind == longs) return Node(left.value.real + right.value.longy);
        if (left.kind == longs and right.kind == reals) return Node(left.value.longy + right.value.real);
        if (left.kind == longs and right.kind == longs) return Node(left.value.longy + right.value.longy);
        todo(op + " operator NOT defined for types %s and %s "s % typeName(left.kind) % typeName(right.kind));
    }



    // todo: 2 * -x
    if (op == "-" or op == "minus" or op == "subtract") {
        if (left.kind == reals and right.kind == reals) return Node(left.value.real - right.value.real);
        if (left.kind == longs and right.kind == reals) return Node(left.value.longy - right.value.real);
        if (left.kind == reals and right.kind == longs) return Node(left.value.real - right.value.longy);
        if (left.kind == longs and right.kind == longs) return Node(left.value.longy - right.value.longy);
        // should be handled by nodeValue:
        if (left.empty() and right.kind == reals) return Node(-right.value.real);//negation
        if (left.empty() and right.kind == longs) return Node(-right.value.longy);//negation
    }


    if (op == "*" or op == "⋅" or op == "×" or op == "✕" or op == "⋆" or op == "∗" or op == "times") {// ⊗
        if (left.kind == strings) return Node(left.string().times(right.value.longy));
        if (right.kind == strings) return Node(right.string().times(left.value.longy));
        if (left.kind == reals and right.kind == reals) return Node(left.value.real * right.value.real);
        if (left.kind == longs and right.kind == reals) return Node(left.value.longy * right.value.real);
        if (left.kind == reals and right.kind == longs) return Node(left.value.real * right.value.longy);
        if (left.kind == longs and right.kind == longs) return Node(left.value.longy * right.value.longy);
    }

    if (op == "%" or op == "rem" or op == "modulo") {
        if (left.kind == longs and right.kind == longs) return Node(left.value.longy % right.value.longy);
    }

    if (op == "/" or op == "÷" or op == "div" or op == "divide") { // "by"
        if (left.kind == reals and right.kind == reals) return Node(left.value.real / right.value.real);
        if (left.kind == longs and right.kind == reals) return Node(left.value.longy / right.value.real);
        if (left.kind == reals and right.kind == longs) return Node(left.value.real / right.value.longy);
        if (left.kind == longs and right.kind == longs) return Node(left.value.longy / right.value.longy);
    }

    if (op == "=" or op == ":=" or op == ":" or op == "⇒" or op == "=>") {
        warn("proper '=' operator");
        left.kind = reference;
        if (right.value.data) {// and ...
            left.kind = right.kind; // there are certainly things lost here!?!
            left.value.data = right.value.data;// todo failed copy assignment: length=0!!!
            if (right.kind == strings)left.value.string->length = right.value.string->length;// DONT WORKAROUND BUGS!!
        } else
            left.value.node = &right;
        return left;
    }
    if (op == "else" or op == "then")return right;// consume by "if"! todo COULD be used as or if there is no 'if'
    if (op == "if") return If(right);
    if (op == "while") return While(right);
    if (isFunction(op, true)) {
//		kind=Type::function; // functor same concept, different arguments
        // careful, functions take arguments, functors take bodies if(1,2,3)!=if{1}{2}{3}
    }
    if (op0.kind == patterns and op0.isEmpty())// pattern can come from left or right!
        return left.isEmpty() ? right : left;
    if (op0.kind == patterns)// pattern can come from left or right!
        return matchPattern(left, op0);
    if (right.kind == patterns)
        return matchPattern(op0, right);

    todo("operator “%s” NOT defined for types %s and %s "s % op % typeName(left.kind) % typeName(right.kind));
    return NIL;
//	put("NO builtin operator "+op0+" calling…")
//	return call(left, op0, right);
}

