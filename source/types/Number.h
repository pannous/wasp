//
// Created by me on 18.09.21.
//

#ifndef WASP_NUMBER_H
#define WASP_NUMBER_H

// types should inherit from node!

typedef enum number_type {
	boolean,
//	byte,:
	int8,// unsigned
//	int16,// unsigned?
	int32,// signed
	int32u,// unsigned
	int64,
	leb128,
	float32,
	float64,
//	floaty,
} number_type;


class Number {
	number_type type;
	long r;
	long i;// imaginary or r/i in rationals or pointer to leb128/bignum
	Number(number_type _type, long _r, long _i) {
		type = _type;
		r = _r;
		i = _i;
	}

	bool compatible(Number &number) {
		if (type == number.type)return true;
		return false;
	}

	Number add(Number &other) {
		Number res = Number(type, r, i);
		if (compatible(other)) {
			res.r += other.r;
			res.i += other.i;
		}
		return res;
	}
};


#endif //WASP_NUMBER_H
