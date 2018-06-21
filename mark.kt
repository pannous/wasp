import kotlinx.serialization.*

import kotlin.io.println as println
import kotlin.io.println as puts
import com.beust.klaxon.JsonReader
import com.beust.klaxon.KlaxonException
import kotlinx.io.StringReader
import java.lang.Double.isFinite

import kotlin.collections.mutableMapOf as map
import kotlin.collections.mutableListOf as list // YES!
//import extensions.*

@Serializable
data class Data(val a: Int, @Optional val b: String = "42")

fun main(args: Array<String>) {
	val json = """
		{"a": 42, "b": "42", "c":{"d":123}}
		"""
//	val dyn: dynamic // ONLY IN JS
	val mark = mark(json)
	print(mark)
//	parseKlaxon(json)
}

fun mark(json: String): Any? {
	return MarkParser(json).values()
}

val NULL:Char = 0.toChar()// Char.(0)// Char.MIN_SURROGATE //  '\\0'

data class MarkParser(var json: String) {
	var index = 0
	var len: Int
	var cur: Char='?'

	init {
		len=json.length
		cur=json[0]
	}

	fun data(): Map<String, Any?> {
		val map = map<String, Any?>()
		white()
		token('{')
		while (!peek('}')) {
			white()
			val key:String = key()
			var value: Any? = null
			white()
			val assign = maybe(':') or maybe('=')
			if (!!assign) {
				value = values()
			}
			white()
			maybe(',')
			map[key] = value
		}
		next()
		return map

		if (json[index] != '{') throw Error("expect")
	}

	fun values():Any? {
		white()
		if(peek('{'))return data()
		if(peek('"'))return key()
		if(isNameStart(current())) return key()
		if(isNumber(current())) return number()
		return null
	}

	private fun number(): Number {

		var number=0
		var sign = NULL
		var string = ""
		var base = 10

			if (cur == '-' || cur == '+') {
				sign = cur;  next();
			}

			// support for Infinity (could tweak to allow other words):
//			if (ch == 'I') {
//				number = word()
//				if (typeof number !== 'number' || isNaN(number)) {
//					error('Unexpected word for number');
//				}
//				return (sign == '-') ? -number : number;
//			}

			// support for NaN
//			if (ch == 'N' ) {
//				number = word();
//				if (!isNaN(number)) {
//					error('expected word to be NaN');
//				}
//				// ignore sign as -NaN also is NaN
//				return number;
//			}

			if (cur == '0') {
				string += cur;  next();
			} else {
				while (cur >= '0' && cur <= '9' ) {
					string += cur;
					next();
				}
				if (cur == '.') {
					string += '.';
					while (cur >= '0' && cur <= '9') {
						string += cur;
						next()
					}
				}
				if (cur == 'e' || cur == 'E') {
					string += cur;
					next();
					if (cur == '-' || cur == '+') {
						string += cur;
						next();
					}
					while (cur >= '0' && cur <= '9') {
						string += cur;
						next();
					}
				}
			}
		number= string.toLong().toInt()

			if (sign == '-') number = -number

			if (!isFinite(number.toDouble())) {
				error("Bad number");
			} else {
				return number;
			}
		}



	private fun white() {
		while (cur == ' ' || cur == '\n' || cur == '\t') next()
	}

	private fun key(): String {
		val quot = maybe('"')
		val word = word(!!quot)
		if (!!quot) token(quot)
		return word
	}

	private fun word(quoted: Boolean): String {
		var w = ""
		if (!quoted && !isNameStart(current()))
			throw Error("Invalid name start " + current())
		while (true) {
			if (isNameChar(cur)) w += cur
			else break
			next()
		}
		return w
	}

	private fun current(): Char {
		return json[index]
	}

	fun isNumber(c: Char): Boolean { // todo : -0x.75E12
		return ('0' <= c && c <= '9')
	}

	fun isNameChar(c: Char): Boolean {
		return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') ||
				c == '_' || c == '.' || c == '-';
	}

	fun isNameStart(c: Char): Boolean {
		return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
	}


	var base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


	private fun peek(c: Char): Boolean {
		return json[index] == c
	}

	private fun maybe(c: Char): Char {
		if (json[index] == c){
			next()
			return c;
		}
		return NULL
	}

	private fun next(): Char {
		val c = json[index++];
		cur = json[index];
		return c;
	}

	private fun token(c: Char): Char {
		if(endOfFile())throw Error("endOfFile")
		if (json[index] != c) throw Error("expected " + c)
		index++
		cur=json[index]
		return c // tested token, not 'next'!
	}

//	val endOfFile:()->Boolean={index>=len}
	val endOfFile={index>=len}
//
}

private infix fun Char.or(maybe: Char): Any {
	if (this == NULL) return maybe
	else return this
}





fun parseKlaxon(json: String) {
	JsonReader(StringReader(json)).use { reader ->
		while (reader.hasNext()) {
			try {
				val root = reader.nextObject()
				for (e in root.entries)
					println(e)
			} catch (e: KlaxonException) {
				if ("read EOF" in e.message!!)
					break // WTF hasNext() EOF!?
				throw e
//				print(e)
			}
//			if(!!nextObject)
//				val readName = reader.nextName()
//				val obj = reader.nextObject()
//				val entries = obj.entries
//				for (e in entries)
//					print(e)
		}
	}
}
