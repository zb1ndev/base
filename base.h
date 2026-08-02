/*

    Version: 0.1
    Author: Joel Zbinden
    Release Date: 7/25/26

    ///////////////////////////////////////////////////////////////////////

    AI DISCLAIMER: AI use was restricted to documentation only, no code 
    generated was used.

    ///////////////////////////////////////////////////////////////////////

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    For more information, please refer to <https://unlicense.org>

    ///////////////////////////////////////////////////////////////////////

*/

#if !defined(BASE_H)
#define BASE_H

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include <stdbool.h>
#include <stdalign.h>
#include <time.h>

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
    #if !defined(__STDC_NO_ATOMICS__)
        #include <stdatomic.h>
        #define BASE_HAS_STDATOMIC 1
    #else
        #define BASE_HAS_STDATOMIC 0
    #endif // __STDC_NO_ATOMICS__
#else
    #define BASE_HAS_STDATOMIC 0
#endif // __STDC_VERSION__

#if !defined(BASE_CALLOC)
    #define BASE_CALLOC                             calloc
#endif // BASE_ALLOC

#if !defined(BASE_REALLOC)
    #define BASE_REALLOC                            realloc
#endif // BASE_REALLOC

#if !defined(BASE_FREE)
    #define BASE_FREE                               free
#endif // BASE_REALLOC

#if defined(EXIT_FAILURE)
    #undef EXIT_FAILURE
#endif // EXIT_FAILURE

#define EXIT_FAILURE -1

// -==================================================- //
//                    PLATFORM DEFINES                  //
// -==================================================- //

#if defined(_WIN32) || defined(_WIN64)
    #define BASE_PLATFORM_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #define BASE_PLATFORM_APPLE 1
#elif defined(__linux__)
    #define BASE_PLATFORM_LINUX 1
#elif defined(__ANDROID__)
    #define BASE_PLATFORM_ANDROID 1
#endif // _WIN32 || _WIN64

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
    #define BASE_ARCH_64BIT 1
#else
    #define BASE_ARCH_32BIT 1
#endif // _WIN64 || __x86_64__ || __ppc64__ || __aarch64__

// -==================================================- //
//                     RANDOM TYPES                     //
// -==================================================- //

typedef void*           void_ptr_t;
typedef int8_t*         char_ptr_t;
typedef uint8_t*        uchar_ptr_t;

typedef uint8_t         scratch_t[1024];

// -==================================================- //
//                      DIAGNOSTICS                     //
// -==================================================- //

typedef int16_t         err_t;

void                    print_error                 (err_t code, const char_ptr_t message);

typedef enum timer_cmd_t { 
    TC_START            = 1, 
    TC_END              = 2
} timer_cmd_t;

double                  timer                       (timer_cmd_t command);

// -==================================================- //
//                  STANDARD EXTENSION                  //
// -==================================================- //

#define KB                                          (1024)
#define MB                                          (KB * 1024)
#define GB                                          (MB * 1024)

#define internal                                    static
#define local_persist                               static
#define null                                        0
#define nullptr                                     ((void*)0)

void_ptr_t              recalloc                    (void_ptr_t ptr, uint64_t nsize, uint64_t osize);

#define                 aligned(ptr, aln)           (((uintptr_t)(ptr) + ((aln) - 1)) & ~((aln) - 1))

// -==================================================- //
//                        ARENAS                        //
// -==================================================- //

typedef struct arena_t arena_t;
struct arena_t {
    void_ptr_t          data;
    arena_t*            next;
    uint64_t            offset;
    uint64_t            capacity;
};
#define empty_arena_t                               (arena_t) {0}

arena_t                 arena_create                (uint64_t size);
void                    arena_drop                  (arena_t* arena);

void_ptr_t              arena_alloc                 (arena_t* arena, uint64_t size, uint16_t alignment);

// -==================================================- //
//                GENERIC DYNAMIC ARRAYS                //
// -==================================================- //

typedef struct array_head_t array_head_t;
struct array_head_t {
    uint32_t            type_size;
    uint32_t            capacity;
    uint32_t            size;
};

#define array_t(type)                               type*

#define array_head(arr)                             ((array_head_t*)(arr) - 1)
#define array_size(arr)                             array_head(arr)->size
#define array_capacity(arr)                         array_head(arr)->capacity
#define array_type(arr)                             array_head(arr)->type_size

void_ptr_t              array_create_f              (arena_t* arena, uint64_t type_size, uint64_t size);
err_t                   array_resize_f              (arena_t* arena, array_t(void_ptr_t) arr, uint64_t size);

#define array_create(arena, type, size)             (type*)array_create_f(arena, sizeof(type), (size))
#define array_resize(arena, arr, size)              array_resize_f(arena, (void_ptr_t*)&(arr), (size))
#define array_drop(arena, arr)                      if ((arena)==nullptr) free(array_head((arr)))

// -==================================================- //
//                       BITMASKS                       //
// -==================================================- //

typedef uint8_t         bitmask8_t;
typedef uint16_t        bitmask16_t;
typedef uint32_t        bitmask32_t;
typedef uint64_t        bitmask64_t;

typedef uint8_t         bitmask128_t                [16];
#define empty_bitmask128_t                          {0}

uint32_t                bitmask128_seek             (uint32_t* bit_out, uint32_t idx);
err_t                   bitmask128_set              (bitmask128_t bitmask, uint32_t idx, uint8_t val);
int8_t                  bitmask128_get              (bitmask128_t bitmask, uint32_t idx);

array_t(char)           get_binary_as_cstr          (uint8_t numbers[], uint16_t nnumbers);

// -==================================================- //
//                        STRINGS                       //
// -==================================================- //

#define strfmt                                      "%.*s"
#define strarg(str)                                 (int)str.size, str.data

typedef struct string_t string_t;
struct string_t {
    uchar_ptr_t         data;
    uint64_t            size;
};
#define empty_string_t                              (string_t) {0}
#define string_from_literal(data)                   (string_t) {data,(uint64_t)sizeof(data) - 1}
#define string_from_view(data, size)                (string_t) {data, size}

string_t                string_from                 (arena_t* arena, const char_ptr_t fmt, ...);
string_t                string_from_size            (arena_t* arena, uint64_t size);
void                    string_drop                 (arena_t* arena, string_t* ptr);

// -==================================================- //
//                          SIMD                        //
// -==================================================- //

#if defined(__x86_64__)
    #include <immintrin.h>
    #define BASE_SIMD_X86 1
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM64)
    #include <arm_neon.h>
    #define BASE_SIMD_NEON 1
#endif // __ARM_NEON || __ARM_NEON__ || _M_ARM64

#if defined(__ARM_FEATURE_SVE)
    #include <arm_sve.h>
    #define BASE_SIMD_SVE 1
#endif // __ARM_FEATURE_SVE

#if defined(__wasm_simd128__)
    #include <wasm_simd128.h>
    #define BASE_SIMD_WASM 1
#endif // __wasm_simd128__

bitmask16_t             simd_match_16char           (uint8_t target, array_t(uint8_t) subjects, uint8_t subject_count, bool diff);
bitmask8_t              simd_match_2u64             (uint64_t target, array_t(uint64_t) subjects, uint8_t subject_count, bool diff);

#define BASE_IMPLEMENTATION                         // TODO(Joel Zbinden): Remove #define for release & install
#if defined(BASE_IMPLEMENTATION)

void print_error(err_t code, const char_ptr_t message) {
    
    errno = code;
    perror(message);

}

double timer(timer_cmd_t command) {

    local_persist clock_t start_time = 0;
    
    switch (command) {

        case TC_START : {
            start_time = clock();
            return 0.0;
        }

        case TC_END : { 
            if (start_time == 0) 
                return 0.0;
            double result = ((double)(clock() - start_time) / CLOCKS_PER_SEC) * 1000.0;
            start_time = 0;
            return result;
        }

        default : {
            if (start_time == 0) 
                return 0.0;
            return ((double)(clock() - start_time) / CLOCKS_PER_SEC) * 1000.0;
        }

    }

}

void_ptr_t recalloc(void_ptr_t ptr, uint64_t nsize, uint64_t osize) {

    if (ptr == nullptr || nsize == 0) {
        print_error(EINVAL, "recalloc");
        return nullptr;
    }
    
    void_ptr_t result = BASE_REALLOC(ptr, nsize);
    if (result == nullptr) {
        print_error(errno, "recalloc: realloc");
        return nullptr;
    }

    if (nsize <= osize)
        return result;
    return memset(result + osize, 0, nsize - osize);

}

arena_t arena_create(uint64_t size) {

    if (size == 0) {
        print_error(EINVAL, "arena_create");
        return empty_arena_t;
    }

    return (arena_t) {
        .data = calloc(size, 1),
        .capacity = size,
        .next = nullptr,
        .offset = 0,
    };

}

void arena_drop(arena_t* arena) {

    if (arena == nullptr)
        return;
    
    if (arena->next != nullptr) {
        arena_drop(arena->next);
        BASE_FREE(arena->next);
    }

    if (arena->data != nullptr)
        BASE_FREE(arena->data);

}

void_ptr_t arena_alloc(arena_t* arena, uint64_t size, uint16_t alignment) {

    if (size == 0) {
        print_error(EINVAL, "arena_alloc");
        return nullptr;
    }

    arena_t* suitable_arena = arena;
    while (suitable_arena->capacity - suitable_arena->offset < size) {
        
        if (suitable_arena->next != nullptr) {
            suitable_arena = suitable_arena->next;
            continue;
        }

        uint64_t next_size = size <= suitable_arena->capacity 
            ? suitable_arena->capacity
            : size;

        suitable_arena->next = BASE_CALLOC(1, sizeof(arena_t));
        if (suitable_arena->next == nullptr) {
            print_error(errno, "arena_alloc: calloc");
            return nullptr;
        }

        *suitable_arena->next = arena_create(next_size);
        suitable_arena = suitable_arena->next;
        break;

    }

    void_ptr_t result = (void_ptr_t)aligned(suitable_arena->data + suitable_arena->offset, alignment);
    suitable_arena->offset += (result - (suitable_arena->data + suitable_arena->offset)) + size;

    assert(((uintptr_t)result & alignment - 1) == 0);

    return result;

}

void_ptr_t array_create_f(arena_t* arena, uint64_t type_size, uint64_t size) {

    if (size == 0 || type_size == 0) {
        print_error(EINVAL, "array_create_f");
        return nullptr;
    }

    array_head_t* head = nullptr;

    if (arena == nullptr) {

        head = BASE_CALLOC(sizeof(array_head_t) + (size * type_size), 1);
        if (head == nullptr) {
            print_error(errno, "array_create: calloc");
            return nullptr;
        }

        goto finish;

    }

    head = arena_alloc(arena, size * type_size, type_size);
    if (head == nullptr) {
        print_error(errno, "array_create: arena_alloc");
        return nullptr;
    }

finish:

    head->capacity = size;
    head->type_size = type_size;
    return (void_ptr_t)(head + 1);

}

err_t array_resize_f(arena_t* arena, void_ptr_t* arr, uint64_t size) {

    if (arr == nullptr) {
        print_error(EINVAL, "array_resize_f");
        return EXIT_FAILURE;
    }

    array_head_t* head = nullptr;
    if (arena == nullptr) {

        head = recalloc(array_head(*arr), sizeof(array_head_t) + (size * array_type(*arr)), sizeof(array_head_t) + (array_capacity(*arr) * array_type(*arr)));
        if (head == nullptr) {
            print_error(errno, "array_resize: recalloc");
            return EXIT_FAILURE;
        }

        goto finish;

    }

    // NOTE(Joel Zbinden): I dont want to deal with reallocations in my arena implementation right now...
    head = arena_alloc(arena, sizeof(array_head_t) + (size * array_type(*arr)), array_type(*arr));
    if (head == nullptr) {
        print_error(errno, "array_resize_f: arena_alloc");
        return EXIT_FAILURE;
    }

    uint64_t copy_size = size > array_capacity(*arr) ? array_capacity(*arr) : size;
    memcpy(head, array_head(*arr), sizeof(array_head_t) + (copy_size * array_type(*arr)));

finish:

    head->capacity = size;
    *arr = (void_ptr_t)(head + 1);
    return EXIT_SUCCESS;

}

uint32_t bitmask128_seek(uint32_t* bit_out, uint32_t idx) {

    // NOTE(Joel Zbinden): Takes the position of a bit in an extended bit 
    // mask and provides the index to the integer where the bit resides 
    // and the bit position within that integer.

    if (bit_out != nullptr)
        *bit_out = idx & 7;
    return idx >> 3;

}

err_t bitmask128_set(bitmask128_t bitmask, uint32_t idx, uint8_t val) {

    if (bitmask == nullptr || idx > 127) {
        print_error(EINVAL, "bitmask_set");
        return EXIT_FAILURE;
    }
    
    uint32_t block, bit;
    block = bitmask128_seek(&bit, idx);
    bitmask[block] |= ((val > 0 ? 1 : 0) << bit);

    return EXIT_SUCCESS;

}

int8_t bitmask128_get(bitmask128_t bitmask, uint32_t idx) {

    if (bitmask == nullptr || idx > 127) {
        print_error(EINVAL, "bitmask_set");
        return EXIT_FAILURE;
    }

    uint32_t block, bit;
    block = bitmask128_seek(&bit, idx);
    return (bitmask[block] >> bit) & 1;

}

array_t(char) get_binary_as_cstr(uint8_t numbers[], uint16_t nnumbers) {

    const uint16_t PRECISION = 8;
    array_t(char) result = array_create(nullptr, char, (PRECISION * nnumbers) + 1);
    for (uint16_t n = 0; n < nnumbers; n++)
        for (uint16_t i = 0; i < PRECISION; i++)
            result[(n * PRECISION) + i] = (numbers[n] >> i) & 1 == 1 ? '1' : '0';
    return result;

}

string_t string_from(arena_t* arena, const char_ptr_t fmt, ...) {

    if (fmt == nullptr) {
        print_error(EINVAL, "string_from");
        return empty_string_t;
    }

    va_list list;
    string_t result = empty_string_t;

    va_start(list, fmt);
    result.size = vsnprintf(nullptr, 0, fmt, list);
    va_end(list);

    if (arena == nullptr) {

        result.data = BASE_CALLOC(result.size, sizeof(char));
        if (result.data == nullptr) {
            print_error(errno, "string_from: calloc");
            return empty_string_t;
        }

        goto finish;

    }

    result.data = arena_alloc(arena, result.size, 1);
    if (result.data == nullptr) {
        print_error(errno, "string_from: arena_alloc");
        return empty_string_t;
    }

finish:

    va_start(list, fmt);
    vsnprintf(result.data, result.size + 1, fmt, list);
    va_end(list);

    return result;

}

string_t string_from_size(arena_t* arena, uint64_t size) {

    if (size == 0) {
        print_error(EINVAL, "string_from_size");
        return empty_string_t;
    }

    string_t result = empty_string_t;

    if (arena == nullptr) {

        result.data = BASE_CALLOC(size, sizeof(char));
        if (result.data == nullptr) {
            print_error(errno, "string_from_size: calloc");
            return empty_string_t;
        }

        return result;

    }

    result.data = arena_alloc(arena, size, 1);
    if (result.data == nullptr) {
        print_error(errno, "string_from: arena_alloc");
        return empty_string_t;
    }

    return result;

}

void string_drop(arena_t* arena, string_t* ptr) {

    if (arena == nullptr)
        free(ptr->data);

}

bitmask16_t simd_match_16char(uint8_t target, uint8_t subjects[16], uint8_t subject_count, bool diff) {

    // NOTE(Joel Zbinden): subjects must be 16 byte aligned as 
    // the aligned load SIMD instruction requires this.
    assert(((uintptr_t)subjects & 15) == 0);

#if defined(BASE_SIMD_X86)

    #if !defined(__GNUC__) && !defined(__clang__)
        #error "This SIMD implementation requires GCC or Clang"
    #endif

    __m128i target_vector = _mm_set1_epi8(target);
    __m128i subject_vector = _mm_load_si128((const __m128i*)subjects);
    __m128i comparison_vector = _mm_cmpeq_epi8(target_vector, subject_vector);
    bitmask16_t result = (bitmask16_t)_mm_movemask_epi8(comparison_vector);

    return (diff == true) 
        ? ~result
        : result;

#endif // BASE_SIMD_X86

    return 0;

}

bitmask8_t simd_match_2u64(uint64_t target, uint64_t subjects[2], uint8_t subject_count, bool diff) {

    // NOTE(Joel Zbinden): subjects must be 16 byte aligned as 
    // the aligned load SIMD instruction requires this.
    assert(((uintptr_t)subjects & 15) == 0);

#if defined(BASE_SIMD_X86)

    #if !defined(__GNUC__) && !defined(__clang__)
        #error "This SIMD implementation requires GCC or Clang"
    #endif

    __m128i target_vector = _mm_set1_epi64x(target);
    __m128i subject_vector = _mm_load_si128((const __m128i*)subjects);
    __m128i cmp = _mm_cmpeq_epi64(target_vector, subject_vector);
    bitmask8_t result = (bitmask8_t)_mm_movemask_pd(_mm_castsi128_pd(cmp));

    return (diff == true) 
        ? ~result
        : result; 
   
#endif // BASE_SIMD_X86

    return 0;

}

#endif // BASE_IMPLEMENTATION

#if defined(__cplusplus)
#undef nullptr   
}
#endif // __cplusplus

#endif // BASE_H