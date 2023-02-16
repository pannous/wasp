/*
 * Copyright 2018 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WASM_RT_H_
#define WASM_RT_H_

#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0  // Compatibility with non-clang compilers.
#endif

#if __has_builtin(__builtin_expect)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define UNLIKELY(x) (x)
#define LIKELY(x) (x)
#endif

#if __has_builtin(__builtin_memcpy)
#define wasm_rt_memcpy __builtin_memcpy
#else
#define wasm_rt_memcpy memcpy
#endif

#if __has_builtin(__builtin_unreachable)
#define wasm_rt_unreachable __builtin_unreachable
#else
#define wasm_rt_unreachable abort
#endif

/**
 * Enable memory checking via a signal handler via the following definition:
 *
 * #define WASM_RT_MEMCHECK_SIGNAL_HANDLER 1
 *
 * This is usually 10%-25% faster, but requires OS-specific support.
 */

#ifndef WASM_RT_SKIP_SIGNAL_RECOVERY
#define WASM_RT_SKIP_SIGNAL_RECOVERY 0
#endif

/** Signal handler not supported on 32-bit platforms. */
#if UINTPTR_MAX > 0xffffffff

#define WASM_RT_SIGNAL_RECOVERY_SUPPORTED 1

/* Signal handler is supported. Use it by default. */
#ifndef WASM_RT_MEMCHECK_SIGNAL_HANDLER
#define WASM_RT_MEMCHECK_SIGNAL_HANDLER 1
#endif

#else
#define WASM_RT_SIGNAL_RECOVERY_SUPPORTED 0

/* Signal handler is not supported. */
#ifndef WASM_RT_MEMCHECK_SIGNAL_HANDLER
#define WASM_RT_MEMCHECK_SIGNAL_HANDLER 0
#endif

#endif

#if WASM_RT_MEMCHECK_SIGNAL_HANDLER && \
    (!WASM_RT_SKIP_SIGNAL_RECOVERY && !WASM_RT_SIGNAL_RECOVERY_SUPPORTED)
/* The signal handler is not supported, error out if the user was trying to
 * enable it. */
#error "Signal handler is not supported for this OS/Architecture!"
#endif

#ifndef WASM_RT_USE_STACK_DEPTH_COUNT
/* The signal handler on POSIX can detect call stack overflows. On windows, or
 * platforms without a signal handler, we use stack depth counting. */
#if WASM_RT_MEMCHECK_SIGNAL_HANDLER && !defined(_WIN32)
#define WASM_RT_USE_STACK_DEPTH_COUNT 0
#else
#define WASM_RT_USE_STACK_DEPTH_COUNT 1
#endif
#endif

#if WASM_RT_USE_STACK_DEPTH_COUNT
/**
 * When the signal handler cannot be used to detect stack overflows, stack depth
 * is limited explicitly. The maximum stack depth before trapping can be
 * configured by defining this symbol before including wasm-rt when building the
 * generated c files, for example:
 *
 * ```
 *   cc -c -DWASM_RT_MAX_CALL_STACK_DEPTH=100 my_module.c -o my_module.o
 * ```
 */
#ifndef WASM_RT_MAX_CALL_STACK_DEPTH
#define WASM_RT_MAX_CALL_STACK_DEPTH 500
#endif

/** Current call stack depth. */
extern uint32_t wasm_rt_call_stack_depth;

#endif

#if defined(_MSC_VER)
#define WASM_RT_NO_RETURN __declspec(noreturn)
#else
#define WASM_RT_NO_RETURN __attribute__((noreturn))
#endif

#if defined(__APPLE__) && WASM_RT_MEMCHECK_SIGNAL_HANDLER
#define WASM_RT_MERGED_OOB_AND_EXHAUSTION_TRAPS 1
#else
#define WASM_RT_MERGED_OOB_AND_EXHAUSTION_TRAPS 0
#endif

/** Reason a trap occurred. Provide this to `wasm_rt_trap`. */
typedef enum {
    WASM_RT_TRAP_NONE, /** No error. */
    WASM_RT_TRAP_OOB,  /** Out-of-bounds access in linear memory or a table. */
    WASM_RT_TRAP_INT_OVERFLOW, /** Integer overflow on divide or truncation. */
    WASM_RT_TRAP_DIV_BY_ZERO,  /** Integer divide by zero. */
    WASM_RT_TRAP_INVALID_CONVERSION, /** Conversion from NaN to integer. */
    WASM_RT_TRAP_UNREACHABLE,        /** Unreachable instruction executed. */
    WASM_RT_TRAP_CALL_INDIRECT,      /** Invalid call_indirect, for any reason. */
    WASM_RT_TRAP_UNCAUGHT_EXCEPTION, /* Exception thrown and not caught */
#if WASM_RT_MERGED_OOB_AND_EXHAUSTION_TRAPS
    WASM_RT_TRAP_EXHAUSTION = WASM_RT_TRAP_OOB,
#else
    WASM_RT_TRAP_EXHAUSTION, /** Call stack exhausted. */
#endif
} wasm_rt_trap_t;

/** Value types. Used to define function signatures. */
typedef enum {
    WASM_RT_I32,
    WASM_RT_I64,
    WASM_RT_F32,
    WASM_RT_F64,
    WASM_RT_FUNCREF,
    WASM_RT_EXTERNREF,
} wasm_rt_type_t;

/**
 * A generic function pointer type, both for Wasm functions (`code`)
 * and host functions (`hostcode`). All function pointers are stored
 * in this canonical form, but must be cast to their proper signature
 * to call.
 */
typedef void (*wasm_rt_function_ptr_t)(void);

/** A function instance (the runtime representation of a function).
 * These can be stored in tables of type funcref, or used as values. */
typedef struct {
    /** The index as returned from `wasm_rt_register_func_type`. */
    uint32_t func_type;
    /** The function. The embedder must know the actual C signature of the
     * function and cast to it before calling. */
    wasm_rt_function_ptr_t func;
    /** A function instance is a closure of the function over an instance
     * of the originating module. The module_instance element will be passed into
     * the function at runtime. */
    void *module_instance;
} wasm_rt_funcref_t;

/** Default (null) value of a funcref */
static const wasm_rt_funcref_t wasm_rt_funcref_null_value = {0, NULL, NULL};

/** The type of an external reference (opaque to WebAssembly). */
typedef void *wasm_rt_externref_t;

/** Default (null) value of an externref */
static const wasm_rt_externref_t wasm_rt_externref_null_value = NULL;

/** A Memory object. */
typedef struct {
    /** The linear memory data, with a byte length of `size`. */
    uint8_t *data;
    /** The current and maximum page count for this Memory object. If there is no
     * maximum, `max_pages` is 0xffffffffu (i.e. UINT32_MAX). */
    uint32_t pages, max_pages;
    /** The current size of the linear memory, in bytes. */
    uint32_t size;
} wasm_rt_memory_t;

/** A Table of type funcref. */
typedef struct {
    /** The table element data, with an element count of `size`. */
    wasm_rt_funcref_t *data;
    /** The maximum element count of this Table object. If there is no maximum,
     * `max_size` is 0xffffffffu (i.e. UINT32_MAX). */
    uint32_t max_size;
    /** The current element count of the table. */
    uint32_t size;
} wasm_rt_funcref_table_t;

/** A Table of type externref. */
typedef struct {
    /** The table element data, with an element count of `size`. */
    wasm_rt_externref_t *data;
    /** The maximum element count of this Table object. If there is no maximum,
     * `max_size` is 0xffffffffu (i.e. UINT32_MAX). */
    uint32_t max_size;
    /** The current element count of the table. */
    uint32_t size;
} wasm_rt_externref_table_t;

/** Initialize the runtime. */
void wasm_rt_init(void);

/** Is the runtime initialized? */
bool wasm_rt_is_initialized(void);

/** Free the runtime's state. */
void wasm_rt_free(void);

/**
 * Stop execution immediately and jump back to the call to `wasm_rt_impl_try`.
 * The result of `wasm_rt_impl_try` will be the provided trap reason.
 *
 * This is typically called by the generated code, and not the embedder.
 */
WASM_RT_NO_RETURN void wasm_rt_trap(wasm_rt_trap_t);

/**
 * Return a human readable error string based on a trap type.
 */
const char *wasm_rt_strerror(wasm_rt_trap_t trap);

/**
 * Register a function type with the given signature. The returned function
 * index is guaranteed to be the same for all calls with the same signature.
 * The following varargs must all be of type `wasm_rt_type_t`, first the
 * params` and then the `results`.
 *
 *  ```
 *    // Register (func (param i32 f32) (result i64)).
 *    wasm_rt_register_func_type(2, 1, WASM_RT_I32, WASM_RT_F32, WASM_RT_I64);
 *    => returns 1
 *
 *    // Register (func (result i64)).
 *    wasm_rt_register_func_type(0, 1, WASM_RT_I32);
 *    => returns 2
 *
 *    // Register (func (param i32 f32) (result i64)) again.
 *    wasm_rt_register_func_type(2, 1, WASM_RT_I32, WASM_RT_F32, WASM_RT_I64);
 *    => returns 1
 *  ```
 */
uint32_t wasm_rt_register_func_type(uint32_t params, uint32_t results, ...);

/**
 * Register a tag with the given size. Returns the tag.
 */
uint32_t wasm_rt_register_tag(uint32_t size);

/**
 * Set the active exception to given tag, size, and contents.
 */
void wasm_rt_load_exception(uint32_t tag, uint32_t size, const void *values);

/**
 * Throw the active exception.
 */
WASM_RT_NO_RETURN void wasm_rt_throw(void);

/**
 * The type of an unwind target if an exception is thrown and caught.
 */
#define WASM_RT_UNWIND_TARGET jmp_buf

/**
 * Get the current unwind target if an exception is thrown.
 */
WASM_RT_UNWIND_TARGET *wasm_rt_get_unwind_target(void);

/**
 * Set the unwind target if an exception is thrown.
 */
void wasm_rt_set_unwind_target(WASM_RT_UNWIND_TARGET *target);

/**
 * Tag of the active exception.
 */
uint32_t wasm_rt_exception_tag(void);

/**
 * Size of the active exception.
 */
uint32_t wasm_rt_exception_size(void);

/**
 * Contents of the active exception.
 */
void *wasm_rt_exception(void);

#if WASM_RT_MEMCHECK_SIGNAL_HANDLER && !defined(_WIN32)
#define WASM_RT_SETJMP(buf) sigsetjmp(buf, 1)
#else
#define WASM_RT_SETJMP(buf) setjmp(buf)
#endif

#define wasm_rt_try(target) WASM_RT_SETJMP(target)

/**
 * Initialize a Memory object with an initial page size of `initial_pages` and
 * a maximum page size of `max_pages`.
 *
 *  ```
 *    wasm_rt_memory_t my_memory;
 *    // 1 initial page (65536 bytes), and a maximum of 2 pages.
 *    wasm_rt_allocate_memory(&my_memory, 1, 2);
 *  ```
 */
void wasm_rt_allocate_memory(wasm_rt_memory_t *,
                             uint32_t initial_pages,
                             uint32_t max_pages);

/**
 * Grow a Memory object by `pages`, and return the previous page count. If
 * this new page count is greater than the maximum page count, the grow fails
 * and 0xffffffffu (UINT32_MAX) is returned instead.
 *
 *  ```
 *    wasm_rt_memory_t my_memory;
 *    ...
 *    // Grow memory by 10 pages.
 *    uint32_t old_page_size = wasm_rt_grow_memory(&my_memory, 10);
 *    if (old_page_size == UINT32_MAX) {
 *      // Failed to grow memory.
 *    }
 *  ```
 */
uint32_t wasm_rt_grow_memory(wasm_rt_memory_t *, uint32_t pages);

/**
 * Free a Memory object.
 */
void wasm_rt_free_memory(wasm_rt_memory_t *);

/**
 * Initialize a funcref Table object with an element count of `elements` and a
 * maximum size of `max_elements`.
 *
 *  ```
 *    wasm_rt_funcref_table_t my_table;
 *    // 5 elements and a maximum of 10 elements.
 *    wasm_rt_allocate_funcref_table(&my_table, 5, 10);
 *  ```
 */
void wasm_rt_allocate_funcref_table(wasm_rt_funcref_table_t *,
                                    uint32_t elements,
                                    uint32_t max_elements);

/**
 * Free a funcref Table object.
 */
void wasm_rt_free_funcref_table(wasm_rt_funcref_table_t *);

/**
 * Initialize an externref Table object with an element count
 * of `elements` and a maximum size of `max_elements`.
 * Usage as per wasm_rt_allocate_funcref_table.
 */
void wasm_rt_allocate_externref_table(wasm_rt_externref_table_t *,
                                      uint32_t elements,
                                      uint32_t max_elements);

/**
 * Free an externref Table object.
 */
void wasm_rt_free_externref_table(wasm_rt_externref_table_t *);

/**
 * Grow a Table object by `delta` elements (giving the new elements the value
 * `init`), and return the previous element count. If this new element count is
 * greater than the maximum element count, the grow fails and 0xffffffffu
 * (UINT32_MAX) is returned instead.
 */
uint32_t wasm_rt_grow_funcref_table(wasm_rt_funcref_table_t *,
                                    uint32_t delta,
                                    wasm_rt_funcref_t init);
uint32_t wasm_rt_grow_externref_table(wasm_rt_externref_table_t *,
                                      uint32_t delta,
                                      wasm_rt_externref_t init);

#ifdef __cplusplus
}
#endif

#endif /* WASM_RT_H_ */
