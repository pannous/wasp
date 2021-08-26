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

Map<long, bool> analyzed;// avoid duplicate analysis (of if/while) todo: via simple tree walk, not this!

List<String> declaredFunctions;
//List<String> declaredFunctions;
Map<String, Signature> functionSignatures;

Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals;
Map<String /*function*/, List<Valtype> /* implicit indices 0,1,2,… */> localTypes;

Map<String /* implicit indices 0,1,2,… */, Node * /* compile-time modifiers/values? */> globals; // access from Angle!


// functions group externally square 1 + 2 == square(1 + 2) VS √4+5=√(4)+5
chars control_flows[] = {"if", "while", "unless", "until", "as soon as", 0};

class Arg {
public:
	String function;
	String name;
	Valtype type;
	Node modifiers;
};


Node constants(Node n) {
	if (eq(n.name, "not"))return True;// not () == True; hack for missing param todo: careful!
	if (eq(n.name, "one"))return Node(1);
	if (eq(n.name, "two"))return Node(2);
	if (eq(n.name, "three"))return Node(3);
	return n;
}


bool isFunction(String op) {
	if (declaredFunctions.has(op))return true;
	if (functionSignatures.has(op))return true;// pre registered signatures
	return op.in(function_list);// or op.in(functor_list); if
}

bool isFunction(Node &op) {
	if (op.kind == declaration)return false;
	return isFunction(op.name);
}

#include "Interpret.h"

#ifndef RUNTIME_ONLY
#endif
bool interpret = true; // todo

Node eval(String code) {
	Node parsed = parse(code);
#ifdef RUNTIME_ONLY
	return parsed; // no interpret, no emit => pure data  todo: WARN
#else
#ifndef WASI
	if (interpret)
		return parsed.interpret();
	else
#endif
	{
		int result = emit(analyze(parsed)).run();
		return Node(result);
	}
#endif

}

List<Arg> extractFunctionArgs(String function, Node &node) {
	List<Arg> args;
	if (node.length > 1)
		args.add({function, node.children[1].name, int32, node});
	if (node.length > 2)
		error("TODO args");
	return args;
}

String extractFunctionName(Node &node) {
	if (node.length > 1)return node.first().name;
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
chars ras[] = {"=", "?:", "+=", "++:", 0};
//List<chars> rightAssociatives = List(ras);
#ifndef WASM
List<chars> rightAssociatives = List<chars>{"=", "?:", "+=", "++:", 0};// a=b=1 == a=(b=1) => a=1
List<chars> prefixOperators = {"not", "!", "√", "-…", "--…", "++…", "+…", "~", "*…", "&…", "sizeof", "new", "delete[]"};
List<chars> suffixOperators = {"++", "--", "…++", "…--", "⁻¹", "⁰", "¹", "²", "³", "…%", "％", "﹪", "٪",
                               "‰"};// modulo % ≠ ％ percent
// todo: norm all those unicode variants first!
// ᵃᵇᶜᵈᵉᶠᵍʰᶥʲᵏˡᵐⁿᵒᵖʳˢᵗᵘᵛʷˣʸᶻ ⁻¹ ⁰ ⁺¹ ⁽⁾ ⁼ ⁿ

// handled different than other operators, because … they affect the namespace context
List<chars> setter_operators = {"="};
List<chars> constructor_operators = {":"};
List<chars> closure_operators = {"::", ":>", "=>", "->"}; // <- =: reserved for right assignment
List<chars> function_operators = {":="};// todo:
List<chars> declaration_operators = {":=", "="}; // , ":" NOT! if 1 : 2 else 3

// ... todo maybe unify variable symbles with function symbols at angle level and differentiate only when emitting code?
// x=7
// double:=it*2  // variable of type 'block' ?

#else
// TODO!
List<chars> rightAssociatives;
List<chars> prefixOperators;
List<chars> suffixOperators;
List<chars> setter_operators;
List<chars> declaration_operators;
List<chars> function_operators;
List<chars> constructor_operators;
//no matching constructor for initialization of 'List<chars>' (aka 'List<const char *>')
#endif

Node &groupFunctions(Node &expression0);

Node &groupDeclarations(Node &expression0, const char *context) {
	Node &expression = *expression0.clone();
	for (Node &node : expression) {
		String &op = node.name;
		if (node.kind == reference) {// only constructors here!
			if (not locals[context].has(op)) {
				locals[context].add(op);// todo : proper calling context!
#ifndef RUNTIME_ONLY
				localTypes[context].add(mapTypeToWasm(node));
#endif
			}
			continue;
		}
		if (node.kind == declaration or declaration_operators.has(op)) {
			// todo: public export function jaja (a:num …) := …
			Node modifiers = expression.to(node);// including public… :(
			Node rest = expression.from(node);
			String name = extractFunctionName(modifiers);

			if (isFunction(name))
				error("Symbol already declared as function: "s + name);
//			if (isImmutable(name))
//				error("Symbol declared as constant or immutable: "s + name);


			if (name and function_operators.has(op)) {
				declaredFunctions.add(name);
//				functionIndices[name]=functionIndices.size(); don't need an index yet!
			}

			Node *body = analyze(rest).clone();
			Node *decl = new Node(name);//node.name+":={…}");
			decl->setType(declaration);
			decl->metas().add(modifiers);
			decl->add(body);// addChildren makes emitting harder

			if (setter_operators.has(op) or constructor_operators.has(op)) {
				if (modifiers.has("global"))
					globals.insert_or_assign(name, body);
				else {
#ifndef RUNTIME_ONLY
					if (not locals[context].has(name)) {
						locals[context].add(name);// todo : proper calling context!
						localTypes[context].add(mapTypeToWasm(*body));
					} else {
						int i = locals[context].position(name);
						localTypes[context][i] = mapTypeToWasm(*body);// update type! todo: check if cast'able!
					}
#endif
				}
				decl->setType(assignment);
				return *decl;
			}

			List<Arg> args = extractFunctionArgs(name, modifiers);
#ifndef RUNTIME_ONLY
			Signature &signature = functionSignatures[name];// use Default
//			signature.is_used=true;// maybe
			signature.emit = true;// all are 'export'

			for (Arg arg: args) {
				locals[name].add(arg.name);
				localTypes[name].add(int32);
				signature.add(int32);// todo: arg type, or pointer
			}
			if (signature.size() == 0 and locals[name].size() == 0 and rest.has("it", false, 100)) {
				localTypes[name].add(int32);
				signature.add(int32);// todo
			}
			signature.returns(i32t);// todo what if not lol
			if (signature.return_type != int32)
				error("BUG");
#endif
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


bool isVariable(String name, Node context0) {
	if (globals.has(name))return true;
	String context = "main";// context0.name;// todo find/store proper enclosing context of expression
	if (locals[context].has(name))return true;
	return false;
}

Node &groupOperators(Node &expression0) {
	Node &expression = *expression0.clone();// modified in place!
	if (analyzed.has(expression0.hash()))return expression;
	analyzed.insert_or_assign(expression0.hash(), 1);
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
		if (node.length)continue;// already processed
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
				expression.replace(i - 1, i + 1, node);// replace ALL REST
				expression.remove(i, -1);
			} else {
				//#ifndef RUNTIME_ONLY
				if (node.name == "=" and prev.kind == reference)
					locals["main"].add(prev.name);
				//#endif
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

Node &groupIf(Node n);

Node &groupWhile(Node n);

Node &groupFunctions(Node &expression0) {
	if (expression0.kind == declaration)return expression0;// handled before
	Node &expression = *expression0.clone();
	for (int i = 0; i < expression.length; ++i) {
//	for (int i = expression.length; i>0; --i) {
		Node &node = expression.children[i];
		String &name = node.name;
		if (name == "if") // kinda functor
		{
			Node iff = groupIf(expression0.from("if"));
			int j = expression.lastIndex(iff.last().next) - 1;
			if (j > i)expression.replace(i, j, iff);// todo figure out if a>b c d e == if(a>b)then c else d; e boundary
		}
		if (name == "while") {
			if (node.length == 2) {
				node[0] = analyze(node[0].setType(expressions));
				node[1] = analyze(node[1].setType(expressions));
				continue;// all good (right?)
			}
			Node iff = groupWhile(expression0.from("while"));
			Node &last = iff.last();
			Node *next = last.next;
			int j = expression.lastIndex(next) - 1;
			if (j > i)expression.replace(i, j, iff);
		}
		if (isFunction(node)) // todo: may need preparsing of declarations!
			node.kind = call;// <- there we go!
		if (node.kind != call)
			continue;

		if (node.length > 0) {
			expression.replace(i, i, groupOperators(node));
			continue;// already done HOW??
		}
//		else found function call!
		if (not functionSignatures.has(name))
			error("missing import for function "s + name);
		functionSignatures[name].is_used = true;

		int minArity = functionSignatures[name].size();// todo: default args!
		int maxArity = functionSignatures[name].size();
		Node rest;
		if (expression[i + 1].kind == groups) {// f(x)
			// todo f (x) (y) (z)
			// todo expression[i+1].length>=minArity
			rest = expression[i + 1];
			if (rest.length > 1)rest.setType(expressions);
			Node args = analyze(rest);
			node.add(args);
			expression.remove(i + 1, i + 1);
			continue;
		}
		rest = expression.from(i + 1);
		if (rest.length > 1)
			rest.setType(expressions);// SUUURE?
		if (rest.kind == groups)// and rest.has(operator))
			rest.setType(expressions);// todo f(1,2) vs f(1+2)
		if (hasFunction(rest) and rest.first().kind != groups)
			error("Ambiguous mixing of functions `ƒ 1 + ƒ 1 ` can be read as `ƒ(1 + ƒ 1)` or `ƒ(1) + ƒ 1` ");
		if (rest.first().kind == groups)
			rest = rest.first();
		// per-function precedence does NOT really increase readability or bug safety
		if (rest.value.data) {
			maxArity--;// ?
			minArity--;
		}
		if (rest.length < minArity)
			error("missing arguments for function %s, currying not yet supported"s % name);
		else if (rest.length == 0 and minArity > 0)
			error("missing arguments for function %s, or to pass function pointer use func keyword"s % name);
		else if (rest.first().kind == operators) { // random() + 1 == random + 1
			// keep whole expression for later analysis in groupOperators!
			return expression;
		} else if (rest.length >= maxArity) {
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

// todo: un-adhoc this!
Node &groupWhile(Node n) {
	if (n.length == 0 and !n.value.data)
		error("no if condition given");
	if (n.length == 1 and !n.value.data)
		error("no if block given");

	Node &condition = n.children[0];
	Node then;
	Node *next; // outside while(){} !
	if (n.length == 0) then = n.values();
	if (n.length > 0) then = n[1];
	if (n.length >= 2 and !n.value.data) {
//		return n; // all good!
		condition = n[0];
		then = n[1];
		next = &n[2];
	}
	if (n.has("do")) {
		condition = n.to("do");
		then = n.from("do");
	}

	if (condition.value.data and !condition.next)
		then = condition.values();

	// todo: UNMESS how?
	if (n.has(":") /*before else: */) {
		condition = n.to(":");
		then = n.from(":");
	} else if (condition.has(":")) {// as child
		then = condition.from(":");
	}
	if (then.has("do"))
		then = n.from("do");

	Node *whilo = new Node("while");// regroup cleanly
	Node &ef = *whilo;
	ef.kind = expressions;
	//	ef.kind = ifStatement;
	if (condition.length > 0)condition.setType(expressions);// so far treated as group!
	if (then.length > 0)then.setType(expressions);
	ef.add(analyze(condition).clone());
	ef.add(analyze(then).clone());
//	ef.children[1] = analyze(then);
	ef.length = 2;
	ef.next = next;// todo all border cases!
//	ef["condition"] = analyze(condition);
//	ef["then"] = analyze(then);
	return ef;
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
//		condition = condition.interpret();// compile time evaluation?!
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
	if (condition.length > 0)condition.setType(expressions);// so far treated as group!
	if (then.length > 0)then.setType(expressions);
	if (otherwise.length > 0)otherwise.setType(expressions);
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
//	Node condit = condition.interpret();
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


Node analyze(Node data) {
#ifndef RUNTIME_ONLY
	locals.setDefault(List<String>());
	localTypes.setDefault(List<Valtype>());
	functionSignatures.setDefault(Signature());
#endif
	// group: {1;2;3} ( 1 2 3 ) expression: (1 + 2) tainted by operator
	if (data.kind == keyNode) {
		data.value.node = analyze(*data.value.node).clone();
	}
	if (data.kind == groups or data.kind == objects or data.kind == operators) {
		Node grouped = *data.clone();
		grouped.children = 0;
		grouped.length = 0;
		for (Node &child: data) {
			if (data.kind == operators)child.setType(expressions);// hack here
			child = analyze(child);// REPLACE with their ast? NO! todo
			grouped.add(child);
		}
		return grouped;
	}
	Node &groupedDeclarations = groupDeclarations(data, "main");
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

String debug_code;

void preRegisterSignatures() {
	// ORDER MATTERS: will be used for functionIndices later!

	// imports
	functionSignatures["logi"] = Signature().add(i32t).import();
	functionSignatures["logf"] = Signature().add(f32t).import();
	functionSignatures["square"] = Signature().add(i32t).returns(i32t).import();

	functionSignatures["main"] = Signature().returns(i32t);;

	// builtins
	functionSignatures["nop"] = Signature().builtin();
	functionSignatures["id"] = Signature().add(i32t).returns(i32t).builtin();
}

Node emit(String code) {
	debug_code = code;// global so we see when debugging
	Node data = parse(code);
#ifdef RUNTIME_ONLY
#ifdef INTERPRETER
	return data.interpret();
#endif
	return data;
#else
	data.log();
	globals.clear();
	locals.clear();
	locals.setDefault(List<String>());
	localTypes.clear();
	localTypes.setDefault(List<Valtype>());
	declaredFunctions.clear();
	functionSignatures.clear();
	functionSignatures.setDefault(Signature());
	locals.insert_or_assign("main", List<String>());
	preRegisterSignatures();// todo: reduntant to emitter
	analyzed.clear();// todo move much into outer analyze function!
	Node charged = analyze(data);
	Code binary = emit(charged);
//	code.link(wasp) more beautiful with multiple memory sections
	int result = binary.run();
	return Node(result);
#endif
}


float function_precedence = 1000;

// todo!
// moved here so that valueNode() works even without Angle.cpp component for micro wasm module
chars function_list[] = {"square", "log", "puts", "print", "printf", "println", "logi", "logf", "log_f32", "logi64",
                         "logx", "logc", "id", "get", "set", "peek", "poke", "read", "write", 0, 0,
                         0};// MUST END WITH 0, else BUG
chars functor_list[] = {"if", "while", 0};// MUST END WITH 0, else BUG

float precedence(Node &operater) {
	String &name = operater.name;
//	if (operater == NIL)return 0; error prone
	if (empty(name))return 0;// no precedence
	if (operater.kind == reals)return 0;//;1000;// implicit multiplication HAS to be done elsewhere!
	if (operater.kind == longs)return 0;//;1000;// implicit multiplication HAS to be done elsewhere!
	if (operater.kind == strings)return 0;// and empty(name)
	if (operater.kind == groups or operater.kind == patterns)
		return precedence("if") * 0.999;// needs to be smaller than functor/function calls
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
//	if (eq(name, "|"))return 7.2;// todo pipe special

	if (eq(name, ":"))return 7.5;// todo:

	if (name.in(function_list))// f 1 > f 2
		return 8;// 1000;// function calls outmost operation todo? add 3*square 4+1

	if (eq(name, "≠"))return 10;
	if (eq(name, "equals"))return 10;
	if (eq(name, "equal"))return 10;
	if (eq(name, "!="))return 10;

	if (eq(name, "⇒"))return 11; // lambdas
	if (eq(name, "=>"))return 11;
	if (eq(name, "::"))return 11;// todo lambda symbol? square = x :: x*x
	if (eq(name, "::="))return 11;// todo all

//	if (eq(name, ":"))return 12;// construction
	if (eq(name, "="))return 12;// declaration
	if (eq(name, ":="))return 13;

	if (eq(name, "else"))return 13.09;
	if (eq(name, "then"))return 13.15;
	if (eq(name, "if"))return 100;
	if (eq(name, "while"))return 101;
//	if (eq(name, "once"))return 101;
//	if (eq(name, "go"))return 101;
	if (name.in(functor_list))// f 1 > f 2
		return function_precedence;// if, while, ... statements calls outmost operation todo? add 3*square 4+1
	return 0;// no precedence
}


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

