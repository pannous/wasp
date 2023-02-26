/**
 Demangles C++ function names mangled according to the IA64 C++ ABI

 This is a pretensious and cocky way to say that this file demangles function names
 mangled by GCC and Clang.

 Material used: https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling
 **/

/* Removes a mangled name from 'str', in the mangled name format
   Returns an object. The 'name' property is the name, the 'str' is the remainder of the string */

var lastType = "";

export // remove for index.html needs extension.mjs or "type": "module" in package.json
function demangle(name) {
    if (!isMangled(name)) return name;

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
        switch (process.ch) {
            case 'v':
                typeInfo.typeStr = ""; // void
                break;
            case 'w':
                typeInfo.typeStr = "wchar_t";
                break;
            case 'b':
                typeInfo.typeStr = "bool";
                break;
            case 'c':
                typeInfo.typeStr = "char";
                break;
            case 'a':
                typeInfo.typeStr = "signed char";
                break;
            case 'h':
                typeInfo.typeStr = "unsigned char";
                break;
            case 's':
                typeInfo.typeStr = "short";
                break;
            case 't':
                typeInfo.typeStr = "unsigned short";
                break;
            case 'i':
                typeInfo.typeStr = "int";
                break;
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
            case 'S': // "self" last type duplicate
                if (types.length > 0)
                    typeInfo = types[types.length - 1]
                // types.push(types[types.length-1]);
                else
                    typeInfo.typeStr = lastType;

                process = popChar(process.str);
                if (process.ch == '_' || process.ch == '0')
                    process = popChar(process.str);
                break
            case 'B': // _Z3absB7v160000d $abs[abi:v160000](double)()
                let bracket = popName(process.str);
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
                    const tname = popName(process.ch.concat(process.str));
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
    return functionname.concat("(" + typelist.join(', ') + ")").replace(/<, /g, "<")
        .replace(/<, /g, "<").replace(/, >/g, ">").replace(/, </g, "<");
}

var const_function = false
function popName(str) {
    /* The name is in the format <length><str> */

    let isLast = false;
    let namestr = "";
    let rlen = 0;
    const ostr = str;
    let isEntity = false;

    let c = str[0];
    if (c == 'N' && str[1] == 'K') {
        str = str.substr(2);
        const_function = true;
        rlen += 2;
    }

    let handled = false;

    while (!isLast) {
        /* This is used for decoding names inside complex namespaces
           Whenever we find an 'N' preceding a number, it's a prefix/namespace */
        let c = str[0];
        isLast = c != "N";
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
            if (c == "n") {
                namestr += "::operator new"
                rlen += 2
                handled = true;
            }
            if (c === "C") { // constructor String::String()
                namestr += "::" + namestr
                rlen += 2
                handled = true;
            }
            if (c === "I") {
                let generic = popName(str.substr(1)).name
                namestr += "<" + generic + ">::" + namestr
                rlen += generic.length + 5
                handled = true;
            }
            if (c === "p") {
                namestr += "::" + getOperatorName(str[1])
                rlen += 2
                handled = true;
            } else
                rlen -= 1;// ?
            break;
        }
        rlen += res[0].length + len;
        const strstart = str.substr(res[0].length);
        lastType = strstart.substr(0, len);
        namestr = namestr.concat(lastType);
        // if (!isLast && c!='I')
        //     namestr = namestr.concat("::");
        str = strstart.substr(len);
    }
    if (rlen > 0 && !handled) {
        let fname = popName(str).name;
        if (fname) {
            if (c == 'I') {
                namestr += fname + "::" + namestr; // List<String>::List hack
            } else
                namestr += "::" + fname; // String::empty
            rlen += fname.length + 2;
        }
    }
    if (isEntity) rlen += 2; // Take out the "E", the entity end mark
    let substr = ostr.substr(rlen);
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


// .cjs
// module.exports = {demangle}
// module.exports.default = demangle
// exports.default = demangle

// .mjs
// export const demangle = demangle;