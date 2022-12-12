//#pragma once
//
// Created by me on 20.10.20.
// https://monoinfinito.wordpress.com/2013/02/19/c-exceptions-under-the-hood-3-an-abi-to-appease-the-linker/
// Wasm cpp exception ABI shim
//

//#include <stdio.h>
//#include "String.h"
//#include <exception>
//#include <typeinfo>
//
//namespace __cxxabiv1 {
//	struct __vmi_class_type_info {
//		virtual void foo() {}
//	} vmiClassTypeInfo;
//
//	struct __pointer_type_info {
//		virtual void foo() {}
//	} pointerTypeInfo;
//
//	struct __class_type_info {
//		virtual void foo() {}
//	} classTypeInfo;
//}
//class type_info {
//public:
//	virtual ~type_info();
//	bool operator==(const type_info &rhs) const noexcept;
//	bool operator!=(const type_info &rhs) const noexcept;
//	bool before(const type_info &rhs) const noexcept;
//	size_t hash_code() const noexcept;
//	chars name() const noexcept;
//};
#define EXCEPTION_BUFF_SIZE 255
char exception_buff[EXCEPTION_BUFF_SIZE];

extern "C" {
//
//void *__cxa_allocate_exception(size_t thrown_size) {
//	if (thrown_size > EXCEPTION_BUFF_SIZE) printf("Exception too big");
//	return &exception_buff;
//}

//void __cxa_free_exception(void *thrown_exception);
////#include <unwind.h>
//void __cxa_throw(
//		void *thrown_exception,
//		struct type_info *tinfo,
//		void (*dest)(void *));
//void __cxa_throw(
//		void *thrown_exception,
//		struct type_info *tinfo,
//		void (*dest)(void *)) {
//    printf("Exception interception\n"); // works, but doesn't add anything to raise() function. Only in unmodified code
//    printf("%s\n", ((String *) thrown_exception)->data);// OMG THIS WORKS
////	printf("%s\n",tinfo);
//    dest(thrown_exception);
//    exit(0);
//    // __cxa_throw never returns
//}

} // extern "C"
