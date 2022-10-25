//
// Created by pannous on 18.05.20.
//
#define _main_

#include "Wasp.h"
#include "Angle.h"
#include "Node.h"
#include "Util.h"
#include "Map.h"
#import "wasm_helpers.h" // IMPORT so that they don't get mangled!
#include "wasm_emitter.h"

bool use_interpreter = false; // todo

List<String> builtin_constants
#ifndef WASM
		= {"pi", "π", "tau", "τ", "euler", "ℯ", 0}
#endif
;


Map<long, bool> analyzed;// avoid duplicate analysis (of if/while) todo: via simple tree walk, not this!

List<String> declaredFunctions;
//List<String> declaredFunctions;
Map<String, Signature> functionSignatures;

// todo : proper context!
Map<String /*function*/, List<String> /* implicit indices 0,1,2,… */> locals;
Map<String /*function*/, List<Valtype> /* implicit indices 0,1,2,… */> localTypes;

Map<String, Node * /* modifiers/values/init expressions! */> globals; // access from Angle!
Map<String /*name*/, Valtype> globalTypes;


// functions group externally square 1 + 2 == square(1 + 2) VS √4+5=√(4)+5
chars control_flows[] = {"if", "while", "unless", "until", "as soon as", 0};

class Arg {
public:
	String function;
	String name;
//	Valtype type;
//	Valtype kind;
	Node type;
	Node modifiers;
};

Valtype mapType(Node &n) {
#ifdef RUNTIME_ONLY
	return int32;
#else
	return mapTypeToWasm(n);
#endif
}


Node getType(Node node) {
	auto name = node.name;
	bool vector = false;
	if (name.endsWith("es")) { // addresses
		// todo: cities …
		name = name.substring(0, -3);
		vector = true;
	} else if (name.endsWith("s")) { // ints …
		name = name.substring(0, -2);
		vector = true;
	}
	Node typ;
	if (types.has(name)) {
		typ = types[name];
		typ.kind = clazz;
	} else {
		typ = Node(name);
		typ.kind = clazz;
		types[name] = typ;
	}
	if (vector) {
		// holup typ.kind = arrays needs to be applied to the typed object!
		typ.kind = arrays;
		typ.type = typ.clone();
//		typ.kind=vectors; // ok, copy value
	}
	return typ;
}


bool isType(Node &expression) {
	auto name = expression.name;
	// todo makes … verbs!
	if (name == "puts" or name == "plus" or name == "minus" or name == "times" or name == "is" or name == "has" or name == "was" or name == "does" or
	    name == "equals")
		return false;// todo …
	// todo: whitelist known singular types?
	if (name.endsWith("s")) { // numbers, persons …
		auto cut = name.substring(0, -2);
		return true;// todo and register?
	}
	return types.has(name);
}

String debug_code;


Node &groupWhile(Node n, String string);

bool isPrimitive(Node &node);

bool isType(Node &node);


Node constants(Node n) {
	if (eq(n.name, "not"))return True;// not () == True; hack for missing param todo: careful!
	if (eq(n.name, "one"))return Node(1);
	if (eq(n.name, "two"))return Node(2);
	if (eq(n.name, "three"))return Node(3);
	return n;
}


bool isFunction(String op) {
	if (op.empty())return false;
	if (declaredFunctions.has(op))return true;
	if (functionSignatures.has(op))return true;// pre registered signatures
	if (op.in(function_list))
		return true;
//	if(op.in(functor_list))
//		return true;
	return false;
}

bool isFunction(Node &op) {
	if (op.kind == strings)return false;
	if (op.kind == declaration)return false;
	return isFunction(op.name);
}

#include "Interpret.h"
#include "wasm_merger.h"

#ifndef RUNTIME_ONLY
#endif
void prepareContext();

Node interpret(String code) {
	Node parsed = parse(code);
	return parsed.interpret();
}


Node eval(String code) {
	Node parsed = parse(code);
#ifdef RUNTIME_ONLY
	return parsed; // no interpret, no emit => pure data  todo: WARN
#else
#ifndef WASI
	if (use_interpreter)
		return parsed.interpret();
	else
#endif
	{
		prepareContext();
		analyzed.setDefault(false);
		long results = emit(analyze(parsed)).run();
		auto resultNode = smartNode(results);
		printf("» %lx %s\n", results, resultNode.serialize().data);
		return resultNode;
	}
#endif

}

Node &groupTypes(Node &expression, const char *context);

List<Arg> groupFunctionArgs(String function, Node &params) {
	//			left = analyze(left, name) NO, we don't want args to become variables!
	List<Arg> args;
	Node nextType = Double;
	if (params.length == 0) {
		params = groupTypes(params, function);
		if (params.name != function)
			args.add({function, params.name, params.type ? *params.type : nextType});
	}
	for (Node &arg: params) {
		if (isType(arg)) {
			if (args.size() > 0 and not args.last().type)
				args.last().type = types[arg.name];
			else nextType = arg;
		} else {
			if (arg.name != function)
				args.add({function, arg.name, arg.type ? *arg.type : nextType, params});
		}
	}
	return args;
}

String extractFunctionName(Node &node) {
	if (not node.name.empty() and node.name != ":=")
		return node.name;
	if (node.length > 1)
		return node.first().name;
	// todo: public go home to family => go_home
	return node.name;
}

// if a then b else c == a and b or c
// (a op c) => op(a c)
// further right means higher prescedence/binding, gets grouped first
// todo "=" ":" handled differently?



//https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Operator_precedence
//List<String> rightAssociatives = {"=", "?:", "+=", "++:"};// a=b=1 == a=(b=1) => a=1
//chars ras[] = {"=", "?:", "+=", "++", 0};
//List<chars> rightAssociatives = List(ras);
#ifndef WASM
List<chars> rightAssociatives = List<chars>{"=", "?:", "-…", "+=", "++…", 0};// a=b=1 == a=(b=1) => a=1


// still needs to check a-b vs -i !!
List<chars> prefixOperators = {"abs",/*norm*/  "not", "¬", "!", "√", "-" /*signflip*/, "--", "++", /*"+" useless!*/
                               "~", "&", "$", "return",
                               "sizeof", "new", "delete[]", "floor", "round", "ceil", "peek", "poke"};
List<chars> suffixOperators = {"++", "--", "…++", "…--", "⁻¹", "⁰", /*"¹",*/ "²", "³", "ⁿ", "…%", /* 23% vs 7%5 */ "％",
                               "﹪", "٪",
                               "‰"};// modulo % ≠ ％ percent
//List<chars> prefixOperators = {"not", "!", "√", "-…" /*signflip*/, "--…", "++…"/*, "+…" /*useless!*/, "~…", "*…", "&…",
//							  "sizeof", "new", "delete[]"};
//List<chars> suffixOperators = { "…++", "…--", "⁻¹", "⁰", /*"¹",*/ "²", "³", "ⁿ", "…%", "％", "﹪", "٪",
//							   "‰"};// modulo % ≠ ％ percent



List<chars> infixOperators = operator_list;
// todo: norm all those unicode variants first!
// ᵃᵇᶜᵈᵉᶠᵍʰᶥʲᵏˡᵐⁿᵒᵖʳˢᵗᵘᵛʷˣʸᶻ ⁻¹ ⁰ ⁺¹ ⁽⁾ ⁼ ⁿ

// handled different than other operators, because … they affect the namespace context
List<chars> setter_operators = {"="};
List<chars> key_pair_operators = {":"};
List<chars> closure_operators = {"::", ":>", "=>", "->"}; // <- =: reserved for right assignment
List<chars> function_operators = {":="};// todo:
List<chars> declaration_operators = {":=", "=",
                                     "::=" /*until global keyword*/}; //  i:=it*2  vs i=1  OK?  NOT ":"! if 1 : 2 else 3

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
List<chars> key_pair_operators;
//no matching constructor for initialization of 'List<chars>' (aka 'List<const char *>')
#endif

Node &groupIf(Node n, String context) {
	if (n.length == 0 and !n.value.data)
		error("no if condition given");
	if (n.length == 1 and !n.value.data)
		error("no if block given");
	Node &condition = n.first();
	Node then;
	if (n.length > 0)then = n[1];
	if (n.length == 0) then = n.values();
	if (n.kind == key) {
		condition = n.from(1);
		then = *n.value.node;
	}
	if (n.has("then")) {
		condition = n.to("then");
		then = n.from("then");
	}

	if (condition.kind == key and condition.value.data and !condition.next)
		then = condition.values();
	if (condition.next and condition.next->name == "else")
		then = condition.values();

	// todo: UNMESS how?
	if (n.has(":") /*before else: */) {
		condition = n.to(":");
		if (condition.has("else"))
			condition = condition.to("else");// shouldn't happen?
		if (then.length == 0)
			then = n.from(":");
	} else if (condition.has(":")) {// as child
		then = condition.from(":");
		//		condition = condition.interpret();// compile time evaluation?!
	}
	Node otherwise;
	if (n.has("else"))
		otherwise = n["else"].values();
	if (then.has("then"))
		then = n.from("then");
	if (then.has("else")) {
		otherwise = then.from("else");
		then = then.to("else");
	}
	if (then.name == ":") {
		if (then.length > 1)
			otherwise = then[1];
		if (then.length > 2)
			error("too many clauses for if?");
		then = then.first();
	}
	if (n.length == 3 and otherwise.isEmpty())
		otherwise = n[2];
	Node *eff = new Node("if");
	Node &ef = *eff;
	ef.kind = expression;
	//	ef.kind = ifStatement;
	if (condition.length > 0)condition.setType(expression);// so far treated as group!
	if (then.length > 0)then.setType(expression);
	if (otherwise.length > 0)otherwise.setType(expression);
	ef["condition"] = analyze(condition, context);
	ef["then"] = analyze(then, context);
	ef["else"] = analyze(otherwise, context);
	//	condition = analyze(condition);
	//	then = analyze(then);
	//	otherwise = analyze(otherwise);
	//	ef.add(condition); breaks even with clone() why??
	//	ef.add(then);
	//	ef.add(otherwise);
	//	Node &node = ef["then"];// debug
	Node &node = ef[2];// debug
	analyzed[ef.hash()] = true;
	return ef;
}

List<String> collectOperators(Node &expression) {
	List<String> operators;
	String previous;
	for (Node &op: expression) {
		String &name = op.name;
		if (not name)continue;
//		name = normOperator(name);// times => * aliases
		if (name.endsWith("="))// += etc
			operators.add(name);
		else if (prefixOperators.has(name)) {
			if (name == "-" and is_operator(previous.codepointAt(0)))
				operators.add(name + "…");//  -2*7 ≠ 1-(2*7)!
			else
				operators.add(name);
		}
//		WE NEED THE RIGHT PRECEDENCE NOW! -2*7 ≠ 1-(2*7)! or is it? √-i (i FIRST)  -√i √( first)
//		else if (prefixOperators.has(op.name+"…"))// and IS_PREFIX
//			operators.add(op.name+"…");
		else if (suffixOperators.has(name))
			operators.add(name);
		else if (operator_list.has(name))
			//			if (op.name.in(operator_list))
			operators.add(name);
//else if (suffixOperators.has(op.name+"…"))
//	operators.add(op.name);
		//		if (op.name.in(function_list))
		//			operators.add(op.name);
		//		if (op.name.in(functor_list))
		//			operators.add(op.name);
		previous = name;
		if (contains(import_keywords, (chars) name.data))
			break;
	}
	auto by_precedence = [](String &a, String &b) { return precedence(a) > precedence(b); };
	operators.sort(by_precedence);
	return operators;
}

Node &groupFunctionCalls(Node &expressiona, String context);

bool isVariable(Node &node) {
	if (node.kind != reference and node.kind != key and !node.isSetter())
		return false;
	if (node.kind == strings)return false;
	return /*node.parent == 0 and*/ not node.name.empty() and node.name[0] >= 'A';// todo;
}

bool isPrimitive(Node &node) {
	Kind type = node.kind;
	if (type == longs or type == strings or type == reals or type == bools or type == arrays or type == buffers)
		return true;
	if (type == codepoints)// todo ...?
		return true;
	return false;
}

Map<String, Node> types;
const Node Long("Long", clazz);
const Node Double("Double", clazz);//.setType(type);
//const Node Double{.name="Double", .kind=classe};//.setType(type);
//const Node Double{name:"Double", kind:classe};//.setType(type);

// todo: see NodeTypes.h for overlap with numerical returntype integer …
void initTypes() {
	types.add("int", Long);// until we really need it
	types.add("long", Long);
	types.add("double", Double);
	types.add("float", Double);
	for (auto name: types)
		types[name].setType(clazz);
}


bool addLocal(const char *context, String name, Valtype valtype);

Node &groupTypes(Node &expression, const char *context) {
	// todo delete type declarations double x, but not double x=7
	// todo if expression.length = 0 and first.length=0 and not next is operator return ø
	if (types.size() == 0)initTypes();
	if (isType(expression)) {// double \n x,y,z  extra case :(
		if (expression.length > 0) {
			for (Node &typed: expression) {// double \n x = 4
				typed.setType(&types[expression.name]);
				addLocal(context, typed.name, mapType(typed));
			}
			expression.name = 0;// hack
			expression.kind = groups;
			return expression.flat();
		} else if (expression.next) {
			expression.next->type = expression.clone();
			return *expression.next;
		} else if (expression.length == 0) {
			return *getType(expression).clone();
		} else {
			//  type name as variable name!
			expression.type = getType(expression).clone();
		}
	}
	for (int i = 0; i < expression.length; i++) {
		Node &node = expression.children[i];
		if (not isType(node))continue;
		if (node.length > 0) {
			node = groupTypes(node, context);// double (x,y,z)
			continue;
		}
		static Node typeDummy;// todo: remove how?
		Node &typed = typeDummy;
//		if (node.next and not is_operator(node.next->name[0])) {
//			typed = *node.next;
		if (i < expression.length - 1 and not is_operator(expression.children[i + 1].name[0])) {
			typed = expression.children[i + 1];
		} else if (i > 1) {
			typed = expression.children[i - 1];
		} else {
#ifdef DEBUG
			error("Type without object: "s + node.serialize() + "\n" + node.Line());// may be ok
#else
			error("Type without object: "s+node.serialize());// may be ok
#endif
			typed = NIL;
		}
//			if (operator_list.has(typed.name))
//				continue; // 3.3 as int …
		auto aType = &types[node.name];

		if (typed.name == "as") { // danger edge cases!
			expression.remove(i - 1, i);
			expression.children[i - 2].type = aType;// todo bug, should be same as
			typed = expression.children[i - 2];
			typed.type = aType;
			continue;
		} else {
			expression.remove(i, i);
		}
		while (isPrimitive(typed) or
		       (typed.kind == reference and typed.length == 0)) {// BAD criterion for next!
			typed.type = aType;// ref ok because types can't be deleted ... rIgHt?
			if (typed.kind == reference or typed.isSetter())
				addLocal(context, typed.name, mapType(*aType));
			// HACK for double x,y,z => z.type=Double !
			if (i + 1 < expression.length)
				typed = expression[++i];
			else if (typed.next) typed = *typed.next;
				// else outer group types currently not supported ((double x) y z)
			else break;
		}
	}
//	if(isPrimitive(expression)
	return expression.flat(); // (1) => 1
}


// return: done?
bool addLocal(const char *context, String name, Valtype valtype) {
	if (name.empty()) {
		warn("empty reference in "s + context);
		return true;// 'done' ;)
	}
	// todo: kotlin style context sensitive symbols!
	if (builtin_constants.has(name))
		return true;
	if (globals.has(name))
		error(name + " already declared as global"s);
	if (isFunction(name))
		error(name + " already declared as function"s);
	if (not locals[context].has(name)) {
		locals[context].add(name);
		localTypes[context].add(valtype);
		return true;// added
	}
//#if DEBUG
	else {
		auto position = locals[context].position(name);
		auto oldType = localTypes[context][position];
		if (oldType == none or oldType == unknown_type)
			localTypes[context][position] = valtype;
		else if (oldType != valtype and valtype != void_block and valtype != voids and valtype != unknown_type) // trace
			warn("local in context %s already known "s % s(context) + name + " with type " + typeName(oldType) +
			     " now " + typeName(valtype));
		// ok, could be cast'able!
	}
//#endif
	return false;// already there
}

Node &groupSetter(String name, Node &body, String context) {
	Node *decl = new Node(name);//node.name+":={…}");
	decl->setType(assignment);
	decl->add(body.clone());// addChildren makes emitting harder
	if (not addLocal(context, name, mapType(body))) {
		int i = locals[context].position(name);
		localTypes[context][i] = mapType(body);// update type! todo: check if cast'able!
	}
	return *decl;
}

Node extractReturnTypes(Node decl, Node body);

Node &groupDeclarations(String &name, Node *return_type, Node modifieres, Node &arguments, Node &body) {
//	String &name = fun.name;
	if (name and not function_operators.has(name))
		declaredFunctions.add(name);

	Signature &signature = functionSignatures[name];// use Default
	signature.emit = true;// all are 'export'
	// decl->metas().add(left);// mutable x=7  todo: either reactivate meta or add type mutable?

	// todo : un-merge x=1 x:1 vs x:=it function declarations for clarity?
	if (setter_operators.has(name) or key_pair_operators.has(name)) {
		Node body = analyze(body, name);
		if (arguments.has("global"))
			globals.insert_or_assign(name, body.clone());
		return groupSetter(name, body, String());
	}

	List<Arg> args = groupFunctionArgs(name, arguments);
	List<String> &parameters = locals[name];// function context!
	for (Arg arg: args) {
		if (empty(arg.name))
			error("empty argument name");
		if (locals[name].has(arg.name))
			error("duplicate argument name: "s + arg.name);
		addLocal(name, arg.name, mapType(arg.type));
		signature.add(arg.type);// todo: arg type, or pointer
	}
	if (signature.size() == 0 and parameters.size() == 0 and body.has("it", false, 100)) {
		addLocal(name, "it", f64);
		signature.add(f64);// todo: any / smarti! <<<
	}

	body = analyze(body, name);// has to come after arg analysis!
	if (!return_type)
		return_type = extractReturnTypes(arguments, body).clone();
	signature.returns(*return_type);// explicit double sin(){} // todo other syntaxes+ multi
	Node &decl = *new Node(name);//node.name+":={…}");
	decl.setType(declaration);
	decl.add(body);
	return decl;
}

Node &groupDeclarations(Node &expression, const char *context) {
	auto first = expression.first();
	if (expression.length == 2 and isType(first.first()) and
	    expression.last().kind == objects) {// c style double sin() {}
		expression = groupTypes(expression, context);
		return groupDeclarations(first.name, first.type, NIL, first.values(), expression.last());
	}

	for (Node &node: expression) {
		String &op = node.name;
		if (isPrimitive(node) and node.isSetter()) {
			if (globals.has(op)) {
				warn("Cant set globals yet!");
				continue;
			}
			addLocal(context, op, mapType(node));
			if (node.length >= 2)
				info("c-style function?");
			else
				continue;
		}
		if (node.kind == reference or (node.kind == key and isVariable(node))) {// only constructors here!
			if (not globals.has(op) and not isFunction(node))
				addLocal(context, op, unknown_type);
			continue;
		}// todo danger, referenceIndices i=1 … could fall through to here:
		if (node.kind != declaration and not declaration_operators.has(op))
			continue;
		if (op.empty())continue;
		if (op == "=") continue; // handle assignment via groupOperators !
		if (op == "::=") continue; // handle globals assignment via groupOperators !
		// todo: public export function jaja (a:num …) := …

		// BEGINNING OF Declaration ANALYSIS
		Node left = expression.to(node);// including public… + ARGS! :(
		Node rest = expression.from(node); // body
		String name = extractFunctionName(left);
		if (left.length == 0 and not declaration_operators.has(node.name))
			name = node.name;// todo: get rid of strange heuristics!
		if (node.length > 1) {
			// C style double sin(x) {…} todo: fragile criterion!! also why is body not child of sin??
			name = node.first().name;
			left = node.first().first();// ARGS
			rest = node.last();
			if (rest.kind == declaration)rest = rest.values();
			locals[context].remove(name);// not a variable!
		}
////			todo i=1 vs i:=it*2  ok ?

		if (name.empty())
			continue;
		if (isFunction(name)) {
			node.kind = call;
			continue;
		}
//			error("Symbol already declared as function: "s + name);
		// if (locals[context].has(name)) // todo double := it * 2 ; double(4)
//				error("Symbol already declared as variable: "s + name);
//			if (isImmutable(name))
//				error("Symbol declared as constant or immutable: "s + name);
		return groupDeclarations(name, 0, left, left, rest);
	}
	return expression;
}

Node extractReturnTypes(Node decl, Node body) {
	return Double;// Long;// todo
}

bool hasFunction(Node &n) {
	for (Node &child: n) {
		if (isFunction(child))
			return true;
	}
	return false;
}

bool isVariable(String name, String context0) {
	if (globals.has(name))return true;
	String context = "main";// context0.name;// todo find/store proper enclosing context of expression
	if (locals[context].has(name))return true;
	return false;
}

bool isPrefixOperation(Node &node, Node &lhs, Node &rhs);

String &checkCanonicalName(String &name);

List<Module> merge_modules;// from (automatic) import statements e.g. import math; use log; …
Map<String, bool> module_done;

// outer analysis 3 + 3  ≠ inner analysis +(3,3)
// maybe todo: normOperators step (in angle, not wasp!)  3**2 => 3^2
Node &groupOperators(Node &expression, String context = "main") {
	if (analyzed.has(expression.hash()))
		return expression;
	List<String> &localContext = locals[context];// todo: merge into Local object! :
	List<Valtype> &localContextTypes = localTypes[context];
	List<String> operators = collectOperators(expression);
	String last = "";
	int last_position = 0;

	if (expression.kind == Kind::operators) {
		if (expression.name == "include") {
			warn(expression.serialize());
			Node &file = expression.values();
//			Node &file = expression.from(1);
			// todo: properly merge, select appropriate functions …
			if (not module_done.has(file.name)) {
				Module import = read_wasm(file.name);
				merge_modules.add(import);// merging binary wasm segments in emit
				module_done.insert_or_assign(file.name, true);
			}
			return *new Node();
//			expression.clear();
//			return expression;
		}
//		else todo("ungrouped dangling operator");
	}

	for (String &op: operators) {
		if (op == "else")continue;// handled in groupIf
		if (op == "-")
			debug = true;
		if (op == "-…") op = "-";// precedence hack
		if (op == "%")functionSignatures["modulo_double"].is_used = true;
		if (op == "%")functionSignatures["modulo_float"].is_used = true;
		if (op == "include")todo("include again!?");
		if (op != last) last_position = 0;
		bool fromRight = rightAssociatives.has(op) or isFunction(op);
		fromRight = fromRight || (prefixOperators.has(op) and op != "-"); // !√!-1 == !(√(!(-1)))
		int i = expression.index(op, last_position, fromRight);
		if (i < 0) {
			if (op == "-" or op == "‖") //  or op=="?"
				continue;// ok -1 part of number, ‖3‖ closing ?=>if
			i = expression.index(op, last_position, fromRight);// try again for debug
			expression.print();
			error("operator missing: "s + op);
		}
		// we can't keep references here because expression.children will get mutated later via replace(…)
		Node node = expression.children[i];
		if (node.length)continue;// already processed
		Node next = expression.children[i + 1];
		next = analyze(next, context);
		Node prev = expression.children[i - 1];
		if (i == 0)prev = NIL;
		String &name = checkCanonicalName(op);

		if (name == "^" or name == "^^" or name == "**") {// todo NORM operators earlier!
			functionSignatures["pow"].is_used = true;
			functionSignatures["powd"].is_used = true;
			functionSignatures["powi"].is_used = true;
			functionSignatures["powf"].is_used = true;
		}
		if (isPrefixOperation(node, prev, next)) {// ++x -i
			// PREFIX Operators
			isPrefixOperation(node, prev, next);
			node.kind = Kind::operators;// todo should have been parsed as such!
			if (op == "-")//  -i => -(0 i)
				node.add(new Node(0));// todo: use f64.neg
			node.add(next);
			expression.replace(i, i + 1, node);
		} else {
			prev = analyze(prev, context);
			if (suffixOperators.has(name)) { // x²
				// SUFFIX Operators
				if (name == "ⁿ") functionSignatures["pow"].is_used = true;
				if (i < 1)error("suffix operator misses left side");
				node.add(prev);
				if (name == "²") {
					node.add(prev);
					node.name = "*"; // x² => x*x
				}
//				analyzed.insert_or_assign(node.hash(), true);
				expression.replace(i - 1, i, node);
				i--;
			} else if (name.in(function_list)) {// handled above!
				while (i++ < node.length)
					node.add(expression.children[i]);
				expression.replace(i, node.length, node);
			} else if (isFunction(next)) { // 3 + double 8
				Node &rest = expression.from(i + 1);
				Node &args = analyze(rest, context);
				node.add(prev);
				node.add(args);
				expression.replace(i - 1, i + 1, node);// replace ALL REST
				expression.remove(i, -1);

			} else {
				//#ifndef RUNTIME_ONLY
				// ways to set type:
				/*
				 * int x
				 * x:int
				 * x=7  needs pre-evaluation of rest!!!
				 * */
				auto var = prev.name;
				if (name.endsWith("=") and not name.startsWith("::") and prev.kind == reference) {
					// todo can remove hack?
					// x=7 and x*=7
					if (addLocal(context, var, mapType(next))) {
						if (name.length > 1 and name.endsWith("=")) // x+=1 etc
							error("self modifier on unknown reference "s + var);
					} else {
						// variable is known but not typed yet, or type again?
						int position = localContext.position(var);
						if (localContextTypes[position] == unknown_type)// todo 'none' ? default `var i` is int32???
							localContextTypes[position] = mapType(next);
						// TODO  pre-evaluation of rest!!! keep old type?
					}
				}
				//#endif
				node.add(prev);
				node.add(next);

				if (name == "::=") {
					if (prev.kind != reference)error("only references can be assigned global (::=)"s + var);
//					if(locals[context].has(prev.name))error("global already known as local "s +prev.name);// let's see what happens;)
					if (globals.has(var))error("global already set "s + var);// todo reassign ok if …
//					globals[prev.name] = &next;// don't forget to emit next as init expression!
					globals[var] = next.clone();// don't forget to emit next as init expression!
					// globalTypes[] set in globalSection, after emitExpression
				} else if (op.length > 1 and op.endsWith("="))
					// Complicated way to express *= += -= … self assignments
					if (op[0] != '=' and op[0] != '!' and op[0] != '?' and op[0] != '<' and op[0] != '>') {
						// *= += etc
						node.name = String(op.data[0]);
						Node *setter = prev.clone();
//					setter->setType(assignment); //
						setter->value.node = node.clone();
						node = *setter;
					}
				if (node.name == "?")
					node = groupIf(node, context);// consumes prev and next
//				analyzed.add(node.hash(), true);
				expression.replace(i - 1, i + 1, node);
			}
		}
		last_position = i;
		last = op;
	}
	return expression;
}

String &checkCanonicalName(String &name) {
	if (name == "**")warn("The power operator in angle is simply '^' : 3^2=9.");// todo: alias warning mechanism
	if (name == "^^")warn("The power operator in angle is simply '^' : 3^2=9. Also note that 1 xor 1 = 0");
	if (name == "||")warn("The disjunction operator in angle is simply 'or' : 1 or 0 = 1");
	if (name == "&&")warn("The conjunction operator in angle is simply 'and' : 1 and 1 = 1");
	return name;
}


// √π -i ++j !true … not delete(x)
bool isPrefixOperation(Node &node, Node &lhs, Node &rhs) {
	if (prefixOperators.has(node.name)) {
//		if (infixOperators.has(node.name) or suffixOperators.has(node.name)) {
		if (lhs.kind == reference)return false; // i++
		if (isPrimitive(lhs))return false; // 3 -1
		if (lhs.kind == operators) return lhs.length == 0;
		if ((lhs.isEmpty() or lhs.kind == operators) and lhs.name != "‖")
			return true;
		return false;
	}
	return false;
}

Node &groupFunctionCalls(Node &expressiona, String context) {
	if (expressiona.kind == declaration)return expressiona;// handled before
	if (isFunction(expressiona)) {
		expressiona.setType(call, false);
		if (not functionSignatures.has(expressiona.name))
			error("missing import for function "s + expressiona.name);
//		if (not expressiona.value.node and arity>0)error("missing args");
		functionSignatures[expressiona.name].is_used = true;
	}
//	Node &expressiona = *expressiona.clone();
	for (int i = 0; i < expressiona.length; ++i) {
//	for (int i = expressiona.length; i>0; --i) {
		Node &node = expressiona.children[i];
		String &name = node.name;
		if (name == "if") // kinda functor
		{
			auto args = expressiona.from("if");
			Node &iff = groupIf(node.length > 0 ? node.add(args) : args, context);
			int j = expressiona.lastIndex(iff.last().next) - 1;
			if (i == 0 and j == expressiona.length - 1)return iff;
			if (j > i)
				expressiona.replace(i, j, iff);// todo figure out if a>b c d e == if(a>b)then c else d; e boundary
			continue;
		}
		if (name == "while") {
			// todo: move into groupWhile
			if (node.length == 2) {
				node[0] = analyze(node[0], context);
				node[1] = analyze(node[1], context);
				continue;// all good
			}
			if (node.length == 1) {// while()… or …while()
				node[0] = analyze(node[0], context);
				Node then = expressiona.from("while");// todo: to closer!?
				int remaining = then.length;
				node.add(analyze(then, context).clone());
				expressiona.remove(i + 1, i + remaining);
				continue;
			} else {
				Node &iff = groupWhile(expressiona.from("while"), context);// todo: sketchy!
				int j = expressiona.lastIndex(iff.last().next) - 1;// huh?
				if (j > i)expressiona.replace(i, j, iff);
			}
		}
		if (isFunction(node)) // needs preparsing of declarations!
			node.kind = call;
		if (node.kind != call)
			continue;

		if (not functionSignatures.has(name))// todo load lib!
			error("missing import for function "s + name);
		if (not functionSignatures.has(name))// todo load lib!
			error("missing import for function "s + name);
		functionSignatures[name].is_used = true;


		int minArity = functionSignatures[name].size();// todo: default args!
		int maxArity = functionSignatures[name].size();

		if (node.length > 0) {
//			if minArity == …
			node = analyze(node.flat(), context);//  f(1,2,3) vs f([1,2,3]) ?
			continue;// already done how
		}
		if (minArity == 0)continue;
		Node rest;
		if (i < expressiona.length - 1 and expressiona[i + 1].kind == groups) {// f(x)
			// todo f (x) (y) (z)
			// todo expressiona[i+1].length>=minArity
			rest = expressiona[i + 1];
			if (rest.length > 1)
				rest.setType(expression);
			Node args = analyze(rest, context);
			node.add(args);
			expressiona.remove(i + 1, i + 1);
			continue;
		}
		rest = expressiona.from(i + 1);
		int arg_length = rest.length;
		if (rest.kind == reference and not arg_length) arg_length = 1;
		if (arg_length > 1)
			rest.setType(expression);// SUUURE?
		if (rest.kind == groups or rest.kind == objects)// and rest.has(operator))
			rest.setType(expression);// todo f(1,2) vs f(1+2)
//		if (hasFunction(rest) and rest.first().kind != groups)
//			if (name != "id")// stupid but true id(x)+id(y)==id(x+id(y))
//				error("Ambiguous mixing of functions `ƒ 1 + ƒ 1 ` can be read as `ƒ(1 + ƒ 1)` or `ƒ(1) + ƒ 1` ");
		if (rest.first().kind == groups)
			rest = rest.first();
		// per-function precedence does NOT really increase readability or bug safety
		if (rest.value.data) {
			maxArity--;// ?
			minArity--;
		}
		if (arg_length < minArity)
			error("missing arguments for function %s, given %d < expected %d. defaults and currying not yet supported"s % name % arg_length % minArity);
		else if (arg_length == 0 and minArity > 0)
			error("missing arguments for function %s, or to pass function pointer use func keyword"s % name);
//		else if (rest.first().kind == operators) { // random() + 1 == random + 1
//			// keep whole expressiona for later analysis in groupOperators!
//			return expressiona;
//		} else if (arg_length >= maxArity) {
		Node &args = analyze(rest, context);// todo: could contain another call!
		node.add(args);
		if (rest.kind == groups)
			expressiona.remove(i + 1, i + 1);
		else
			expressiona.remove(i + 1, i + arg_length);
//		} else
//			todo("missing arity match case");
	}
	return expressiona;
}

// todo: un-adhoc this!
Node &groupWhile(Node n, String context) {
	if (n.length == 0 and !n.value.data)
		error("no if condition given");
	if (n.length == 1 and !n.value.data)
		error("no if block given");

	Node &condition = n.children[0];
	Node then;
	if (n.length == 0) then = n.values();
	if (n.length == 1) {
		if (n.next)
			then = *n.next;
			//		else if(previous)
//			then = previous
		else
			error("missing block for while statement");// should be in parser/analyzer or carry over code pointer!
	}
	if (n.length > 0) then = n[1];
	if (n.length >= 2 and !n.value.data) {
//		return n; // all good!
		condition = n[0];
		then = n[1];
	}

	// todo: UNMESS how? UNMESS by applying operator ":" first a/r grouping in valueNode
	if (n.has(":") /*before else: */) {
		condition = n.to(":");
		then = n.from(":");
	} else if (condition.has(":")) {// as child
		then = condition.from(":");
	}
	if (n.has("do")) {
		condition = n.to("do");
		then = n.from("do");
	}
	if (then.has("do"))
		then = n.from("do");

	if (condition.value.data and !condition.next)
		then = condition.values();
	if (condition.kind == reference) {
		// find better condition todo HOW TO UNMESS??
		for (Node &child: n) {
			if (child.kind == groups or child.kind == objects) {// while x y z {}
				condition = n.to(child);
				then = child;
				break;
			}
		}
	}

	Node *whilo = new Node("while");// regroup cleanly
	Node &ef = *whilo;
	ef.kind = expression;// todo no longer functor?
	//	ef.kind = ifStatement;
	//	if (condition.length > 0)condition.setType(expression);// so far treated as group! todo: expression should be ok even if it's group!
//	if (then.length > 0)then.setType(expression);// NO! it CAN BE A GROUP!, e.g. while(i++){log(1);put(2);}
//	ef.add(analyze(condition).clone());
//	ef.add(analyze(then).clone());
//	ef.length = 2;
	ef["condition"] = analyze(condition, context);
	ef["then"] = analyze(then, context);
	analyzed[ef.hash()] = 1;
	return ef;
}


Node &analyze(Node &node, String context) {
	long hash = node.hash();
	if (analyzed.has(hash))
		return node;

	// group: {1;2;3} ( 1 2 3 ) expression: (1 + 2) tainted by operator
	Kind type = node.kind;
	String &name = node.name;


	if (type == functor) {
		if (name == "while")return groupWhile(node, context);
		if (name == "if")return groupIf(node, context);
		if (name == "?")return groupIf(node, context);
	}
	if ((type == expression and not name.empty())) {
		addLocal(context, name, int32);//  todo deep type analysis x = π * fun() % 4
	}
	if (type == key) {
		if (node.value.node /* i=ø has no node */)
			node.value.node = analyze(*node.value.node, context).clone();
		if (node.length > 0) {
			// (double x, y)  (while x) : y
			auto first = node.first().first();
			if (isType(first))
				return groupTypes(node, context);
			else if (first.name == "while")
				return groupWhile(node, context);
			else if (first.name == "if")
				return groupIf(node, context);
			else if (node.length > 1)
				error("unknown key expression: "s + node.serialize());
		}
		addLocal(context, name, node.value.node ? mapType(*node.value.node) : none);
	}
	if (isPrimitive(node)) {
		if (isVariable(node))
			addLocal(context, name, mapType(node));
		return node;// nothing to be analyzed!
	}

	bool is_function = isFunction(node);
	//todo merge/clean
	if (type == operators or type == call or is_function) {
		if (is_function)node.kind = call;
		Node &grouped = groupOperators(node, context);// outer analysis id(3+3) => id(+(3,3))
		if (grouped.length > 0)
			for (Node &child: grouped) {// inner analysis while(i<3){i++}
				if (&child == 0)continue;
				child = analyze(child);// REPLACE with their ast
			}
		if (functionSignatures.has(name))
			functionSignatures[name].is_used = true;
		return grouped;
	}

	Node &groupedTypes = groupTypes(node, context);
	if (isPrimitive(node)) return node;
	Node &groupedDeclarations = groupDeclarations(groupedTypes, context);
	Node &groupedFunctions = groupFunctionCalls(groupedDeclarations, context);
	Node &grouped = groupOperators(groupedFunctions, context);
	if (analyzed[grouped.hash()])return grouped;// done!
	analyzed.insert_or_assign(grouped.hash(), 1);
	if (type == groups or type == objects or type == expression) {
		// children analyzed individually, not as expression WHY?
		if (grouped.length > 0)
			for (Node &child: grouped) {
				child = analyze(child, context);// REPLACE ref with their ast ok?
			}
	}
	return grouped;
//	return *grouped.clone();// why?? where is leak?
}


int run_wasm_file(chars file) {
	let buffer = load(String(file));
#if RUNTIME_ONLY
	error("RUNTIME_ONLY");
	return -1;
#else
	prepareContext();
	return run_wasm((bytes) buffer.data, buffer.length);
#endif
}


void preRegisterSignatures() {
	// ORDER MATTERS: will be used for functionIndices later!
	globals.insert_or_assign("π", new Node(3.1415926535897932384626433));// todo: if used
	//	functionSignatures.insert_or_assign("put", Signature().add(pointer).returns(voids));
// todo: remove all as they come via wasp.wasm log.wasm etc
	functionSignatures.insert_or_assign("atoi0", Signature().add(charp).returns(int32));// todo int64
	functionSignatures.insert_or_assign("strlen0", Signature().add(charp).returns(int32));// todo int64
	functionSignatures.insert_or_assign("malloc", Signature().add(int64).returns(int64));
	functionSignatures.insert_or_assign("okf", Signature().add(float32).returns(float32));
	functionSignatures.insert_or_assign("okf5", Signature().add(float32).returns(float32));
	functionSignatures.insert_or_assign("pow", Signature().import().add(float64).add(float64).returns(float64));
//	functionSignatures.insert_or_assign("log", Signature().import().add(float32).returns(float32));
	functionSignatures.insert_or_assign("log", Signature().import().add(float64).returns(float64));
	functionSignatures.insert_or_assign("powd", Signature().import().add(float64).add(float64).returns(float64));
	functionSignatures.insert_or_assign("powi", Signature().import().add(int32).add(int32).returns(int64));
	functionSignatures.insert_or_assign("powf", Signature().import().add(float32).add(float32).returns(float32));

//	if (functionSignatures.has("logs"))
//		return;// already imported runtime!

	functionSignatures.insert_or_assign("puti", Signature().import().add(int32).returns(voids));
	functionSignatures.insert_or_assign("putf", Signature().import().add(float32).returns(voids));
	functionSignatures.insert_or_assign("putd", Signature().import().add(float64).returns(voids));
	//	functionSignatures.insert_or_assign("powl", Signature().import().add(int64).add(int64).returns(int64));
	//	js_sys::Math::pow  //pub fn pow(base: f64, exponent: f64) -> f64
	functionSignatures.insert_or_assign("puts", Signature().import().add(charp).returns(voids));
//	functionSignatures.insert_or_assign("puts", Signature().import().add(charp).returns(int32));
	functionSignatures.insert_or_assign("not_ok", Signature().returns(voids));
	functionSignatures.insert_or_assign("ok", Signature().returns(int32));// todo why not rely on read_wasm again?
	functionSignatures.insert_or_assign("oki", Signature().add(int32).returns(int32));

//	functionSignatures.insert_or_assign("render", Signature().add(node).add(pointer).returns(none));
//	functionSignatures.insert_or_assign("render", Signature().runtime().add(node).returns(int32));
//functionSignatures.insert_or_assign("render", Signature().add(node).add(pointer).returns(int32));
	// todo: long + double !
	// imports
	functionSignatures["modulo_float"] = Signature().builtin().add(float32).add(float32).returns(float32);
	functionSignatures["modulo_double"].builtin().add(float64).add(float64).returns(float64);
	functionSignatures["square"] = Signature().add(i32t).returns(i32t).import();
//	functionSignatures["main"] = Signature().returns(i32t);;
	functionSignatures["main"] = Signature().returns(i64t); // ok in all modern environments~
	functionSignatures["print"] = functionSignatures["puts"];// todo: for now, later it needs to distinguish types!!
	functionSignatures["paint"].import().returns(voids);// paint surface
	functionSignatures.insert_or_assign("init_graphics", Signature().import().returns(pointer));// surface
//	functionSignatures["init_graphics"].import().returns(pointer);// BUUUUG!

	// TODO!!!
	// functionSignatures[ ] access is BROKEN!!! use functionSignatures.insert_or_assign so long!

//	if(functionSignatures["init_graphics"].return_type!=pointer)error("WWWAAA");
	// builtins
	functionSignatures["nop"] = Signature().builtin();
//	functionSignatures["id"] = Signature().add(i32t).returns(i32t).builtin();
	functionSignatures.insert_or_assign("id", Signature().add(i32t).returns(i32t).builtin());

	// library signatures are parsed in consumeExportSection() via demangle
	// BUT their return type is not part of name, so it needs to be hardcoded, if ≠ int32:
//	functionSignatures["concat"] = Signature().add(string).add(string).returns(string).runtime();// chars to be precise
	functionSignatures["concat"] = Signature().add(charp).add(charp).returns(charp).runtime();// chars to be precise
}

void clearContext() {
#ifndef RUNTIME_ONLY
	globals.clear();
	globals.setDefault(new Node());
	functionIndices.clear();
	functionIndices.setDefault(-1);
	locals.clear();
	locals.setDefault(List<String>());
	localTypes.clear();
	localTypes.setDefault(List<Valtype>());
	declaredFunctions.clear();
	functionSignatures.clear();
	functionSignatures.setDefault(Signature());
	preRegisterSignatures();
	analyzed.clear();// todo move much into outer analyze function!
	analyzed.setDefault(0);
	//	if(data.kind==groups) data.kind=expression;// force top level expression! todo: only if analyze recursive !
#endif
}

// 2MB Debug runtime needs 3 seconds in wasmtime! :(
// test with SMALL runtime!!
Node runtime_emit(String prog) {
#ifdef RUNTIME_ONLY
	printf("emit wasm not built into release runtime");
	return ERROR;
#endif
	clearContext();
	functionIndices.clear();
	functionIndices.setDefault(-1);
	Module runtime = read_wasm("wasp.wasm");
	preRegisterSignatures();
//	functionIndices["print"]=functionIndices["logs"]  print default is print(Node), KEEP IT!!
	Node charged = analyze(parse(prog));
	Code lib = emit(charged, &runtime, "main");// start already declared: main if not compiled/linked as lib
	lib.save("main.wasm");// partial wasm!
	functionIndices.clear();// no longer needed
	Module main = read_wasm("main.wasm");
	Code code = merge_wasm(runtime, main);
	code.save("merged.wasm");
	read_wasm("merged.wasm");
	long result = code.run();// todo parse stdout string as node and merge with emit() !
	clearContext();
	return smartNode(result);
}

// smart pointers returned if ABI does not allow multi-return, as in int main(){}

Node smartNode(long long smartPointer64) {
	if (!isSmartPointer(smartPointer64))
		return Node(smartPointer64);
	auto result = smartPointer64;
	if (smartPointer64 & double_mask_64 and not((smartPointer64 & negative_mask_64) == negative_mask_64)) {
		// todo rare cases, where doubles don't match 0x7F…
		double val = *(double *) &smartPointer64;
		return Node(val);
	}

	auto smart_pointer = result & 0xFFFFFFFF;// data part
	long long smart_type = result & 0xFFFFFFFF00000000;// type part
	if (smart_type == array_header_64 /* and abi=wasp */ ) {
		// smart pointer to smart array
		int *index = ((int *) wasm_memory) + smart_pointer;
		int kind = *index++;
		if (kind == array_header_32)
			kind = *index++;
		int len = *index++; // todo: leb128 vector later
		Node arr = Node();
//		arr.kind.value = kind;
		int pos = 0;
		while (len-- > 0) {
			auto val = index[pos++];
			arr.add(new Node(val));
		}
		arr.kind = objects;
//			check_eq(arr.length,len);
//			check(arr.type=…
		return arr;
	}
	if (smart_type == string_header_64) {
		// smart pointer for string
		return Node(((char *) wasm_memory) + smart_pointer);
	}
	breakpoint_helper
	error1("missing smart pointer type %x"s % smart_type + " “" + typeName(Type(smart_type)) + "”");
	return Node();
}

Node smartNode32(int smartPointer32) {
	auto result = smartPointer32;
	auto smart_pointer = result & 0x00FFFFFF;// data part
	if ((result & 0xF0000000) == array_header_32 /* and abi=wasp */ ) {
		// smart pointer to smart array
		int *index = ((int *) wasm_memory) + smart_pointer;
		int kind = *index++;
		if (kind == array_header_32)
			kind = *index++;
		int len = *index++; // todo: leb128 vector later
		Node arr = Node();
//		arr.kind.value = kind;
		int pos = 0;
		while (len-- > 0) {
			auto val = index[pos++];
			arr.add(new Node(val));
		}
		arr.kind = objects;
//			check_eq(arr.length,len);
//			check(arr.type=…
		return arr;
	}
	if ((result & 0xF0000000) == string_header_32 /* and abi=wasp */ ) {
		// smart pointer for string
		return Node(((char *) wasm_memory) + smart_pointer);
	}
	error1("missing smart pointer type "s + typeName(Type(smartPointer32)));
	return Node();
}

// todo dedup runtime_emit!
//Node emit(String code, ParseOptions options) {
Node emit(String code) {// emit and run!
//	if (code.endsWith(".wasm")){
//		auto filename = findFile(code);
//		return Node(run_wasm(filename));
//	}
	Node data = parse(code);
	debug_code = code;// global so we see when debugging
#ifdef RUNTIME_ONLY
#ifdef INTERPRETER
	return data.interpret();
#endif
	return data;
#else
	data.print();
	clearContext();
//	preRegisterSignatures();
	Node &charged = analyze(data);
	charged.print();
	Code binary;
//	if (options & no_main) // todo: lib_main!
//		binary = emit(charged, 0, 0);
//	else
	binary = emit(charged);
	for (Module &import: merge_modules) {
		Module prog = read_wasm(binary.data, binary.length);
		binary = merge_wasm(import, prog);
	}
//	code.link(wasp) more beautiful with multiple memory sections
	long result = binary.run();// check js console if no result
	return smartNode(result);
#endif
}


float function_precedence = 1000;

// todo!
// moved here so that valueNode() works even without Angle.cpp component for micro wasm module
chars function_list[] = {"abs", "norm", "square", "root", "put", "puts", "print", "printf", "println",
                         "log", "ln", "log10", "log2", "similar",
                         "putx", "putc", "id", "get", "set", "peek", "poke", "read", "write", 0, 0,
                         0};// MUST END WITH 0, else BUG
chars functor_list[] = {"if", "while", "go", "do", "until", 0};// MUST END WITH 0, else BUG


float precedence(Node &operater) {
	String &name = operater.name;
//	if (operater == NIL)return 0; error prone
	if (operater.kind == reals)return 0;//;1000;// implicit multiplication HAS to be done elsewhere!
	if (operater.kind == longs)return 0;//;1000;// implicit multiplication HAS to be done elsewhere!
	if (operater.kind == strings)return 0;// and empty(name)

	// todo: make live easier by making patterns only operators if data on the left
	if (operater.kind == patterns and operater.parent) return 98;// precedence("if") * 0.98
	//	todo why do groups have precedence again?
//	if (operater.kind == groups) return 99;// needs to be smaller than functor/function calls
	if (operater.name.in(function_list))return 999;// function call todo: remove here
	if (empty(name))return 0;// no precedence
	return precedence(name);
}


// like c++ here HIGHER up == lower value == more important
float precedence(String name) {
	if (eq(name, "abs"))return 0.08;
	if (eq(name, "‖"))return 0.10; // norms / abs

	if (eq(name, "."))return 0.5;
	if (eq(name, "of"))return 0.6;
	if (eq(name, "in"))return 0.7;
	if (eq(name, "from"))return 0.8;

	if (eq(name, "not"))return 1;
	if (eq(name, "¬"))return 1;
	if (eq(name, "-…"))return 1;// unary operators are immediate, no need for prescidence
	if (eq(name, "!"))return 1;
	if (eq(name, "√"))return 1;// !√1 √!-1
	if (eq(name, "^"))return 2;// todo: ambiguity? 2^3+1 vs 2^(x+1)
	if (eq(name, "**"))return 2;//
	if (eq(name, "^^"))return 2;// how did it work without??

	if (eq(name, "#"))return 3;// count
	if (eq(name, "++"))return 3;
//	if (eq(node.name, "+"))return 3;//
	if (eq(name, "--"))return 3;

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
	if (eq(name, "upto"))return 6.3;// range
	if (eq(name, "…"))return 6.3;
	if (eq(name, "..."))return 6.3;
	if (eq(name, ".."))return 6.3;// excluding range
	if (eq(name, "..<"))return 6.3;// excluding range
	if (eq(name, "<"))return 6.5;
	if (eq(name, "<="))return 6.5;
	if (eq(name, ">="))return 6.5;
	if (eq(name, ">"))return 6.5;
	if (eq(name, "≥"))return 6.5;
	if (eq(name, "≤"))return 6.5;
	if (eq(name, "≈"))return 6.5;
	if (eq(name, "=="))return 6.6;
	if (eq(name, "is"))return 6.6; // careful, use 'be' for := assignment
	if (eq(name, "eq"))return 6.6;
	if (eq(name, "equals"))return 6.6;
	if (eq(name, "is not"))return 6.6;// ambiguity: a == !b vs a != b
	if (eq(name, "isnt"))return 6.6;
	if (eq(name, "isn't"))return 6.6;
	if (eq(name, "equal"))return 10;
	if (eq(name, "≠"))return 10;
	if (eq(name, "!="))return 10;

	if (eq(name, "and"))return 7.1;
	if (eq(name, "&&"))return 7.1;
	if (eq(name, "&"))return 7.1;
	if (eq(name, "∧"))return 7.1;
	if (eq(name, "⋀"))return 7.1;


	if (eq(name, "xor"))return 7.2;
	if (eq(name, "^|"))return 7.2;
	if (eq(name, "⊻"))return 7.2;

	if (eq(name, "or"))return 7.2;
	if (eq(name, "||"))return 7.2;
	if (eq(name, "∨"))return 7.2;
	if (eq(name, "⋁"))return 7.2;
//	if (eq(name, "|"))return 7.2;// todo pipe special

	if (eq(name, ":"))return 7.5;// todo:
	if (eq(name, "?"))return 7.6;

	if (name.in(function_list))// f 1 > f 2
		return 8;// 1000;// function calls outmost operation todo? add 3*square 4+1


	if (eq(name, "⇒"))return 11; // lambdas
	if (eq(name, "=>"))return 11;
	if (eq(name, "::"))return 11;// todo lambda symbol? square = x :: x*x

//	if (eq(name, ":"))return 12;// construction
	if (eq(name, "="))return 12;// declaration
	if (eq(name, ":="))return 13;
	if (eq(name, "be"))return 13;// counterpart 'is' for ==
	if (eq(name, "::="))return 14; // globals setter
//	if (eq(name, "is"))return 13;// careful, could be == (6.6)

	if (eq(name, "else"))return 13.09;
	if (eq(name, "then"))return 13.15;
	if (eq(name, "if"))return 100;
	if (eq(name, "while"))return 101;
//	if (eq(name, "once"))return 101;
//	if (eq(name, "go"))return 101;
	if (name.in(functor_list))// f 1 > f 2
		return function_precedence;// if, while, ... statements calls outmost operation todo? add 3*square 4+1

	if (eq(name, "return"))return 1000;
	return 0;// no precedence
}