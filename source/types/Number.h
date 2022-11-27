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





#include "../Util.h"

class BigInt {
public:
    bytes digits;

    BigInt(long l) {
        digits = (bytes) itoa(l);// todo …
    }

    BigInt operator+(BigInt other) { todo("BigInt"); }

    BigInt operator-(BigInt other) { todo("BigInt"); }

    BigInt operator*(BigInt other) { todo("BigInt"); }

    BigInt operator/(BigInt other) { todo("BigInt"); }
};


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

// can represent 60bit integers, floats
struct SmartNumber {
    byte header;// NumberType
    byte more;
    int number;
};

union NumberValue { // 64 bit:
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
        long i = atoi0(a);
        double d = atof0(a);
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
