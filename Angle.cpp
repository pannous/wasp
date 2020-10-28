//
// Created by pannous on 18.05.20.
//
#define _main_
#include "Wasp.h"
#include "Angle.h"
#import "WasmHelpers.h" // IMPORT so that they don't get mangled!
#include "Node.h"
#include "wasm-emitter.h"

bool recursive = true;// whats that?

int main4(int argp, char **argv) {
#ifdef register_global_signal_exception_handler
	register_global_signal_exception_handler();
#endif
	try {
		error("HhhU");
//		testCurrent();
		return 42;
	} catch (chars err) {
		printf("\nERROR\n");
		printf("%s", err);
	} catch (String err) {
		printf("\nERROR\n");
		printf("%s", err.data);
	} catch (SyntaxError *err) {
		printf("\nERROR\n");
		printf("%s", err->data);
	}
	return -1;
}
int start(){ // for wasm-ld
	main4(0,0);
}
int _start(){ // for wasm-ld
	main4(0,0);
}

/*
0x2218	8728	RING OPERATOR	∘
 */
Node Node::apply(Node left, Node op0, Node right) {
	printf("apply");
	left.log();
	op0.log();
	right.log();
	left = left.evaluate();
	String &op = op0.name;
	bool lazy = (op == "or") and (bool) left;
	if (!lazy)
		right = right.evaluate(false);

	if (op == "not" or op == "¬" or op == "!") {
		// todo: what if left is present?
		Node x = right.evaluate();
		return x.empty() ? True : False;
	}
	if (op == "#" or op == '#') {
		return Node(right.length);// or right["size"] or right["count"]  or right["length"]
	}
	if (op == "√") { // why String( on mac?
		if (right.kind == floats)
			left.add(Node(sqrt(right.value.floaty)));
		if (right.kind == longs)
			left.add(Node(sqrt(right.value.number)).setType(floats));
		return left.evaluate();
	}

//	if(!is_KNOWN_operator(op0))return call(left, op0, right);

	if (op == "|") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == longs and right.kind == longs) return Node((long) (left.value.number | right.value.number));
		// pipe todo
	}

	if (op == "&") {// todo
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == bools or right.kind == bools)
			return left.value.data and right.value.data ? True : False;
		return Node(left.value.number & right.value.number);
	}

	if (op == "xor" or op == "^|") {
		printf("XOR REACHED");
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == bools or right.kind == bools){
			return left.value.number ^ right.value.number ? True : False;
		}
		return Node(left.value.number ^ right.value.number);
	}

	if (op == "and" or op == "&&") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == bools or right.kind == bools) return left.value.data and right.value.data ? True : False;
		return Node(left.value.number and right.value.number);
	}

	if (op == "or" or op == "||" or op == "&") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (!left.empty() and left != NIL and left != False)return left;
		return left.value.data or right.value.data ? True : False;
	}

	if (op == "==" or op == "equals") {
		return left == right ? True : False;
	}

	if (op == "!=" or op == "^=" or op == "≠" or op == "is not") {
		return left != right ? True : False;
	}

	if (op == "+" or op == "add" or op == "plus") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == floats and right.kind == floats) return Node(left.value.floaty + right.value.floaty);
		if (left.kind == floats and right.kind == longs) return Node(left.value.floaty + right.value.number);
		if (left.kind == longs and right.kind == floats) return Node(left.value.number + right.value.floaty);
		if (left.kind == longs and right.kind == longs) return Node(left.value.number + right.value.number);
//		if(left.type==arrays …
	}

	// todo: 2 * -x
	if (op == "-" or op == "minus" or op == "subtract") {
		if (left.kind == floats and right.kind == floats) return Node(left.value.floaty - right.value.floaty);
		if (left.kind == longs and right.kind == floats) return Node(left.value.number - right.value.floaty);
		if (left.kind == floats and right.kind == longs) return Node(left.value.floaty - right.value.number);
		if (left.kind == longs and right.kind == longs) return Node(left.value.number - right.value.number);
	}

	if (op == "*" or op == "⋆" or op == "×" or op == "∗" or op == "times") {// ⊗
		if (left.kind == strings or right.kind == strings) return Node(left.string().times(right.value.number));
		if (left.kind == floats and right.kind == floats) return Node(left.value.floaty * right.value.floaty);
		if (left.kind == longs and right.kind == floats) return Node(left.value.number * right.value.floaty);
		if (left.kind == floats and right.kind == longs) return Node(left.value.floaty * right.value.number);
		if (left.kind == longs and right.kind == longs) return Node(left.value.number * right.value.number);
		todo(op + " operator NOT defined for types %s and %s ");

//		if (right.type == numbers) return Node(left.value.number * right.value.number);
	}
	todo(op + " is NOT a builtin operator ");
	return NIL;
//	log("NO builtin operator "+op0+" calling…")
//	return call(left, op0, right);
}

Node &Node::setType(Type type) {
	if (length < 2 and (type == groups or type == objects)); // skip!
	else this->kind = type;
//	if(name.empty()){
//		if(type==objects)name = object_name;
//		if(type==groups)name = groups_name;
//		if(type==patterns)name = patterns_name;
//	}
	return *this;
}

Node values(Node n) {
//	if (eq(n.name, "not"))return True;// not () == True; hack for missing param todo: careful!
	if (eq(n.name, "one"))return Node(1);
	if (eq(n.name, "two"))return Node(2);
	if (eq(n.name, "three"))return Node(3);
	return n;
}

Node Node::evaluate(bool expectOperator /* = true*/) {
	if (length == 0)return values(*this);
	if (length == 1)
		if (kind == operators)
			return apply(NIL, *this, *children);
		else return values(children[0]);
	if (length > 1)
		if (kind == operators)
			return apply(NIL, *this, this->clone()->setType(objects));
//	if (type != expression and type != keyNode)
//		return *this;
	float max = 0; // do{
	Node right;
	Node left;
	for (Node &node : *this) {// foreach
		float p = precedence(node);
		if (p > max) max = p;
		node.log();
	}
	if (max == 0) {
//		breakpoint_helper // ok need not always have known operators
		if (!name.empty())
			warn(String("could not find operator: ") % name);
		return *this;
	}
	Node *op = 0;
	for (Node &n : *this) {
		float p = precedence(n);
		if (p == max and not op) {
			op = &n;
		} else if (op)
			right.add(n);
		else left.add(n);
	}
//		remove(&op);// fucks up pointers?
	if (recursive and op)
		return apply(left, *op, right);
//	};// while (max > 0);
	return *this;
}

Node eval(String code){
	return parse(code).evaluate();
}
// if a then b else c == a and b or c
// (a op c) => op(a c)
String operator_list[]={"+","-","*","/","^","xor","and","or"}; // "while" ...
Node groupOperators(Node expression){
	for(String op : operator_list){
		for(Node c : expression){
			if(c.name==op){
				expression.value.node=&c;
//				if binary op
				Node lhs = expression.to(c);
				Node rhs = expression.from(c);
				c["lhs"]=groupOperators(lhs);
				c["rhs"]=groupOperators(rhs);
				// promote expression to its main operator!  (a op c) => op(a c) !!!
				expression.kind = operators;
				expression.name = c.name;
				expression["lhs"]=groupOperators(lhs);
				expression["rhs"]=groupOperators(rhs);
				return c;// (a op c) => op(a c)
			}
		}
	}
}

Node Angle::analyze(Node data){
	if(data.kind==expression){
		return groupOperators(data);
	}
	for(Node child: data)
		analyze(child);
	return data;
}
Node analyze(Node data){
	return Angle::analyze(data);
}

Node emit(String code){
	Node data=parse(code);
	Node charged = Angle::analyze(data);
	Node node = emit(charged).run();
	return node;
}
