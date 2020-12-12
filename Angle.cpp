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

bool recursive = true;// whats that?
List<String> declaredSymbols;// todo: buildup by preparsing


// functions group externally square 1 + 2 == square(1 + 2) VS √4+5=√(4)+5
chars control_flows[] = {"if", "while", "unless", "until", "as soon as", 0};


class Arg{
public:
	String function;
	String name;
	Valtype type;
	Node modifiers;
};


int main4(int argp, char **argv) {
#ifdef register_global_signal_exception_handler
	register_global_signal_exception_handler();
#endif
	try {
		Node("hello");
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


Node constants(Node n) {
	if (eq(n.name, "not"))return True;// not () == True; hack for missing param todo: careful!
	if (eq(n.name, "one"))return Node(1);
	if (eq(n.name, "two"))return Node(2);
	if (eq(n.name, "three"))return Node(3);
	return n;
}

Node eval(Node n) {
#ifndef WASI
	return n.evaluate();
#endif
	return Node();
}

Node If(Node condition, Node then) {
	Node ok0 = condition.evaluate();
	bool ok = (bool) ok0;
	if (condition.name == "0")ok = false;// hack
	if (ok)return then.evaluate();
	else return False;
}

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
		condition = condition.evaluate();
	}
	if (then.has("then"))
		then = n.from("then");
	if (then.has("else"))
		then = then.to("else");
//	if(condition.name=="condition")
//		condition = condition.values();

	Node condit = condition.evaluate();
	bool condition_fulfilled = (bool) condit;
	if (condition.kind == reals or condition.kind == longs)
		condition_fulfilled = (!condition.name or empty(condition.name)) and condition.value.data or condition.name != "0";
	else if (condition.value.data and condition.kind == objects) // or ...
		error("If statements need a space after colon");
	if (condition_fulfilled) {
		if (then.name == "then") {
			if (then.value.data or then.children) // then={} as arg
				return eval(then.values());
			return eval(n[2]);
		}
		return eval(then);
	} else {
		if (n.has("else"))
			return eval(n.from("else"));
		if (n.length == 3 and not n.has(":"))
			return eval(n[2]);// else
		else
			return False;
	}
}

bool isFunction(Node &op) {
	if (op.kind == declaration)return false;
	if (declaredSymbols.has(op.name))return true;
	return op.name.in(function_list);
}

bool isFunction(String op) {
	if (declaredSymbols.has(op))return true;
	return op.in(function_list);// or op.in(functor_list); if
}


Node Node::evaluate(bool expectOperator /* = true*/) {
	if (length == 0)return constants(*this);
	if (length == 1) {
		if (kind == operators or isFunction(*this))
			return apply_op(NIL, *this, *children);
		else return constants(children[0]);
	}

	if (length > 1)
		if (kind == operators or precedence(*this))
			return apply_op(NIL, *this, this->clone()->setType(objects).setName(empty_name));

	if (length == 2 and children[1].kind == expressions) {
		length = 1;
		return this->merge(children[1]).evaluate();// stop hacking
	}
//	if (type != expression and type != keyNode)
//		return *this;
	float max = 0; // do{
	float min = 999999;
	Node right;
	Node left;
	Node unknown_symbols;
	for (Node &node : *this) {// foreach
		float p = precedence(node);
		if (p > max) max = p;
		if (p < min and p != 0) min = p;
		if (p == 0 and node.kind == reference)unknown_symbols.add(node);
	}
	if (max == 0) {
		if (!empty(name) or length > 1) {
			breakpoint_helper // ok need not always have known operators
			info(String("No operator in : ") + serialize());
			if (unknown_symbols.length > 0 and expectOperator)
				error("unknown symbol "s + unknown_symbols.serialize());
		}
		for (int i = 0; i < length; ++i) {
			Node child = children[i];
			Node evaled = child.evaluate();
			children[i] = evaled;
		}
		return *this;
	}
	Node *op = 0;
	Node *inner = 0;// NIL;
	Node *prev = 0;
	for (Node &n : *this) {
		float p = precedence(n);
		if (p == min and not inner) inner = &n;
		else if (not inner) prev = &n;
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
			result = left.add(result).evaluate();
		}
		return result;
	}
//	};// while (max > 0);
	return *this;
}
bool interpret = true;

Node eval(String code) {
#ifndef WASI
	if (interpret)
		return parse(code).evaluate();
	else
#endif
#ifndef RUNTIME_ONLY
		return emit(analyze(parse(code))).run();// int -> Node todo: int* -> Node*
#else
		return parse(code).evaluate();
#endif
}

Node &groupIf(Node n);

List<Arg> extractFunctionArgs(String function, Node &node) {
	List<Arg> args;
	if(node.length>1)
	args.add({function, node.children[1].name, int32, node});
	if(node.length>2)
		error("TODO args");
	return args;
}

String extractFunctionName(Node &node) {
	if(node.length>1)return node.first().name;
	// todo: public go home to family => go_home
	return node.name;
}

// if a then b else c == a and b or c
// (a op c) => op(a c)
// further right means higher prescedence/binding, gets grouped first
// todo "=" ":" handled differently?


List<String> collectOperators(Node &expression) {
	List<String> operators;
	for (Node &op : expression) {
		if (operator_list.has(op.name))
//			if (op.name.in(operator_list))
			operators.add(op.name);
//		if (op.name.in(function_list))
//			operators.add(op.name);
//		if (op.name.in(functor_list))
//			operators.add(op.name);
	}
	auto by_precedence = [](String &a, String &b) { return precedence(a) > precedence(b); };
//	auto by_precedence = [](String &a) { return -precedence(a); };
	operators.sort(by_precedence);
	return operators;
}

//https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Operator_precedence
//List<String> rightAssociatives = {"=", "?:", "+=", "++:"};// a=b=1 == a=(b=1) => a=1
chars ras[]={"=", "?:", "+=", "++:",0};
//List<chars> rightAssociatives = List(ras);
#ifndef WASM
List<chars> rightAssociatives = List<chars>{"=", "?:", "+=", "++:",0};// a=b=1 == a=(b=1) => a=1
List<chars> prefixOperators = {"not", "!", "√", "-…", "--…", "++…", "+…", "~", "*…", "&…", "sizeof", "new", "delete[]"};
List<chars> suffixOperators = {"…++", "…--", "⁻¹", "⁰", "¹", "²", "³", "…%", "﹪", "％", "٪", "‰"};// ᵃᵇᶜᵈᵉᶠᵍʰᶥʲᵏˡᵐⁿᵒᵖʳˢᵗᵘᵛʷˣʸᶻ ⁻¹ ⁰ ⁺¹ ⁽⁾ ⁼ ⁿ
List<chars> declaration_operators = {":="};
#else
// TODO!
List<chars> rightAssociatives;
List<chars> prefixOperators;
List<chars> suffixOperators;
List<chars> declaration_operators;
//no matching constructor for initialization of 'List<chars>' (aka 'List<const char *>')
#endif

// a + b c + d
Node &groupDeclarations(Node &expression0) {
	Node &expression = *expression0.clone();
	for (Node &node : expression) {
		if (node.kind == declaration or declaration_operators.has(node.name)) {
			// todo: public export function jaja (a:num …) := …
			Node modifiers = expression.to(node);// including public… :(
			Node rest = expression.from(node);
			String name = extractFunctionName(modifiers);
			if(isFunction(name))
				error("Symbol already declared: "s+name);
			declaredSymbols.add(name);
			List<Arg> args= extractFunctionArgs(name, modifiers);
#ifndef RUNTIME_ONLY
			Signature& signature = functionSignatures[name];// use Default
			for(Arg arg: args){
				locals[name].add(arg.name);
				signature.add(int32);
			}
			if (signature.size() == 0 and rest.has("it", false, 100))
				signature.add(i32t);
			signature.returns(i32t);// todo what if not lol
#endif
			Node *body = analyze(rest).clone();
			Node *decl = new Node(name);//node.name+":={…}");
			decl->setType(declaration);
			decl->metas().add(modifiers);
//			decl->add(symbol);
			decl->add(body);// addChildren makes emitting harder
			return *decl;
		}
	}
	return expression;
}

bool hasFunction(Node &n) {
	for (Node &child : n) {
		if (isFunction(child))
			return true;
	}
	return false;
}

Node &groupOperators(Node &expression0) {
	Node &expression = *expression0.clone();// modified in place!
	if (expression.name == "if")return expression;// analyzed before!
	List<String> operators = collectOperators(expression);
	String last = "";
	int last_position = 0;
	for (String &op : operators) {
		if (op != last)last_position = 0;
		bool fromRight = rightAssociatives.has(op) or isFunction(op);
		fromRight = fromRight || prefixOperators.has(op); // !√!-1 == !(√(!(-1)))
		int i = expression.index(op, last_position, fromRight);
		if (i < 0) {
			i = expression.index(op, last_position, fromRight);// try again for debug
			expression0.log();
			expression.log();
			error("operator missing "s + op);
		}
		Node &node = expression.children[i];
		if(node.length)continue;// already processed
		Node &next = expression.children[i + 1];
		if (prefixOperators.has(node.name)) {// {++x
			node.add(next);
			expression.replace(i, i + 1, node);
		} else {
			Node &prev = expression.children[i - 1];
			if (suffixOperators.has(node.name)) { // x²
				if (i < 1)error("suffix operator misses left side");
				node.add(prev);
				expression.replace(i - 1, i, node);
			} else if (node.name.in(function_list)) {// handled above!
				while (i++ < node.length)
					node.add(expression.children[i]);
				expression.replace(i, node.length, node);
			} else if (isFunction(next)) { // 3 + double 8
				Node rest = expression.from(i + 1);
				Node args = analyze(rest);
				node.add(prev);
				node.add(args);
				expression.replace(i - 1,  i + 1 , node);// replace ALL REST
				expression.remove(i, -1);
			} else {
#ifndef RUNTIME_ONLY
				if(node.name=="=" and prev.kind==reference)
					locals["main"].add(prev.name);
#endif
				node.add(prev);
				node.add(next);
				expression.replace(i - 1, i + 1, node);
			}
		}
		last_position = i;
		last = op;
	}
	return expression;
}

Node &groupFunctions(Node &expression0) {
	if(expression0.kind==declaration)return expression0;// handled before
	Node &expression = *expression0.clone();
		for (int i = 0; i < expression.length; ++i) {
//	for (int i = expression.length; i>0; --i) {
		Node &node = expression.children[i];
		if (node.name == "if") // kinda functor
			return groupIf(expression0.from("if"));
		if (isFunction(node)) // todo: may need preparsing of declarations!
			node.kind = call;
		if (node.kind != call)
			continue;
		if (node.length > 0){
			expression.replace(i, i, groupOperators(node));
			continue;// already done HOW??
		}
//		else found function call!
		int maxArity = 1;// todo
		int minArity = 1;
		Node rest;
		if(expression[i+1].kind==groups){// f(x) todo f (x) (y) (z)
//	todo		expression[i+1].length>=minArity
			rest = expression[i + 1];
			if(rest.length>1)rest.setType(expressions);
			Node args = analyze(rest);
			node.add(args);
			expression.remove(i + 1, i + 1);
			continue;
		}
		rest= expression.from(i + 1);
		if(rest.length>1)
			rest.setType(expressions);// SUUURE?
		if(rest.kind==groups)// and rest.has(operator))
			rest.setType(expressions);// todo f(1,2) vs f(1+2)
		if (hasFunction(rest) and rest.first().kind != groups)
			error("Ambiguous mixing of functions `ƒ 1 + ƒ 1 ` can be read as `ƒ(1 + ƒ 1)` or `ƒ(1) + ƒ 1` ");
		if (rest.first().kind == groups)
			rest = rest.first();
		// per-function precedence does NOT really increase readability or bug safety
		if (rest.value.data) {
			maxArity--;
			minArity--;
		}
		if (rest.length < maxArity)
			error("missing arguments for function %s, currying not yet supported"s % node.name);
		else if (rest.length == 0 and minArity > 0)
			error("missing arguments for function %s, or to pass function pointer use func keyword"s % node.name);
		else if (rest.length >= maxArity) {
			Node args = analyze(rest);// todo: could contain another call!
			node.add(args);
			if (rest.kind == groups)
				expression.remove(i + 1, i + 1);
			else
				expression.remove(i + 1, i + rest.length);
		} else
			error("???");
	}
	return expression;
}


Node groupOperators2(Node &expression) {
	if (expression.name == "if")return groupIf(expression);
	if (expression.kind == operators and expression.length > 1)
		return expression;// already done (RIGHT!?)
	if (expression.kind == call and expression.meta and expression.meta->has("analyzed"))
		return expression;// already grouped
	if (expression.length == 0)return expression;
	if (expression.kind == longs)return expression;
	if (expression.length == 1)
		if (expression.kind != call)
			return groupOperators(expression.children[0]); // Nothing to be grouped
	expression.log();
	Node lhs;
	for (Node &fun : expression) {
		int isFunc = fun.name.in(function_list);
		int isControl = fun.name.in(control_flows);
		if ((isControl or isFunc) and fun.length == 0) { // todo: op.length>0 means already has body?
			if (isFunc) fun.kind = call;
			if (!fun.children) {
				Node *n = &fun;
				if (n->next and n->next->kind == groups)
					fun.addSmart(n->next); //f(x,y)+1
				else
					while ((n = n->next)) // f x+1
						fun.add(n);
			}
			if (!fun.meta)fun.meta = new Node("analyzed");
			Node &flat = fun.flat();
			Node *right = groupOperators(flat).clone();// applied on children
			if (lhs.isEmpty())return *right;
			lhs.addSmart(right);
			return groupOperators(lhs);
		} else lhs.addSmart(fun);
	}

	// todo : left associativity! 3-2-1==(3-2)-1==0 NOT 3-(2-1)==2
	//	left associativity is called Right-to-left in C++ https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Operator_precedence !!
	// extra : chained comparison a < b < c is  (a < b) and (b < c), not equivalent to either (a < b) < c or a < (b < c)
	for (String operator_name : operator_list) {
		for (Node &op : expression) {
			if (op.name == operator_name) {
				if (op.kind == operators and op.length > 1)
					continue;
				Node lhs = expression.to(op);
				Node rhs = expression.from(op);
				op["lhs"] = groupOperators(lhs);
				op["rhs"] = groupOperators(rhs);
				if (op.children[0].next != &op.children[1])error("WOOT");
				if (expression.kind == call) {// f 3*3 => f(*(3 3))
					expression.children = op.clone();
					expression.length = 1;
					return expression;
				}
				// should NOT MODIFY original AST, because iterate by value, right?
				return op;// (a op c) => op(a c)
			}
		}
	}
	return expression;// no op
}


Node &groupIf(Node n) {
	if (n.length == 0 and !n.value.data)
		error("no if condition given");
	if (n.length == 1 and !n.value.data)
		error("no if block given");
	Node &condition = n.children[0];
	Node then;
	if (n.length > 0)then = n[1];
	if (n.length == 0) then = n.values();
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
		condition = condition.evaluate();
	}
	Node otherwise;
	if (n.has("else"))
		otherwise = n["else"];
	if (then.has("then"))
		then = n.from("then");
	if (then.has("else")) {
		otherwise = then.from("else");
		then = then.to("else");
	}
	if (n.length == 3 and otherwise.isEmpty())
		otherwise = n[2];
	Node *eff = new Node("if");
	Node &ef = *eff;
	ef.kind = expressions;
//	ef.kind = ifStatement;
	if(condition.length>0)condition.setType(expressions);// so far treated as group!
	if(then.length>0)then.setType(expressions);
	if(otherwise.length>0)otherwise.setType(expressions);
	ef["condition"] = analyze(condition);
	ef["then"] = analyze(then);
	ef["else"] = analyze(otherwise);
//	ef[0] = groupOperators(condition);
//	ef[1] = groupOperators(then);
//	ef[2] = groupOperators(otherwise);
//	Node &node = ef["else"];// debug
	Node &node = ef[2];// debug
	return ef;
	return *ef.clone();
//
//	Node condit = condition.evaluate();
//	bool condition_fulfilled = (bool) condit;
//	if (condition.kind == reals or condition.kind == longs)
//		condition_fulfilled = empty(condition.name) and condition.value.data or condition.name != "0";
//	else if (condition.value.data and condition.kind == objects) // or ...
//		error("If statements need a space after colon");
//	if (condition_fulfilled) {
//		if (then.name == "then") {
//			if (then.value.data or then.children) // then={} as arg
//				return eval(then.values());
//			return eval(n[2]);
//		}
//		return eval(then);
//	} else {
//		if (n.has("else"))
//			return eval(n.from("else"));
//		if (n.length == 3 and not n.has(":"))
//			return eval(n[2]);// else
//		else
//			return False;
//	}
}

Node do_call(Node left, Node op0, Node right) {
	String op = op0.name;
	if (op == "id")return right;// identity
	if (op == "square")return Node(square(right.numbere()));
	if (op == "√")return Node(sqrt1(right.numbere()));
	if (op == "printf") {
		log(right);
		return right;
	}

	error("Unregistered function "s + op);
	return ERROR;
}

Node matchPattern(Node object, Node pattern0) {
//	[1 2 3]#1 == 1 == [1 2 3][0]
	Node pattern = pattern0.evaluate(); // [1 2 3][3-2]==2
	if (pattern.kind == longs)return object[(int) pattern.numbere()];
	if (pattern.kind == strings)return object[pattern.value.string];
// todo proper matches, references...
	return object[pattern.name];
}

/*
0x2218	8728	RING OPERATOR	∘
 */
Node Node::apply_op(Node left, Node op0, Node right) {
//	printf("apply_op\n");
//	left.log();
//	op0.log();
//	right.log();
//	if(right.length==0 and op0.param){
//		warn("using param for args");
//		right = *op0.param;
//	}
	String &op = op0.name;
	if (!isFunction(op)) // 1 + square 2  => "1+" kept dangling
		left = left.evaluate(false);
	bool lazy = (op == "or") and (bool) left;
	lazy = lazy || (op == "and") and not(bool) left;
	lazy = lazy || (op == "#");// length and index
	lazy = lazy || (op == "if");
//	lazy = lazy || arg#n is block

	if (!lazy)
		right = right.evaluate(false);

	if (isFunction(op))
		return do_call(left, op0, right);

	if (op0.kind == patterns)
		return matchPattern(left, op0);

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
			long index = right.value.longy;
			if (index <= 0)
				error("index<=0 ! Angle index operator # starts from 1. So [a b c]#2 == b. Use [] operator for zero based indexing");
			if (index > left.length)error("Index out of range: %d > %d !"s % index % left.length);
			return left.children[index - 1];
		}
	}


	if (op == "not" or op == "¬" or op == "!") {
		// todo: what if left is present?
		Node x = right.evaluate();
		return x.isEmpty() ? True : False;
	}

	if (op == "√") { // why String( on mac?
		if (right.kind == reals)
			left.addSmart(Node(sqrt1(right.value.real)));
		if (right.kind == longs)
			left.addSmart(Node(sqrt1(right.value.longy)).setType(reals));
		return left.evaluate();
	}

//	if(!is_KNOWN_operator(op0))return call(left, op0, right);

	if (op == "|") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == longs and right.kind == longs) return Node((long) (left.value.longy | right.value.longy));
		// pipe todo
	}

	if (op == "&") {// todo
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == bools or right.kind == bools)
			return left.value.data and right.value.data ? True : False;
		return Node(left.value.longy & right.value.longy);
	}

	if (op == "xor" or op == "^|") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == bools or right.kind == bools) {
			return left.value.longy ^ right.value.longy ? True : False;
		}
		return Node(left.value.longy ^ right.value.longy);
	}

	if (op == "and" or op == "&&") {
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
	if (op == "or" or op == "||" or op == "&") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (!left.isEmpty() and left != NIL and left != False)return left;
		return right;
	}

	if (op == "==" or op == "equals") {
		return left == right ? True : False;
	}

	if (op == "!=" or op == "^=" or op == "≠" or op == "is not") {
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
	}


	if (op == "*" or op == "⋆" or op == "×" or op == "∗" or op == "times") {// ⊗
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
			if (right.kind == strings)left.value.string.length = right.value.string.length;// DONT WORKAROUND BUGS!!
		} else
			left.value.node = &right;
		return left;
	}
	if (op == "else" or op == "then")return right;// consume by "if"! todo COULD be used as or if there is no 'if'
	if (op == "if") return If(right);
	if (isFunction(op)) {
//		kind=Type::function; // functor same concept, different arguments
		// careful, functions take arguments, functors take bodies if(1,2,3)!=if{1}{2}{3}
	}
	todo("operator “%s” NOT defined for types %s and %s "s % op % typeName(left.kind) % typeName(right.kind));
	return NIL;
//	log("NO builtin operator "+op0+" calling…")
//	return call(left, op0, right);
}


Node analyze(Node data) {
#ifndef RUNTIME_ONLY
	functionSignatures.setDefault(Signature());
#endif
	// group: {1;2;3} ( 1 2 3 ) expression: (1 + 2) tainted by operator
	if(data.kind==keyNode){
		data.value.node = analyze(*data.value.node).clone();
	}
	if (data.kind == groups or data.kind == objects) {
		Node grouped = *data.clone();
		grouped.children = 0;
		grouped.length = 0;
		for (Node &child: data) {
			child = analyze(child);// REPLACE with their ast? NO! todo
			grouped.add(child);
		}
		return grouped;
	}
	Node &groupedDeclarations = groupDeclarations(data);
	Node &groupedFunctions = groupFunctions(groupedDeclarations);
	Node &grouped = groupOperators(groupedFunctions);
	return grouped;
}

Node analyze2(Node data) {
	if (data.kind == reference and data.length == 0)return data;
	if (data.kind == expressions or data.kind == declaration) {
		return groupOperators(data);
	} else // or data.kind==groups or
		warn("REPLACE with their ast?");
	Node grouped = *data.clone();
	grouped.children = 0;
	grouped.length = 0;
	for (Node &child: data) {
		child = analyze(child);// REPLACE with their ast? NO! todo
		grouped.add(child);
	}
	grouped.log();
	return grouped;
}

Node emit(String code) {
	Node data = parse(code);
#ifdef RUNTIME_ONLY
	return data.evaluate();
#else
	data.log();
	locals.clear();
	locals.setDefault(List<String>());
	declaredSymbols.clear();
	functionSignatures.clear();
	functionSignatures.setDefault(Signature());
	locals.insert_or_assign("main", List<String>());
	Node charged = analyze(data);
	Code binary = emit(charged);
//	code.link(wasp) more beautiful with multiple memory sections
	Node node=binary.run();
	return node;
#endif
}


float function_precedence = 1000;

// todo!
// moved here so that valueNode() works even without Angle.cpp component for micro wasm module
chars function_list[] = {"square", "log", "puts", "print", "printf", "println", "logi", "logf", "log_f32", "logi64",
                         "logx", "logc", "id", 0};// MUST END WITH 0, else BUG
chars functor_list[] = {"if", "while", 0};// MUST END WITH 0, else BUG

float precedence(Node &operater) {
	String &name = operater.name;
//	if (operater == NIL)return 0; error prone
	if (empty(name))return 0;// no precedence
	if (operater.kind == reals)return 0;//;1000;// implicit multiplication HAS to be done elsewhere!
	if (operater.kind == longs)return 0;//;1000;// implicit multiplication HAS to be done elsewhere!
	if (operater.kind == strings)return 0;// and empty(name)
	if (operater.kind == groups or operater.kind == patterns) return precedence("if") * 0.999;// needs to be smaller than functor/function calls
	if (operater.name.in(function_list))return 999;// function call todo: remove here
	return precedence(name);
}


float precedence(String name) {
	// like c++ here HIGHER up == lower value == more important
//	switch (node.name) nope
//		name = operater.value.string;// NO strings are not automatic operators lol WTF
	if (eq(name, "."))return 0.5;
	if (eq(name, "of"))return 0.6;
	if (eq(name, "in"))return 0.7;
	if (eq(name, "from"))return 0.8;

	if (eq(name, "not"))return 1;
	if (eq(name, "¬"))return 1;
	if (eq(name, "-..."))return 1;
	if (eq(name, "!"))return 1;
	if (eq(name, "√"))return 1;// !√1 √!-1
	if (eq(name, "#"))return 3;// count
	if (eq(name, "++"))return 3;
//	if (eq(node.name, "+"))return 3;//
	if (eq(name, "--"))return 3;
	if (eq(name, "-…"))return 3;// 1 + -x

	if (eq(name, "/"))return 4.9;
	if (eq(name, "÷"))return 4.9;


	if (eq(name, "times"))return 5;
	if (eq(name, "*"))return 5;
	if (eq(name, "×"))return 5;
	if (eq(name, "add"))return 6;
	if (eq(name, "plus"))return 6;
	if (eq(name, "+"))return 6;
	if (eq(name, "minus"))return 6;
	if (eq(name, "-"))return 6;
	if (eq(name, "%"))return 6.1;
	if (eq(name, "rem"))return 6.1;
	if (eq(name, "modulo"))return 6.1;

	if (eq(name, "<"))return 6.5;
	if (eq(name, "<="))return 6.5;
	if (eq(name, ">="))return 6.5;
	if (eq(name, ">"))return 6.5;
	if (eq(name, "≥"))return 6.5;
	if (eq(name, "≤"))return 6.5;
	if (eq(name, "≈"))return 6.5;
	if (eq(name, "=="))return 6.6;

	if (eq(name, "and"))return 7.1;
	if (eq(name, "&&"))return 7.1;
	if (eq(name, "&"))return 7.1;
	if (eq(name, "xor"))return 7.2;
	if (eq(name, "or"))return 7.2;
	if (eq(name, "||"))return 7.2;

	if (eq(name, ":"))return 7.5;// todo:

	if (name.in(function_list))// f 1 > f 2
		return 8;// 1000;// function calls outmost operation todo? add 3*square 4+1

	if (eq(name, "⇒"))return 9;// todo
	if (eq(name, "=>"))return 9;// todo:
	if (eq(name, "="))return 10;
	if (eq(name, "≠"))return 10;
	if (eq(name, "!="))return 10;
	if (eq(name, ":="))return 11;
	if (eq(name, "equals"))return 10;
	if (eq(name, "equal"))return 10;
	if (eq(name, "else"))return 11.09;
	if (eq(name, "then"))return 11.15;
	if (eq(name, "if"))return 100;
	if (name.in(functor_list))// f 1 > f 2
		return function_precedence;// if, while, ... statements calls outmost operation todo? add 3*square 4+1
	return 0;// no precedence
}
