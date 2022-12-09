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


//class SmartPointer64;
typedef unsigned long long SmartPointer64;

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
//unsigned long long
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
    NaN = 0xFFF0000000000000L, // compatible with
//    NaN = 0b111111111xxxxxxx, NaN FLOAT representation in IEEE 754 single precision (32-bit)
//    NaN = 0b011111111xxxxxxx, NaN FLOAT representation in JS!  IEEE 754 single precision (32-bit) =>
    PlusNaN = 0x7FF0000000000000L, // NaN FLOAT representation in JS  "The sign bit does not matter"
//    PlusNaN = 0x7FFFFFFFFFFFFFFFL, // NaN FLOAT representation in JS  "The sign bit does not matter"
//    NaN = 0x0F0F0F0F0F0F0F0FL,
//    NaN = 0x0F0F000000000000L,
//    NAN = 0xF0F0F0F0F0F0F0F0L,
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

union smartlong{
    long long lon;
    SmartPointer64 smarty; // encoding NaN, Infinity, -Infinity, missing ≈ empty ≈ null AND OTHER types/data!!
};


#include "../Util.h"

class BigInt {
public:
    bytes digits;

    BigInt(long l) {
        digits = (bytes) formatLong(l);// todo …
    }

    BigInt operator+(BigInt other) { todo("BigInt"); }

    BigInt operator-(BigInt other) { todo("BigInt"); }

    BigInt operator*(BigInt other) { todo("BigInt"); }

    BigInt operator/(BigInt other) { todo("BigInt"); }
};

// todo: reconcile with Valtype
// todo: reconcile with Primitive … AGAIN!
// todo: reconcile with SmartNumber / smarty4 smarty32 smarty64 HEADERs
enum NumberType {
    number_undefined, // NaN
    number_null, // special zero
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
    number_complex,// because why not
    number_bigint,
    number_fraction,
    number_infinite,
    number_minus_infinite,
    number_digits, // unparsed digits in char*
};

struct Fraction {
    long nominator;
    long denominator;
};

struct Complex {
    float real;
    float imagine;
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
    long long longe; // see Node.longy
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

class Number {
    NumberType type;
    NumberValue value;
    bool signable;// vs always unsigned positive
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

    Number(long a) {
        value.longe = a;
        type = number_long;
    }

    Number(long long a) {
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

    Number(BigInt b) {
        value.bigint = b;
        type = number_bigint;
    }

    Number(chars a) {
        // todo auto BigInt
        long i = parseLong(a);
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

    Number operator+(Number other) {
        NumberType type2 = other.type;
        if (type > number_long or type2 > number_long) {
            if (type > number_double or type2 > number_double)
                todo("Number operator +");
            return Number(value.doubl + other.value.doubl);
        }
        long l1 = value.longe;
        long l2 = other.value.longe;
        // overflow => bigger type
        if (abs(l1) + abs(l2) < 0)return Number(BigInt(l1) + BigInt(l2));
        long sum = l1 + l2;
        if (sum > -2l << 32 and sum < 2l << 32) return Number((int) sum);// number fits int
        return Number(sum);// max(type2,number_type)
    }

    Number operator*(Number other) {
        NumberType type2 = other.type;
        if (type > number_long or type2 > number_long) {
            if (type > number_double or type2 > number_double)
                todo("Number operator *");
            return Number(value.doubl * other.value.doubl);// ignore 10E330 overflow
        }
        long l1 = value.longe;
        long l2 = other.value.longe;
        if (abs(l1) * abs(l2) < 0) // overflow => bigger type
            return Number(BigInt(l1) + BigInt(l2));
        long prod = l1 * l2;
        if (prod > -2l << 32 and prod < 2l << 32) return Number((int) prod);// number fits int
        return Number(prod);// max(type2,number_type)
    }

};

Number parseNumber(chars string);
