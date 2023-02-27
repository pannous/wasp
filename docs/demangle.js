/**
 Demangles C++ function names mangled according to the IA64 C++ ABI

 This is a pretensious and cocky way to say that this file demangles function names
 mangled by GCC and Clang.

 Material used: https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling
 **/

/* Removes a mangled name from 'str', in the mangled name format
   Returns an object. The 'name' property is the name, the 'str' is the remainder of the string */

var lastType = "";
var baseType = "";
var const_function = false //  String::empty() const
let isLower = (c) => c >= 'a' && c <= 'z'
// let isTypeCode = c => "vwdufcCjSsPbNtLgDxXeEoOA".indexOf(c) >= 0

export // remove for index.html needs extension.mjs or "type": "module" in package.json
function demangle(name) {
    if (!isMangled(name)) return name;

    lastType = "";
    const_function = false //  String::empty() const

    /* Encoding is the part between the _Z (the "mangling mark") and the dot, that prefix
       a vendor specific suffix. That suffix will not be treated here yet */
    let indexOfDot = name.indexOf('.');
    const encoding = name.substr(2, (indexOfDot < 0) ? undefined : indexOfDot - 2);

    let fname = popName(encoding);
    let functionname = fname.name;
    let types = [];

    let template_count = 0;
    let template_types = [];

    // Process the types
    let str = fname.str;

    while (str.length > 0) {
        let process = popChar(str);
        /* The type info
           isBase -> is the type the built-in one in the mangler, represented with few letters?
           typeStr: the type name
           templateType: type info for the current template.
           The others are self-descriptive
        */
        let typeInfo = {
            isBase: true, typeStr: "", isConst: false, numPtr: 0,
            isRValueRef: false, isRef: false, isRestrict: false,
            templateStart: false, templateEnd: false,
            isVolatile: false, templateType: null
        };

        /* Check if we have a qualifier (like const, ptr, ref... )*/
        var doQualifier = true;
        while (doQualifier) {
            switch (process.ch) {
                case 'R':
                    typeInfo.isRef = true;
                    process = popChar(process.str);
                    break;
                case 'O':
                    typeInfo.isRValueRef = true;
                    process = popChar(process.str);
                    break;
                case 'r':
                    typeInfo.isRestrict = true;
                    process = popChar(process.str);
                    break;
                case 'V':
                    typeInfo.isVolatile = true;
                    process = popChar(process.str);
                    break;
                case 'K':
                    typeInfo.isConst = true;
                    process = popChar(process.str);
                    break;
                case 'P':
                    typeInfo.numPtr++;
                    process = popChar(process.str);
                    break;
                default:
                    doQualifier = false;
            }
        }

        /* Get the type code. Process it */
        let typeName = getTypeName(process.ch);
        if (typeName)
            typeInfo.typeStr = typeName;
        else switch (process.ch) {
            case 'p':
                process = popChar(process.str);
                typeInfo.typeStr = getOperatorName(process.ch);
                break
            case 'D':
                let t = popChar(process.str);
                switch (t.ch) {
                    case 'i':
                        typeInfo.typeStr = "char32_t";
                        break;
                    default:
                        typeInfo.typeStr = "char???";
                }
                break;
            case 'S': // "self" type duplicate
                if (types.length > 0 && str[1] == '0')
                    typeInfo = types[types.length - 1]
                else if (str[1] == '_')
                    typeInfo.typeStr = baseType;
                else
                    typeInfo.typeStr = lastType;

                process = popChar(process.str);
                if (process.ch == '_' || process.ch == '0')
                    process = popChar(process.str);
                break
            case 'B': // _Z3absB7v160000d $abs[abi:v160000](double)()
                let bracket = popName(process.str, false);
                process.str = bracket.str;
                break;
            case 'I':
                // Template open bracket (<)
                var ty = typeInfo
                if (types.length > 0)
                    ty = types[types.length - 1]
                // types.push();
                ty.templateStart = true;
                template_types.push(ty);
                template_count++;
                break;
            case 'E':
                if ((template_count <= 0)) {
                    str = process.str;
                    // repeat type not here
                    // typeInfo.typeStr = typeInfo.typeStr.concat("$$$");
                    break;
                }
                // Template closing bracket (>)
                typeInfo.templateEnd = true;
                template_count--;
                typeInfo.templateType = template_types[template_count];
                template_types = template_types.slice(0, -1);
                break;
            case 'j':
                typeInfo.typeStr = "unsigned int";
                break;
            case 'l':
                typeInfo.typeStr = "long";
                break;
            case 'm':
                typeInfo.typeStr = "unsigned long";
                break;
            case 'x':
                typeInfo.typeStr = "long long";
                break;
            case 'y':
                typeInfo.typeStr = "unsigned long long";
                break;
            case 'n': // BigInt, operator new in other context
                typeInfo.typeStr = "__int128";
                break;
            case 'o':
                typeInfo.typeStr = "unsigned __int128";
                break;
            case 'f':
                typeInfo.typeStr = "float";
                break;
            case 'd':
                typeInfo.typeStr = "double";
                break;
            case 'e':
                typeInfo.typeStr = "long double";
                break;
            case 'g':
                typeInfo.typeStr = "__float128";
                break;
            case 'z':
                typeInfo.typeStr = "...";
                break;
            /* No type code. We have a type name instead */
            default: {
                let number = parseInt(process.ch, 10);
                if (!isNaN(number) || process.ch == 'N') {

                    if (process.ch == 'N' && process.str[0] == 'K') {
                        process = popChar(process.str);
                        typeInfo.isConst = true;
                    }

                    // It's a custom type name
                    const tname = popName(process.ch.concat(process.str), false);
                    typeInfo.typeStr = typeInfo.typeStr.concat(tname.name);
                    process.str = tname.str;
                }
            }
                break;
        }
        if (typeInfo.typeStr)
            types.push(typeInfo);
        str = process.str;
    }

    /* Create the string representation of the type */
    const typelist = types.map((t) => {
        let typestr = "";
        if (t.isConst) typestr = typestr.concat("const ");
        if (t.isVolatile) typestr = typestr.concat("volatile ");
        typestr = typestr.concat(t.typeStr);
        if (t.templateStart) typestr = typestr.concat("<");
        if (t.templateEnd) typestr = typestr.concat(">");

        if (!t.templateStart) {
            if (t.isRef) typestr = typestr.concat("&");
            if (t.isRValueRef) typestr = typestr.concat("&&");
            for (let i = 0; i < t.numPtr; i++) typestr = typestr.concat("*");
            if (t.isRestrict) typestr = typestr.concat(" __restrict");
        }

        if (t.templateType) {
            if (t.templateType.isRef) typestr = typestr.concat("&");
            if (t.templateType.isRValueRef) typestr = typestr.concat("&&");
            for (let i = 0; i < t.templateType.numPtr; i++) typestr = typestr.concat("*");
        }
        return typestr;
    });

    /* stupid shortcut to fix templates and make it fast Without that, we would need to complicate the code
       What it does is remove the commas where we would have the angle brackets for the templates */
    let result = functionname.concat("(" + typelist.join(', ') + ")");
    result = result.replace(/<, /g, "<").replace(/<, /g, "<")
    result = result.replace(/, >/g, ">").replace(/, </g, "<");
    if (const_function) result += " const";
    return result
}

function popName(str, operators = true) {
    /* The name is in the format <length><str> */

    let isLast = false;
    let namestr = "";
    let rlen = 0;
    const ostr = str;
    let isEntity = false;
    let handled = false;
    let isClass = false


    let c = str[0];
    if (c == 'N' && str[1] == 'K') {
        str = str.substr(2);
        const_function = true;
        // isClass = true;
        // isEntity= true;
        operators = true
        rlen += 2;
    }


    while (!isLast) {
        /* This is used for decoding names inside complex namespaces
           Whenever we find an 'N' preceding a number, it's a prefix/namespace */
        let c = str[0];

        if (c != "N" && c != 'K') {
            if (!const_function)
                handled = true;
            isLast = true;
        } else {
            isClass = true;
        }

        /* IF NOT IN popName St means std:: in the mangled string
           This std:: check is for inside the name, not outside,
           unlike the one in the demangle function
         */
        // if (str.substr(1, 2) === "St") {
        //     // namestr = namestr.concat("std::");
        //     str = str.replace("St", "");
        //     rlen++;
        // }

        /* This is used for us to know we'll find an E in the end of this name
           The E marks the final of our name
        */
        isEntity = isEntity || !isLast;
        if (!isLast) str = str.substr(1);
        const res = /(\d*)/.exec(str);
        const len = parseInt(res[0], 10);
        if (isNaN(len)) {
            if (c === "C") { // constructor String::String()
                namestr += namestr.substr(0, namestr.length - 2)
                rlen += 2
                handled = true;
                isClass = false
            }
            if (c === "I") { // generic
                isClass = false
                let generic = popName(str.substr(1)).name
                namestr = namestr.substr(0, namestr.length - 2)
                namestr += "<" + generic + ">::" + namestr
                rlen += generic.length + 5
                handled = true;
                break
            }
            if (c == "c") {// cast
                // _ZNK6StringcvbEv
                // function $String::operator bool() const()
                namestr += " " // todo bool()
                rlen += 4
                handled = true;
            } else if (isLower(c) && operators) {
                // !isTypeCode(c) ambivalent
                namestr += getOperatorName(c + str[1])
                rlen += 2
                handled = true;
            } else
                rlen -= 1;// ?
            break;
        }
        rlen += res[0].length + len;
        const strstart = str.substr(res[0].length);
        lastType = strstart.substr(0, len);
        if (isClass) {
            baseType = lastType
            namestr += lastType + "::";
            isClass = false;
        } else
            namestr = namestr.concat(lastType);

        // if (!isLast && c!='I')
        //     namestr = namestr.concat("::");
        str = strstart.substr(len);
    }
    if (rlen > 0 && !handled) {
        let popped = popName(str, operators);
        let fname = popped.name;
        if (fname) {
            namestr += "::" + fname; // String::empty
            return {name: namestr, str: popped.str};
        }
    }
    if (isEntity) rlen += 2; // Take out the "E", the entity end mark
    let substr = ostr.substr(rlen);
    if (substr.startsWith("E"))
        substr = substr.substr(1)
    return {name: namestr, str: substr};
}

function popChar(str) {
    return {ch: str[0], str: str.slice(1)};
}

/* Check if the name passed is a IA64 ABI mangled name */
function isMangled(name) {
    return name.startsWith("_Z");
}

function getOperatorName(ch) {
    switch (ch) {
        case 'nw':
            return 'operator new'
        case 'cv':
            return 'operator ' // cast … bool() …
        case 'dl':
            return 'operator delete'
        case 'pl':
            return 'operator+'
        case 'na':
            return "operator new[]"
        case 'da':
            return "operator delete[]"
        case 'aw':
            return "operator co_await"
        case 'ng':
            return "operator-" // negation
        case 'mi':
            return "operator-" // minus
        case 'ad':
            return "operator&"
        case 'de':
            return "operator*"
        case 'co':
            return "operator~"
        case 'ml':
            return "operator*"
        case 'dv':
            return "operator/"
        case 'rm':
            return "operator%"
        case 'an':
            return "operator&"
        case 'or':
            return "operator|"
        case 'eo':
            return "operator^"
        case 'mI':
            return "operator-="
        case 'mL':
            return "operator*="
        case 'dV':
            return "operator/="
        case 'rM':
            return "operator%="
        case 'aN':
            return "operator&="
        case 'oR':
            return "operator|="
        case 'eO':
            return "operator^="
        case 'ls':
            return "operator<<"
        case 'rs':
            return "operator>>"
        case 'lS':
            return "operator<<="
        case 'rS':
            return "operator>>="
        case 'ne':
            return "operator!="
        case 'lt':
            return "operator<"
        case 'le':
            return "operator<="
        case 'ge':
            return "operator>="
        case 'ss':
            return "operator<=>" // spaceship comparison
        case 'nt':
            return "operator!"
        case 'aa':
            return "operator&&"
        case 'oo':
            return "operator||"
        case 'pp':
            return "operator++"
        case 'mm':
            return "operator--"
        case 'cm':
            return "operator,"
        case 'pm':
            return "operator->*"
        case 'pt':
            return "operator->"
        case 'cl':
            return "operator()"
        case 'qu':
            return "operator?"
        case 'pL':
            return 'operator+='
        case 'aS': // assign
            return 'operator='
        case 'eq': // eq
            return 'operator=='
        case 'gt':
            return 'operator>'
        case 'ix': // index
            return 'operator[]'
        case 'C':
            return 'operator->'
        case 'D':
            return 'operator*'
        case 'E':
            return 'operator++'
        case 'F':
            return 'operator--'
        case 'G':
            return 'operator-'
        case 'H':
            return 'operator+'
        case 'l':
            return 'operator+'
        case 'I':
            return 'operator&'
        case 'J':
            return 'operator->*'
        case 'K':
            return 'operator/'
        case 'L':
            return 'operator%'
        case 'M':
            return 'operator< '
        case 'N':
            return 'operator<='
        case 'O':
            return 'operator> '
        case 'P':
            return 'operator>='
        case 'Q':
            return 'operator,'
        case 'R':
            return 'operator()'
        case 'S':
            return 'operator~'
        case 'T':
            return 'operator^'
        case 'U':
            return 'operator|'
        case 'V':
            return 'operator&&'
        case 'W':
            return 'operator||'
        case 'X':
            return 'operator*='
        case 'Y':
            return 'operator+='
        case 'Z':
            return 'operator-='
        case '0':
            return 'constructor'
        case '1':
            return 'destructor'
        case '2':
            return 'operator new'
        case '3':
            return 'operator delete'
        case '4':
            return 'operator='
        case '5':
            return 'operator>>'
        case '6':
            return 'operator<<'
        case '7':
            return 'operator!'
        case '8':
            return 'operator=='
        case '9':
            return 'operator!='
        default:
            return 'operator???' + ch;
    }
}


function getTypeName(ch) {
    switch (ch) {
        case 'v':
            return ""; // void
        case 'i':
            return "int";
        case 'w':
            return "wchar_t";
        case 'd':
            return "double";
        case 'u':
            return "unsigned int";
        case 'f':
            return "float";
        case 'c':
            return "char";
        case 'C':
            return "unsigned char";
        case 'j':
            return "unsigned int";
        case 's':
            return "short";
        case 'P':
            return "unsigned short";
        case 'b':
            return "bool";
        case 'N':
            return "__int128";
        case 't':
            return "unsigned __int128";
        case 'L':
            return "long";
        case 'g':
            return "unsigned long";
        case 'D':
            return "long double";
        case 'x':
            return "long long";
        case 'X':
            return "unsigned long long";
        case 'e':
            return "...";
        case 'A':
            return "__float128";
        default:
            return "";
    }
}


// .cjs
// module.exports = {demangle}
// module.exports.default = demangle
// exports.default = demangle

// .mjs
// export const demangle = demangle;