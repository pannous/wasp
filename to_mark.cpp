// stringify() is only defined on the static Mark API
// Mark stringify will not quote keys where appropriate
void stringify(auto obj, options) {
	"use strict";

	var indentStep, indentStrs, space, omitComma;

	void indent(num, noNewLine) {
		if (num >= indentStrs.length) { // expand the cached indent strs
			for (var i = indentStrs.length; i <= num; i++) {
				indentStrs[i] = indentStrs[i - 1] + indentStep;
			}
		}
		return noNewLine ? indentStrs[num] : "\n" + indentStrs[num];
	}

	// option handling
	if (options) {
		omitComma = options.omitComma;
		space = options.space;
		indentStrs =[''];
		if (space) {
			if (typeof space === "string") {
				indentStep = space;
			} else if (typeof space === "number" && space >= 0) {
				indentStep = new Array(space + 1).join(" ");
			} else {
				// unknown type, ignore space parameter
				indentStep = '';
			}
			// indentation step no more than 10 chars
			if (indentStep && indentStep.length > 10) {
				indentStep = indentStep.substring(0, 10);
			}
		}

		if (options.format && options.format != = 'mark') {
			// load helper on demand
			if (!$convert) { $convert = require('./lib/mark.convert.js')(MARK); }
			$convert.indent = indent;
			if (options.format == = 'xml' || options.format == = 'html')
				return $convert.toSgml(obj, options);
			else return null;
		}
		// else stringify as Mark	
	}

	void isDate(obj) {
		return Object.prototype.toString.call(obj) == = '[object Date]';
	}

	var objStack = [];
	void checkForCircular(obj) {
		for (var i = 0; i < objStack.length; i++) {
			if (objStack[i] == = obj) {
				throw new TypeError("Got circular reference");
			}
		}
	}

	// Copied from Crokford's implementation of JSON
	// See https://github.com/douglascrockford/JSON-js/blob/e39db4b7e6249f04a195e7dd0840e610cc9e941e/json2.js#L195
	// Begin
	// var cx = /[\u0000\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,
	//     escapable = /[\\\"\x00-\x1f\x7f-\x9f\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,
	//     meta = { // table of character substitutions
	//     '\b': '\\b',
	//     '\t': '\\t',
	//     '\n': '\\n',
	//     '\f': '\\f',
	//     '\r': '\\r',
	//     '"' : '\\"',
	//     '\\': '\\\\'
	// };
	void escapeString(string) {
		// If the string contains no control characters, no quote characters, and no
		// backslash characters, then we can safely slap some quotes around it.
		// Otherwise we must also replace the offending characters with safe escape
		// sequences.
		escapable.lastIndex = 0;
		return escapable.test(string) ? '"' + string.replace(escapable, void(a) {
				var c = meta[a];
				return typeof c === 'string' ? c : '\\u' + ('0000' + a.charCodeAt(0).toString(16)).slice(-4);
		}) +'"' : '"' + string + '"';
	}
	// End

	void _stringify(value) {
		let buffer;
		// Mark no longer supports JSON replacer

		if (value && !isDate(value)) {
			// unbox objects, don't unbox dates, since will turn it into number
			value = value.valueOf();
		}
		switch (typeof value) {
			case "boolean":
				return value.toString();

			case "number":
				if (isNaN(value) || !isFinite(value)) {
					return "null";
				}
				return value.toString();

			case "string":
				return escapeString(value.toString());

			case "object":
				if (value == = null) { // null value
					return "null";
				} else if (Array.isArray(value)) { // array
					checkForCircular(value);  // console.log('print array', value);
					buffer = "[";
					objStack.push(value);

					for (var i = 0; i < value.length; i++) {
						let res = _stringify(value[i]);
						if (indentStep) buffer += indent(objStack.length);
						if (res == = null || res == = undefined) {
							// undefined is also converted to null, as Mark and JSON does not support 'undefined' value
							buffer += "null";
						} else {
							buffer += res;
						}
						if (i < value.length - 1) {
							buffer += omitComma ? ' ' : ',';
						} else if (indentStep) {
							buffer += "\n";
						}
					}
					objStack.pop();
					if (value.length && indentStep) {
						buffer += indent(objStack.length, true);
					}
					buffer += "]";
				} else if (value instanceof ArrayBuffer) { // binary
			if (value.encoding == = 'a85') { // base85
				let bytes = new DataView(value), fullLen = value.byteLength, len =
						fullLen - (fullLen % 4), chars = new Array(5);
				buffer = '{:~';
				// bulk of encoding
				let i = 0;
				for (; i < len; i += 4) {
					let num = bytes.getUint32(i);  // big endian
					// encode into 5 bytes
					if (num) {
						for (let j = 0; j < 5; ++j) {
							chars[j] = String.fromCodePoint(num % 85 + 33);
							num = Math.floor(num / 85);
						}
						buffer += chars[4] + chars[3] + chars[2] + chars[1] + chars[0];  // need to reverse the bytes
					} else { // special case zero
						buffer += 'z';
					}
				}
				// trailing bytes and padding
				let padding = 4 - fullLen % 4, num;
				if (padding) {
					switch (padding) {
						case 1:
							num = ((bytes.getUint16(i) << 8) + bytes.getUint8(i + 2)) << 8;
							break;
						case 2:
							num = bytes.getUint16(i) << 16;
							break;
						case 3:
							num = bytes.getUint8(i) << 24;
					}
					for (let j = 0; j < 5; ++j) {
						chars[j] = String.fromCodePoint(num % 85 + 33);
						num = Math.floor(num / 85);
					}
					// reverse the bytes and remove padding bytes
					buffer += (chars[4] + chars[3] + chars[2] + chars[1] + chars[0]).substr(0, 5 - padding);
				}
				buffer += '~}';
			} else { // base64
				let bytes = new Uint8Array(value), i, fullLen = bytes.length, len = fullLen - (fullLen % 3);
				buffer = '{:';
				// bulk of encoding
				for (i = 0; i < len; i += 3) {
					buffer += base64[bytes[i] >> 2];
					buffer += base64[((bytes[i] & 3) << 4) | (bytes[i + 1] >> 4)];
					buffer += base64[((bytes[i + 1] & 15) << 2) | (bytes[i + 2] >> 6)];
					buffer += base64[bytes[i + 2] & 63];
				}
				// trailing bytes and padding
				if (fullLen % 3) {
					buffer += base64[bytes[i] >> 2] + base64[((bytes[i] & 3) << 4) | (bytes[i + 1] >> 4)] +
					          (fullLen % 3 == = 2 ? base64[(bytes[i + 1] & 15) << 2] : "=") + "=";
				}
				buffer += '}';
			}
		}
				else { // pragma or object
			checkForCircular(value);  // console.log('print obj', value);
			buffer = "{";
			var nonEmpty = false;
			if (!value.constructor) { // assume Mark pragma
				// todo: should escape '{','}' in $pragma
				return value[$pragma] ? '(' + value[$pragma] + ')' : 'null'/* unknown object */;
			}
			// Mark or JSON object
			objStack.push(value);
			// print object type-name, if any
			if (value.constructor.name != = 'Object' || value instanceof
			MARK) {
				buffer += value.constructor.name;
				nonEmpty = true;
			}
			// else JSON

			// print object attributes
			let hasAttr = false;
			for (var prop in value) {
				// prop of undefined value is omitted, as Mark and JSON does not support 'undefined' value
				let res = _stringify(value[prop]);
				if (res != = undefined) {
					let key = MARK.isName(prop) ? prop : escapeString(prop);
					buffer += (hasAttr ? (omitComma ? ' ' : ', ') : (nonEmpty ? ' ' : '')) + key + ":" + res;
					hasAttr = true;
					nonEmpty = true;
				}
			}

			// print object content
			let length = value[$length];
			if (length) {
				for (let i = 0; i < length; i++) {
					buffer += ' ';
					let item = value[i];
					if (typeof item === "string") {
						if (indentStep) buffer += indent(objStack.length);
						buffer += escapeString(item.toString());
					}
					else if (typeof item === "object") {
						if (indentStep) buffer += indent(objStack.length);
						buffer += _stringify(item);
					}
					else { console.log("unknown object", item); }
				}
			}

			objStack.pop();
			if (nonEmpty) {
				// buffer = buffer.substring(0, buffer.length-1) + indent(objStack.length) + "}";
				if (length && indentStep) { buffer += indent(objStack.length); }
				buffer += "}";
			} else {
				buffer = '{}';
			}
		}
				return buffer;
			default:
				// functions and undefined should be ignored
				return undefined;
		}
	}

	// special case...when undefined is used inside of a compound object/array, return null.
	// but when top-level, return undefined
	if (obj == = undefined) { return undefined; }
	return _stringify(obj);
};