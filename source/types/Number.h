//
// Created by me on 18.09.21.
//

// types should inherit from node!

//typedef enum number_type {
//	boolean,
////	byte,:
//	int8,// unsigned
////	int16,// unsigned?
//	int32,// signed
//	int32u,// unsigned
//	int64,
//	leb128,
//	float32,
//	float64,
////	floaty,
//} number_type;

//#include <cmath>

//class SmartPointer64;
typedef uint64 SmartPointer64;

// https://en.wikipedia.org/wiki/NaN
// signals and payloads are not new inventions ;)
// Quiet NaN  do not raise any additional exceptions as they propagate through most operations. except formatting …
// Signaling NaN introduced in IEEE 754
/*There have been several ideas for how these might be used:
    Filling uninitialized memory with signaling NaNs would produce the invalid operation exception if the data is used before it is initialized
    Using an sNaN as a placeholder for a more complicated object, such as:
        A representation of a number that has underflowed
        A representation of a number that has overflowed
        Number in a higher precision format
        A complex number
When encountered, a trap handler could decode the sNaN and return an index to the computed result.
 In practice, this approach is faced with many complications. */
// int 60 : we reserve the 4 highest bits for special semantics and values
// whenever the first hex in a 16byte smartlong sequence is NOT 0L, it means it is special:
enum smartlongs {
//uint64
    zero = 0L, // lol
    LONG_MAX_VALUE = 0x7FFFFFFFFFFFFFFFL, // ≠  Number.MAX_VALUE 2^^1024 - 1 ( Float 1.7976931348623157E+308 )
    LONG_MIN_VALUE = 0x8000000000000000L, // todo
    max_value = 0x0EFFFFFFFFFFFFFFFL,// different from usual LONG_MAX_VALUE 2^59 values ok? 'Only'
    max_value_boarder = 0x0F00000000000000L, // 2^56 room for special values between plus_infinity
//    min_value=0xF100000000000000L, // different from usual LONG_MIN_VALUE 2^59 values ok
    min_value = 0xFFF1000000000000L, // 51 bit negatives more than enough! why? to be compatible with NaN
//    plus_infinity  = 0x0F000000000000000L, //smartlongs::MAX_VALUE + 1 , // *2 vs +1 => reserve 2^(64-1) special values!
    plus_infinity = 0x0FFFFFFFFFFFFFFFL,// ∞ inf
    minus_infinity = 0xF000000000000000L,// -∞ -inf
//    NaN = 0x0FFFFFFFFFFFFFFFL,
    not_a_number = 0xFFF0000000000000L, // compatible with
    NaN = not_a_number,
//    NaN = 0b111111111xxxxxxx, NaN FLOAT representation in IEEE 754 single precision (32-bit)
//    NaN = 0b011111111xxxxxxx, NaN FLOAT representation in JS!  IEEE 754 single precision (32-bit) =>
    PlusNaN = 0x7FF0000000000000L, // NaN FLOAT representation in JS  "The sign bit does not matter"
//    PlusNaN = 0x7FFFFFFFFFFFFFFFL, // NaN FLOAT representation in JS  "The sign bit does not matter"
//    NaN = 0x0F0F0F0F0F0F0F0FL,
//    NaN = 0x0F0F000000000000L,
//    Nan = 0xF0F0F0F0F0F0F0F0L,
//    ∞=infinity=0x1FFFFFFFFFFFFFFFL,
//    -∞=minus_infinity=0xFFFFFFFFFFFFFFFFL,
//    minus_one_leb=0x7f01010101010101,// 7 nops
//    NaN = 0xEEEE0000EEEE0000L, // special semantics
};

/* JS:
 hex(1152921504606847000-64)
'1000000000000000'
 hex(1152921504606847000-65)
'fffffffffffff80'
*/

union smartlong {
    int64 lon;
    SmartPointer64 smarty; // encoding NaN, Infinity, -Infinity, missing ≈ empty ≈ null AND OTHER types/data!!
};


#include "../Util.h"

class BigInt {
public:
    bytes digits;

    BigInt(int64 l) {
        digits = (bytes) formatLong(l);// todo …
    }

    BigInt operator+(BigInt other) {
        todo("BigInt");
        return 0;
    }

    BigInt operator-(BigInt other) {
        todo("BigInt");
        return 0;
    }

    BigInt operator*(BigInt other) {
        todo("BigInt");
        return 0;
    }

    BigInt operator/(BigInt other) {
        todo("BigInt");
        return 0;
    }
};

int64 gcd(int64 a, int64 b) {
    if (a == 0) return b;
    return gcd(b % a, a);
}

// todo: reconcile with Valtype
// todo: reconcile with Primitive … AGAIN!
// todo: reconcile with SmartNumber / smarty4 smarty32 smarty64 HEADERs
enum NumberType {
    number_undefined, // NaN
    number_null, // special zero todo ??
    number_zero,
    number_bool,
    number_byte,
    number_u8 = number_byte,
    number_short,
    number_sint16 = number_short,
    number_int,
    number_int32 = number_int,
    number_long,
    number_int64 = number_long,
    number_float,
    number_double,
    number_float64 = number_double,
    number_complex32x32,// because why not
    number_bigint,// pointer to larger BigInt struct
    number_fraction32x32,
    number_epsilon, // type of value?
    number_infinite,
    number_minus_infinite,
    number_digits, // unparsed digits in char*
};

chars typeName(NumberType type) {
    switch (type) {
        case number_undefined:
            return "undefined";
        case number_null:
            return "null";
        case number_bool:
            return "bool";
        case number_byte:
            return "byte";
        case number_short:
            return "short";
        case number_int:
            return "int";
        case number_long:
            return "long";
        case number_float:
            return "float";
        case number_double:
            return "double";
        case number_complex32x32:
            return "complex";
        case number_bigint:
            return "bigint";
        case number_fraction32x32:
            return "fraction";
        case number_infinite:
            return "infinite";
        case number_minus_infinite:
            return "-∞";
        case number_digits:
            return "digits";
        case number_zero:
            return "zero";
        case number_epsilon:
            return "epsilon";
    }
}

struct Fraction64x64 {
    int64 nominator;
    int64 denominator;
};

struct Fraction {
    int nominator;
    int denominator;
};

struct Complex {
    float real;
    float imaginary;
};

// can represent 60bit integers, floats COMPATIBLE
//  Node Kind number = 0x70, // SmartNumber or Number* as SmartPointer? ITS THE SAME!
struct SmartNumber {
    byte header;// NumberType
    byte more;
    int number;
};

union NumberValue { // 64 bit: (56/60 bit if extracted from SmartNumber
    double doubl; // see Node.real
    int64 longe; // see Node.longy
    Fraction fraction;// two int
    Complex complex;// float
    char *digits; // todo
//    BigInt* bigint;// pointer to BigInt
    BigInt bigint;// pointer to digits
    SmartNumber smartNumber;// same as ALL above but 60bit with 4bit header NumberType included
    NumberValue() {}
};

union SmartNumberUnion {
    u64 raw;
    NumberValue value;
    SmartNumber type;// .header
    SmartNumber number; // combined type and value
};

bool isInt(int64 i) {
    return i < 0x80000000 and i >= -0x80000000l;
}

class Number {

    Number(int64 nominator, int64 denominator) {
        if (isInt(nominator) and isInt(denominator)) {
            value.fraction.nominator = nominator;
            value.fraction.denominator = denominator;
            type = number_fraction32x32;
        } else {
            value.doubl = nominator / denominator;// todo losing precision ok?
            type = number_double;
//            auto bigInt = BigInt(nominator) / BigInt(denominator);
//            value.bigint=bigInt;
        }
    }

    NumberType type = number_undefined;
    NumberValue value;

public:
    Number(bool a) {
        value.longe = a;
        type = number_bool;
    }

    Number(byte a) {
        value.longe = a;
        type = number_byte;
    }

    Number(short a) {
        value.longe = a;
        type = number_short;
    }

    Number(int a) {
        value.longe = a;
        type = number_int;
    }

    Number(int64 a) {
        value.longe = a;
        type = number_long;
    }

    Number(float a) {
        value.doubl = a;
        type = number_float;
    }

    Number(double a) {
        value.doubl = a;
        type = number_double;
    }

    // float128 WE DONT WANT THAT
//    Number(long double a) {
//        value.doubl = a;
//        type = number_double;
//    }


    Number(BigInt b) {
        value.bigint = b;
        type = number_bigint;
    }

    Number(chars a) {
        // todo auto BigInt
        int64 i = parseLong(a);
        double d = parseDouble(a);
//        value.digits = a;
//        number_type = number_digits;
        // todo : BigInt maybe
        if (i == d) {
            value.longe = i;
            type = number_long;
        } else {
            value.doubl = d;
            type = number_double;
        }
    }

    explicit
    operator double() {
        if (type == number_double) return value.doubl;
        if (type == number_long) return value.longe;
        if (type == number_fraction32x32) return value.fraction.nominator / (double) value.fraction.denominator;
        if (type == number_complex32x32) {
            if (value.complex.imaginary == 0)
                return value.complex.real;
            else
                error("Number is complex");
        }
        todo("Number cast to double for types "s + typeName(type));
        return 0;
    }


    explicit
    operator float() {
        return (double) *this;
    }

//    bool operator==(float other) {
//        return Number(other) == *this;
//    }

    bool operator==(Number other) {
        NumberType type2 = other.type;
        // order matters since we test from simple to complex types
        if (type <= number_long and type2 <= number_long)
            return value.longe == other.value.longe;
        if (type <= number_double and type2 <= number_long)
            return value.doubl == other.value.longe;
        if (type <= number_long and type2 <= number_double)
            return value.longe == other.value.doubl;
        if (type <= number_double and type2 <= number_double)
            return value.longe == other.value.doubl;
        if (type == number_fraction32x32 and type2 <= number_long)
            return value.fraction.nominator / value.fraction.denominator == other.value.longe;
        if (type == number_fraction32x32 and type2 == number_double)
            return (double) *this == other.value.doubl;
//            return (float) *this == (float) other.value.doubl;
        todo("Number operator ==  for types "s + typeName(type) + " and " + typeName(type2));
        return false;
    };


    Number operator+(Number other) {
        NumberType type2 = other.type;
        if (type > number_long or type2 > number_long) {
            if (type > number_double or type2 > number_double) todo("Number operator +");
            return Number(value.doubl + other.value.doubl);
        }
        int64 l1 = value.longe;
        int64 l2 = other.value.longe;
        // overflow => bigger type
        if (abs(l1) + abs(l2) < 0)
            return Number(BigInt(l1) + BigInt(l2));
        int64 sum = l1 + l2;
        if (isInt(sum)) return Number((int) sum);// number fits int
        return Number(sum);// max(type2,number_type)
    }

    Number operator^(Number other) {
        NumberType type2 = other.type;
        if (type <= number_long and type2 <= number_long)
            return Number(powd(value.longe, other.value.longe));
        if (type <= number_long and type2 <= number_double)
            return Number(powd(value.longe, other.value.doubl));
        if (type <= number_double and type2 <= number_long)
            return Number(powd(value.doubl, other.value.longe));
        if (type <= number_double and type2 <= number_double)
            return Number(powd(value.doubl, other.value.doubl));
        todo("Number operator ^ for "s + typeName(type) + " and " + typeName(type2));
        return Number(0);
    }

    Number operator*(Number other) {
        NumberType type2 = other.type;
        auto value2 = other.value;
        if (type <= number_long and type2 <= number_long) {
            int64 l1 = value.longe;
            int64 l2 = value2.longe;
            if (abs(l1) * abs(l2) < 0) // overflow => bigger type
                return Number(BigInt(l1) * BigInt(l2));
            int64 prod = l1 * l2;
            if (isInt(prod)) return Number((int) prod);// number fits int
            return Number(prod);// max(type2,number_type)
        }
        if (type <= number_double and type2 <= number_long)
            return value.doubl * value2.longe;
        if (type <= number_long and type2 <= number_double)
            return value.longe * value2.doubl;
        if (type <= number_double and type2 <= number_double)
            return value.longe * value2.doubl;
        if (type == number_fraction32x32 and type2 == number_double)
            return (double) *this * value2.doubl;
        if (type == number_fraction32x32 and type2 == number_long)
            return Number(value.fraction.nominator * value2.longe, value.fraction.denominator);

        if (type > number_long or type2 > number_long) {
            return Number(value.doubl * value2.doubl);// ignore 10E330 overflow
        }
        todo("Number operator * for "s + typeName(type) + " and " + typeName(type2));
        return Number(0);
    }

    Number operator/(Number other) {
        auto type2 = other.type;
        auto value2 = other.value;
        if (type <= number_long and type2 <= number_long) {
            if (value.longe % value2.longe == 0)
                return Number(value.longe / value2.longe);
            return Number(value.longe, value2.longe);
        }
        if (type <= number_double and type2 <= number_long)
            return value.doubl / value2.longe;
        if (type <= number_long and type2 <= number_double)
            return value.longe / value2.doubl;
        if (type <= number_double and type2 <= number_double)
            return value.longe / value2.doubl;
        if (type == number_fraction32x32 and type2 == number_double)
            return (double) *this / value2.doubl;
        if (type == number_fraction32x32 and type2 == number_long)
            return Number(value.fraction.nominator, value.fraction.denominator * value2.longe).simplify();

        todo("Number operator /  for types "s + type + " and " + type2);
        return Number(0);
    }

    Number simplify() {
        if (type <= number_long) {
            if (value.longe == 0 or value.longe == 1)type = number_bool;
            if (value.longe >= 0 and value.longe < 0x100)type = number_byte;
            if (value.longe > -0x8000 and value.longe < 0x8000)type = number_short;
            if (isInt(value.longe))type = number_int;
            return *this;
        }
        if (type <= number_double) {
            if ((int64) value.doubl == value.doubl)
                return Number((int64) value.doubl);
            return *this;
        }
        if (type == number_fraction32x32) {
            auto gcd1 = gcd(value.fraction.nominator, value.fraction.denominator);
            if (gcd1 == value.fraction.denominator)return Number(value.fraction.nominator);
            return Number(value.fraction.nominator / gcd1, value.fraction.denominator / gcd1);
        }
        return *this;
    }

    //    bool signable();// vs always unsigned positive
};

Number parseNumber(chars string);
