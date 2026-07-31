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

#define                 aligned(ptr, aln)           (((uintptr_t)ptr + (aln - 1)) & ~(aln - 1))

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

// -==================================================- //
//                      FAST STRINGS                    //
// -==================================================- //

// NOTE(Joel Zbinden): The following is an implementation of an 
// interlaced string manager, it is omptimized for cache locality 
// and fast read access for loading multiple collided strings into 
// SIMD vectors at once. The only realistic case this is "fast" in 
// is vector accelerated string comparisons like the ones I take
// advantage of in my hash map implementation. 

// TODO(Joel Zbinden): Currently only have an 8 bit length 
// implementation for the 8 bit length keyed hashmap, will 
// need to implement higher precision lengths in the future.

typedef                 uint32_t                    fstr_t;
typedef                 array_t(uint8_t)            fstrman_t;

#define BASE_FSTR_MAX_LENGTH                        0xFF

fstrman_t               fstrman_create              (arena_t* arena, uint32_t size);
void                    fstrman_drop                (arena_t* arena, fstrman_t fstr_man);

fstr_t                  fstr_from_cstr              (fstrman_t fstr_man, char_ptr_t fmt);
fstr_t                  fstr_from                   (fstrman_t fstr_man, const char_ptr_t fmt, ...);
string_t                string_from_fstr            (arena_t* arena, fstrman_t fstr_man, fstr_t str);

array_t(fstr_t)         fstr_matchn                 (arena_t* arena, fstrman_t fstr_man, string_t* target, fstr_t start, uint32_t subject_count);

// -==================================================- //
//                        HASHMAPS                      //
// -==================================================- //

typedef struct hash_kvp_t hash_kvp_t;
struct hash_kvp_t {
    string_t            value;
    fstr_t              key;                        // NOTE(Joel Zbinden): If key size is zero, the KVP is free, else it's occupied.
};
#define empty_hash_kvp_t                            (hash_kvp_t) {0}

// NOTE(Joel Zbinden): The number of collision interlacers is 
// proportional to the block_size and the size is proportional 
// to the block_count or the  max_collitions parameter on creation.

typedef struct hashmap_t hashmap_t;

typedef uint64_t (*hash_func_t)(string_t* src);

#define BASE_FNV_PRIME                              16777619
#define BASE_FNV_OFFSET_BASE                        2166136261

uint64_t                hashmap_fnv1_hash           (string_t* src);

struct hashmap_t {
    array_t(hash_kvp_t) data;
    uint32_t            block_size;
    uint32_t            block_count;
    array_t(fstrman_t)  col_intrlcrs;                // NOTE(Joel Zbinden): These are the collision interlacers, they stores the keys in an interlaced pattern by collision.
    hash_func_t         hash_function;
};
#define empty_hashmap_t                             (hashmap_t) {0}

hashmap_t               hashmap_create              (arena_t* arena, uint32_t block_size, uint32_t max_collisions, hash_func_t hash_function);
hash_kvp_t*             hashmap_get_kvp             (arena_t* arena, hashmap_t* map, string_t* key, bool create);

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

fstrman_t fstrman_create(arena_t* arena, uint32_t size) {

    if (size == 0) {
        print_error(EINVAL, "fstrman_from");
        return nullptr;
    }

    fstrman_t result = array_create(arena, uint8_t, size * BASE_FSTR_MAX_LENGTH);
    if (result == nullptr) {
        print_error(errno, "fstrman_from: array_create");
        return nullptr;
    }

    return result;

}

void fstrman_drop(arena_t* arena, fstrman_t fstr_man) {
    
    array_drop(arena, fstr_man);

}

fstr_t fstr_from_cstr(fstrman_t fstr_man, char_ptr_t src) {

    uint8_t src_length = strlen(src) & BASE_FSTR_MAX_LENGTH;

    uint32_t current_id = array_size(fstr_man);
    uint32_t fstr_man_cap = array_capacity(fstr_man) / BASE_FSTR_MAX_LENGTH;
    
    for (size_t i = 0; i < src_length; i++)
        fstr_man[current_id + (i *  fstr_man_cap)] = src[i];
    return current_id;
    
}

fstr_t fstr_from(fstrman_t fstr_man, const char_ptr_t fmt, ...) {

    if (fstr_man == nullptr || fmt == nullptr) {
        print_error(EINVAL, "string_from");
        return UINT32_MAX;
    }

    va_list list;
    scratch_t scratch = {0};

    va_start(list, fmt);
    uint8_t result_size = vsnprintf(nullptr, 0, fmt, list) & BASE_FSTR_MAX_LENGTH;
    va_end(list);

    va_start(list, fmt);
    vsnprintf(scratch, result_size, fmt, list);
    va_end(list);

    return fstr_from_cstr(fstr_man, scratch);

}

string_t string_from_fstr(arena_t* arena, fstrman_t fstr_man, fstr_t str) {

    if (fstr_man == nullptr) {
        print_error(EINVAL, "string_from_fstr");
        return empty_string_t;
    }

    uint32_t fstr_man_cap = array_capacity(fstr_man) / BASE_FSTR_MAX_LENGTH;
    string_t result = string_from_size(arena, BASE_FSTR_MAX_LENGTH + 1);
    result.size = BASE_FSTR_MAX_LENGTH;

    for (size_t i = 0; i < result.size; i++)
        result.data[i] = fstr_man[str + (i * fstr_man_cap)];
    return result;

}

array_t(fstr_t) fstr_matchn(arena_t* arena, fstrman_t fstr_man, string_t* target, fstr_t start, uint32_t subject_count) {

    // NOTE(Joel Zbinden): This implementation of a vector 
    // accelerated interlaced 1 to multiple string comparison
    // only compares string contents as the overhead from 
    // finding and gathering matched strings would most likely 
    // slow this down.

    array_t(fstr_t) result = array_create(arena, fstr_t, subject_count);
    if (result == nullptr) {
        print_error(errno, "fstr_matchn: array_create");
        return nullptr;
    }

    const uint32_t iterations = subject_count >> 4;
    const uint32_t left_overs = subject_count &  3;

    const uint32_t left_over_start = iterations * 16;
    const uint32_t fstr_man_cap = array_capacity(fstr_man) / BASE_FSTR_MAX_LENGTH;

    for (uint32_t i = 0; i < iterations; i++) {

        uint16_t active = UINT16_MAX;
        for (uint8_t c = 0; c < (uint8_t)(target->size & BASE_FSTR_MAX_LENGTH); c++) {
            uchar_ptr_t ptr = fstr_man + start + (i * 16) + (fstr_man_cap * c);
            active &= simd_match_16char(target->data[c], ptr, 16, false);
            if (active == 0)
                break;
        }

        while (active) {
            uint32_t index = __builtin_ctz(active);
            result[array_size(result)++] = (left_over_start + index) + start;
            active &= active - 1;
        }

    }

    uint16_t active = UINT16_MAX;
    for (uint8_t c = 0; c < (uint8_t)(target->size & BASE_FSTR_MAX_LENGTH); c++) {
        uchar_ptr_t ptr = fstr_man + start + left_over_start + (fstr_man_cap * c);
        active &= simd_match_16char(target->data[c], ptr, left_overs, false);
        if (active == 0)
            break;
    }

    while (active) {
        uint32_t index = __builtin_ctz(active);
        result[array_size(result)++] = (left_over_start + index) + start;
        active &= active - 1;
    }

    return result;

}

uint64_t hashmap_fnv1_hash(string_t* src) {

    if (src == nullptr) {
        print_error(EINVAL, "hashmap_fnv1_hash");
        return UINT64_MAX;
    }

    uint32_t hash = BASE_FNV_OFFSET_BASE;
    unsigned char *walker = (unsigned char*)src->data;

    while (*walker) {
        hash ^= *walker;
        hash *= BASE_FNV_PRIME;
        walker++;
    }

    return (uint64_t)hash;

}

hashmap_t hashmap_create(arena_t* arena, uint32_t block_size, uint32_t max_collisions, hash_func_t hash_function) {

    // NOTE(Joel Zbinden): block_size must be a power of 2
    if (block_size & (block_size & ~(block_size - 1)) == block_size || hash_function == nullptr) {
        print_error(EINVAL, "hashmap_create: block_size maybe not power of 2?");
        return empty_hashmap_t;
    }

    hashmap_t result = empty_hashmap_t;

    result.block_size = block_size;
    result.block_count = max_collisions;

    if (arena == nullptr) {

        result.data = calloc(result.block_count * result.block_size, sizeof(hash_kvp_t));
        if (result.data == nullptr) {
            print_error(errno, "hashmap_create: calloc");
            return empty_hashmap_t;
        }

        goto finish;

    }

    result.data = arena_alloc(arena, result.block_count * result.block_size * sizeof(hash_kvp_t), 8);
    if (result.data == nullptr) {
        print_error(errno, "hashmap_create: calloc");
        return empty_hashmap_t;
    }

finish:

    result.col_intrlcrs = array_create(arena, fstrman_t, block_size);
    if (result.col_intrlcrs == nullptr) {
        print_error(errno, "hashmap_create: array_create");
        if (arena == nullptr)
            BASE_FREE(result.data);
        return empty_hashmap_t;
    }

    for (size_t i = 0; i < block_size; i++)
        result.col_intrlcrs[i] = fstrman_create(arena, max_collisions);
    return result;

}

hash_kvp_t* hashmap_get_kvp(arena_t* arena, hashmap_t* map, string_t* key, bool create) {

    hash_kvp_t* result = nullptr;

    if (map == nullptr || key == nullptr) {
        print_error(EINVAL, "hashmap_get_kvp");
        return nullptr;
    }

    // NOTE(Joel Zbinden): Since we know map->block_size is a power of 2 we can just mask against it.
    uint64_t hashed = map->hash_function(key) & map->block_size;   
    array_t(fstr_t) matches = fstr_matchn(arena, map->col_intrlcrs[hashed], key, 0, map->block_count);

    if (array_size(matches) == 0) {
        
        if (create == false)
            return nullptr;

        uchar_ptr_t empty_ptr = strchr(map->col_intrlcrs[hashed], 0);
        if (empty_ptr == nullptr) {
            print_error(ENODATA, "hashmap_get_kvp: strchr");
            return nullptr;
        }

        fstr_t empty = empty_ptr - map->col_intrlcrs[hashed];
        return &map->data[(empty * map->block_size) + hashed];

    }

    return &map->data[(matches[0] * map->block_size) + hashed];

}

#endif // BASE_IMPLEMENTATION

#if defined(__cplusplus)
#undef nullptr   
}
#endif // __cplusplus

#endif // BASE_H