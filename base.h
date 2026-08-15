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
        #define BASE_HAS_STDATOMIC
    #endif // __STDC_NO_ATOMICS__
#endif // __STDC_VERSION__

#define success                                     0
#define failure                                     -1

// -==================================================- //
//                    PLATFORM DEFINES                  //
// -==================================================- //

#if defined(_WIN32) || defined(_WIN64)
    #define BASE_PLATFORM_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #define BASE_PLATFORM_APPLE
#elif defined(__linux__)
    #define BASE_PLATFORM_LINUX
#elif defined(__ANDROID__)
    #define BASE_PLATFORM_ANDROID
#endif // _WIN32 || _WIN64

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
    #define BASE_ARCH_64BIT
#else
    #define BASE_ARCH_32BIT
#endif // _WIN64 || __x86_64__ || __ppc64__ || __aarch64__

// -==================================================- //
//                     RANDOM TYPES                     //
// -==================================================- //

typedef void*           void_ptr_t;
typedef int8_t*         i8_ptr_t;
typedef uint8_t*        u8_ptr_t;

typedef uint8_t         scratch_t[1024];

// -==================================================- //
//                      DIAGNOSTICS                     //
// -==================================================- //

typedef int16_t         err_t;
void                    print_error                 (err_t code, const i8_ptr_t message);

typedef enum timer_cmd_t {
    TC_GET              = 0,
    TC_START            = 1, 
    TC_END              = 2
} timer_cmd_t;

uint64_t                timer                       (timer_cmd_t command);

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
i8_ptr_t                arena_strdup                (arena_t* arena, i8_ptr_t src);

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

#define                 aligned(ptr, aln)           (((uintptr_t)(ptr) + ((aln) - 1)) & ~((aln) - 1))
void_ptr_t              recalloc                    (void_ptr_t ptr, uint64_t nsize, uint64_t osize);

typedef uint64_t alloc_head_t;

void_ptr_t              base_alloc                  (arena_t* arena, uint64_t size, uint16_t alignment);
uint64_t                base_alloc_get_size         (arena_t* arena, void_ptr_t ptr, uint16_t alignment);
void_ptr_t              base_realloc                (arena_t* arena, void_ptr_t prev, uint64_t size, uint16_t alignment);
void                    base_free                   (arena_t* arena, void_ptr_t ptr, uint16_t alignment);

i8_ptr_t                base_strdup                 (arena_t* arena, i8_ptr_t src);

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
#define array_create(arena, type, size)             (type*)array_create_f(arena, sizeof(type), (size))

err_t                   array_resize_f              (arena_t* arena, array_t(void_ptr_t) arr, uint64_t size);
#define array_resize(arena, arr, size)              array_resize_f(arena, (void_ptr_t*)&(arr), (size))

#define array_drop(arena, arr)                      if ((arena)==nullptr&&arr!=nullptr) free(array_head((arr)))

// -==================================================- //
//                       BITMASKS                       //
// -==================================================- //

typedef uint8_t         bitmask8_t;
typedef uint16_t        bitmask16_t;
typedef uint32_t        bitmask32_t;
typedef uint64_t        bitmask64_t;

typedef uint64_t        bitmask128_t                [2];
#define empty_bitmask128_t                          {0}

uint32_t                bitmask128_seek             (uint32_t* bit_out, uint32_t idx);
err_t                   bitmask128_set              (bitmask128_t bitmask, uint32_t idx, uint8_t val);
int8_t                  bitmask128_get              (bitmask128_t bitmask, uint32_t idx);

i8_ptr_t                bitmask_to_cstr             (arena_t* arena, uint64_t numbers[], uint16_t nnumbers);

// -==================================================- //
//                        STRINGS                       //
// -==================================================- //

#define strfmt                                      "%.*s"
#define strarg(str)                                 (int)(str).size, (str).data

typedef struct utf8_t utf8_t;
struct utf8_t {
    u8_ptr_t            data;
    uint64_t            size;
};
#define empty_utf8_t                                (utf8_t) {0}
#define utf8_from_literal(data)                     (utf8_t) {data,(uint64_t)sizeof(data) - 1}
#define utf8_from_view(data, size)                  (utf8_t) {data,(uint64_t)size}

utf8_t                  utf8_from                   (arena_t* arena, const i8_ptr_t fmt, ...);
utf8_t                  utf8_from_size              (arena_t* arena, uint64_t size);
void                    utf8_drop                   (arena_t* arena, utf8_t* ptr);

// -==================================================- //
//                          SIMD                        //
// -==================================================- //

#if defined(__x86_64__)
    #include <immintrin.h>
    #include <x86intrin.h>
    #define BASE_SIMD_X86
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM64)
    #include <arm_neon.h>
    #define BASE_SIMD_NEON
#endif // __ARM_NEON || __ARM_NEON__ || _M_ARM64

#if defined(__ARM_FEATURE_SVE)
    #include <arm_sve.h>
    #define BASE_SIMD_SVE
#endif // __ARM_FEATURE_SVE

#if defined(__wasm_simd128__)
    #include <wasm_simd128.h>
    #define BASE_SIMD_WASM
#endif // __wasm_simd128__

bitmask16_t             simd_match_16char           (uint8_t target, array_t(uint8_t) subjects, uint8_t subject_count, bool diff);
bitmask8_t              simd_match_2u64             (uint64_t target, array_t(uint64_t) subjects, uint8_t subject_count, bool diff);

// -==================================================- //
//                          ZENMAP                      //
// -==================================================- //

typedef struct collisions_t collisions_t;
struct collisions_t {
    u8_ptr_t            data, interlaced;           // NOTE(Joel Zbinden): must be 16 byte aligned, only interlaces the first character of strings held in data.
    uint32_t            max, key_shift, key_max;    // NOTE(Joel Zbinden): must be a power of 2.
    array_t(uint32_t)   free_list;
};
#define empty_collisions_t                          (collisions_t) {0}  

collisions_t            collisions_create           (arena_t* arena, uint32_t max_collisions, uint32_t max_key_length);
void                    collisions_drop             (arena_t* arena, collisions_t* collisions);

uint32_t                collisions_add              (collisions_t* collisions, utf8_t data);
uint32_t                collisions_find             (arena_t* arena, collisions_t* collisions, utf8_t data);
void                    collisions_remove           (collisions_t* collisions, uint32_t id);

typedef uint32_t (*zenmap_hfunc_t)(utf8_t* string);

#define BASE_FNV1_HASH_PRIME                        16777619U
#define BASE_FNV1_HASH_BASE                         2166136261U
uint32_t                zenmap_fnv1_hash            (utf8_t* string);

typedef struct zenmap_t zenmap_t;
struct zenmap_t {

    utf8_t*             key_data;
    void_ptr_t          value_data;

    collisions_t*       collisions;
    zenmap_hfunc_t      hashing_function;
    uint32_t            block_size, block_count;    // NOTE(Joel Zbinden): ^2.

    uint64_t            key_count;
    uint16_t            max_key_length;
    uint16_t            type_alignment;
    uint32_t            type_size;

};
#define empty_zenmap_t                              (zenmap_t) {0}

zenmap_t                zenmap_create_f             (arena_t* arena, uint32_t type_size, uint16_t type_alignment, uint32_t block_size, uint32_t block_count, uint16_t max_key_length, zenmap_hfunc_t func);
#define zenmap_create_f(arena, type, algn, block_size, block_count, max_key_length, hash)                       \
    zenmap_create_f((arena), (sizeof(type)), (algn), (block_size), (block_count), (max_key_length), (hash))

void_ptr_t              zenmap_get_f                (zenmap_t* map, utf8_t* key, bool create);
#define zenmap_get(map, type, key, create)                                                                      \
    (type*)zenmap_get_f((map), (key), (create))

err_t                   zenmap_remove               (zenmap_t* map, utf8_t* key);
bool                    zenmap_has_key              (zenmap_t* map, utf8_t* key);

array_t(utf8_t)         zenmap_get_keys             (zenmap_t* map);
array_t(void)           zenmap_get_values           (zenmap_t* map);

uint64_t                zenmap_get_memsize          (zenmap_t* map);
uint64_t                zenmap_get_size             (zenmap_t* map);
void                    zenmap_drop                 (arena_t* arena, zenmap_t* map);

#if defined(BASE_IMPLEMENTATION)

void print_error(err_t code, const i8_ptr_t message) {
    
    errno = code;
    perror(message);

}

uint64_t timer(timer_cmd_t command) {

    local_persist uint64_t start_time = 0;
    
    if (command == TC_START) {
        start_time = __rdtsc();
        return 0;
    }

    if (start_time == 0)
        return 0;
    double result = (__rdtsc() - start_time);

    if (command == TC_END)
        start_time = 0;
    return result;

}

void_ptr_t recalloc(void_ptr_t ptr, uint64_t nsize, uint64_t osize) {

    if (ptr == nullptr || nsize == 0) {
        print_error(EINVAL, "recalloc");
        return nullptr;
    }
    
    void_ptr_t result = realloc(ptr, nsize);
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
        free(arena->next);
    }

    if (arena->data != nullptr)
        free(arena->data);

}

void_ptr_t arena_alloc(arena_t* arena, uint64_t size, uint16_t alignment) {

    if (size == 0) {
        print_error(EINVAL, "arena_alloc");
        return nullptr;
    }

    arena_t* suitable_arena = arena;
    while (suitable_arena->capacity - suitable_arena->offset < size + (alignment - 1)) {
        
        assert(suitable_arena->offset <= suitable_arena->capacity);
        
        if (suitable_arena->next != nullptr) {
            suitable_arena = suitable_arena->next;
            continue;
        }

        uint64_t next_size = size <= suitable_arena->capacity 
            ? suitable_arena->capacity
            : size;

        suitable_arena->next = calloc(1, sizeof(arena_t));
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

i8_ptr_t arena_strdup(arena_t* arena, i8_ptr_t src) {

    if (arena == nullptr || src == nullptr) {
        print_error(EINVAL, "arena_strdup");
        return nullptr;
    }

    uint32_t src_length = strlen(src);
    i8_ptr_t result = arena_alloc(arena, src_length + 1, 1);
    for (uint32_t i = 0; i < src_length; i++)
        result[i] = src[i];
    return result;

}

void_ptr_t base_alloc(arena_t* arena, uint64_t size, uint16_t alignment) {

    if (size == 0) {
        print_error(EINVAL, "base_realloc");
        return nullptr;
    } 

    uint64_t real_size = size + (alignment - 1) + sizeof(alloc_head_t);

    if (arena == nullptr) {

        alloc_head_t* head = aligned_alloc(alignment, real_size);
        if (head == nullptr) {
            print_error(errno, "base_alloc: aligned_alloc");
            return nullptr;
        }

        memset(head, 0, real_size);
        *head = size;
        return (void_ptr_t)aligned((void_ptr_t)(head + 1), alignment);

    }

    alloc_head_t* head = arena_alloc(arena, real_size, alignment);
    if (head == nullptr) {
        print_error(errno, "base_alloc: arena_alloc");
        return nullptr;
    }

    *head = size;
    return (void_ptr_t)aligned((void_ptr_t)(head + 1), alignment);

}

uint64_t base_alloc_get_size(arena_t* arena, void_ptr_t ptr, uint16_t alignment) {

    if (ptr == nullptr) {
        print_error(EINVAL, "base_realloc");
        return null;
    }

    assert(((uintptr_t)ptr & (alignment - 1)) == 0);
    uint64_t head_offset = aligned(sizeof(alloc_head_t), alignment);
    return *(alloc_head_t*)(ptr - (void_ptr_t)head_offset);

}

void_ptr_t base_realloc(arena_t* arena, void_ptr_t prev, uint64_t size, uint16_t alignment) {

    if (prev == nullptr || size == 0) {
        print_error(EINVAL, "base_realloc");
        return nullptr;
    } 

    assert(((uintptr_t)prev & (alignment - 1)) == 0);
    uint64_t head_offset = aligned(sizeof(alloc_head_t), alignment);
    uint64_t real_size = size + (alignment - 1) + sizeof(alloc_head_t);
    alloc_head_t head = *(alloc_head_t*)(prev - (void_ptr_t)head_offset);

    if (size <= head) 
        return prev;

    if (arena == nullptr) {

        alloc_head_t* result_head = realloc((alloc_head_t*)(prev - (void_ptr_t)head_offset), real_size);
        if (result_head == nullptr) {
            print_error(errno, "base_realloc: realloc");
            return nullptr;
        }

        *result_head = size;
        void_ptr_t result = (void_ptr_t)aligned((void_ptr_t)(result_head + 1), alignment); 
        
        if (size <= head) 
            return result;
        return memset(result + head, 0, size - head);

    }

    void_ptr_t result = base_alloc(arena, size, alignment);
    if (result == nullptr) {
        print_error(errno, "base_realloc: base_alloc");
        return nullptr;
    }

    return memcpy(result, prev, head);

}

i8_ptr_t base_strdup(arena_t* arena, i8_ptr_t src) {

    if (arena == null)
        return strdup(src);
    return arena_strdup(arena, src);

}

void base_free(arena_t* arena, void_ptr_t ptr, uint16_t alignment) {
    
    if (arena != nullptr)
        return;

    assert((uintptr_t)ptr & (alignment - 1) == 0);
    uint64_t head_offset = aligned(sizeof(alloc_head_t), alignment);
    free((alloc_head_t*)(ptr - (void_ptr_t)head_offset));

}

__attribute__((noinline)) // NOTE(Joel Zbinden): GCC giving false positives.
void_ptr_t array_create_f(arena_t* arena, uint64_t type_size, uint64_t size) {

    if (size == 0 || type_size == 0) {
        print_error(EINVAL, "array_create_f");
        return nullptr;
    }

    array_head_t* head = nullptr;

    if (arena == nullptr) {

        head = calloc(sizeof(array_head_t) + (size * type_size), 1);
        if (head == nullptr) {
            print_error(errno, "array_create_f: calloc");
            return nullptr;
        }

        goto finish;

    }

    head = (array_head_t*)arena_alloc(arena, sizeof(array_head_t) + (size * type_size), type_size);
    if (head == nullptr) {
        print_error(errno, "array_create_f: arena_alloc");
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
        return failure;
    }

    array_head_t* head = nullptr;
    if (arena == nullptr) {

        head = recalloc(array_head(*arr), sizeof(array_head_t) + (size * array_type(*arr)), sizeof(array_head_t) + (array_capacity(*arr) * array_type(*arr)));
        if (head == nullptr) {
            print_error(errno, "array_resize_f: recalloc");
            return failure;
        }

        goto finish;

    }

    // NOTE(Joel Zbinden): I dont want to deal with reallocations in my arena implementation right now...
    head = arena_alloc(arena, sizeof(array_head_t) + (size * array_type(*arr)), array_type(*arr));
    if (head == nullptr) {
        print_error(errno, "array_resize_f: arena_alloc");
        return failure;
    }

    uint64_t copy_size = size > array_capacity(*arr) ? array_capacity(*arr) : size;
    memcpy(head, array_head(*arr), sizeof(array_head_t) + (copy_size * array_type(*arr)));

finish:

    head->capacity = size;
    *arr = (void_ptr_t)(head + 1);
    return success;

}

uint32_t bitmask128_seek(uint32_t* bit_out, uint32_t idx) {

    // NOTE(Joel Zbinden): takes the position of a bit in an extended bit 
    // mask and provides the index to the integer where the bit resides 
    // and the bit position within that integer.

    if (bit_out != nullptr)
        *bit_out = idx & 63;
    return idx >> 6;

}

err_t bitmask128_set(bitmask128_t bitmask, uint32_t idx, uint8_t val) {

    if (bitmask == nullptr || idx > 127) {
        print_error(EINVAL, "bitmask_set");
        return failure;
    }
    
    uint32_t block, bit;
    block = bitmask128_seek(&bit, idx);
    bitmask[block] |= ((val > 0 ? 1 : 0) << bit);

    return success;

}

int8_t bitmask128_get(bitmask128_t bitmask, uint32_t idx) {

    if (bitmask == nullptr || idx > 127) {
        print_error(EINVAL, "bitmask_set");
        return failure;
    }

    uint32_t block, bit;
    block = bitmask128_seek(&bit, idx);
    return (bitmask[block] >> bit) & 1;

}

i8_ptr_t bitmask_to_cstr(arena_t* arena, uint64_t numbers[], uint16_t nnumbers) {

    const uint16_t precision = 64;
    if (nnumbers == 0 || numbers == nullptr) {
        print_error(EINVAL, "bitmask_to_cstr");
        return nullptr;
    }

    if (arena == nullptr) {

        i8_ptr_t result = calloc((precision * nnumbers) + 1, sizeof(char));
        if (result == nullptr) {
            print_error(errno, "bitmask_to_cstr: calloc");
            return nullptr;
        }

        goto finish;
    }

    i8_ptr_t result = arena_alloc(arena, (precision * nnumbers) + 1, 1);
    if (result == nullptr) {
        print_error(errno, "bitmask_to_cstr: arena_alloc");
        return nullptr;
    }

finish:

    for (uint16_t n = 0; n < nnumbers; n++) {
        for (uint16_t i = 0; i < precision; i++) {
            uint16_t idx = (n << 6) + i; 
            result[idx] = ((numbers[n] >> i) & 1 == 1 
                ? '1' 
                : '0'
            );
        }
    }

    return result;

}

utf8_t utf8_from(arena_t* arena, const i8_ptr_t fmt, ...) {

    if (fmt == nullptr) {
        print_error(EINVAL, "utf8_from");
        return empty_utf8_t;
    }

    va_list list;
    utf8_t result = empty_utf8_t;

    va_start(list, fmt);
    result.size = vsnprintf(nullptr, 0, fmt, list) + 1;
    va_end(list);

    if (arena == nullptr) {

        result.data = calloc(result.size + 1, sizeof(char));
        if (result.data == nullptr) {
            print_error(errno, "utf8_from: calloc");
            return empty_utf8_t;
        }

        goto finish;

    }

    result.data = arena_alloc(arena, result.size + 1, 1);
    if (result.data == nullptr) {
        print_error(errno, "utf8_from: arena_alloc");
        return empty_utf8_t;
    }

finish:

    va_start(list, fmt);
    vsnprintf(result.data, result.size + 1, fmt, list);
    va_end(list);

    return result;

}

utf8_t utf8_from_size(arena_t* arena, uint64_t size) {

    if (size == 0) {
        print_error(EINVAL, "utf8_from_size");
        return empty_utf8_t;
    }

    utf8_t result = empty_utf8_t;

    if (arena == nullptr) {

        result.data = calloc(size, sizeof(char));
        if (result.data == nullptr) {
            print_error(errno, "utf8_from_size: calloc");
            return empty_utf8_t;
        }

        return result;

    }

    result.data = arena_alloc(arena, size, 1);
    if (result.data == nullptr) {
        print_error(errno, "utf8_from_size: arena_alloc");
        return empty_utf8_t;
    }

    return result;

}

void utf8_drop(arena_t* arena, utf8_t* ptr) {

    if (arena != nullptr || ptr == nullptr)
        return;

    if (ptr->data != nullptr)
        free(ptr->data);

}

bitmask16_t simd_match_16char(uint8_t target, uint8_t subjects[16], uint8_t subject_count, bool diff) {

    // NOTE(Joel Zbinden): subjects must be 16 byte aligned as 
    // the aligned load SIMD instruction requires this.
    assert(((uintptr_t)subjects & 15) == 0);

#if defined(BASE_SIMD_NEON)

    #error ARM Neon is currently not implemented.

#endif // BASE_SIMD_NEON

#if defined(BASE_SIMD_SVE)

    #error ARM SVE is currently not implemented.

#endif // BASE_SIMD_SVE

#if defined(BASE_SIMD_WASM)

    #error WASM SIMD is currently not implemented.

#endif // BASE_SIMD_WASM

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

#if defined(BASE_SIMD_NEON)

    #error ARM Neon is currently not implemented.

#endif // BASE_SIMD_NEON

#if defined(BASE_SIMD_SVE)

    #error ARM SVE is currently not implemented.

#endif // BASE_SIMD_SVE

#if defined(BASE_SIMD_WASM)

    #error WASM SIMD is currently not implemented.

#endif // BASE_SIMD_WASM

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

collisions_t collisions_create(arena_t* arena, uint32_t max_collisions, uint32_t max_key_length) {

    // NOTE(Joel Zbinden): Number of blocks or collisions and 
    // maximum key length must be a powers of 2.
    assert((max_collisions & (max_collisions - 1)) == 0);
    assert((max_key_length & (max_key_length - 1)) == 0);

    if (max_collisions == 0 || max_key_length == 0) {
        print_error(EINVAL, "collisions_create");
        return empty_collisions_t;
    }

    collisions_t result = empty_collisions_t;    
    result.max = max_collisions;
    result.key_max = max_key_length;
    result.key_shift = __builtin_ctz(max_key_length);

    result.free_list = array_create(arena, uint32_t, max_collisions);
    for (size_t i = max_collisions; i > 0; i--)
        result.free_list[array_size(result.free_list)++] = i;
     
    if (arena == nullptr) {

        result.data = calloc(max_collisions << result.key_shift, sizeof(uint8_t));
        if (result.data == nullptr) {
            print_error(errno, "collision_create: calloc");
            return empty_collisions_t;
        }

        result.interlaced = aligned_alloc(16, max_collisions);
        if (result.interlaced == nullptr) {
            print_error(errno, "collision_create: arena_alloc");
            return empty_collisions_t;
        }

        memset(result.interlaced, 0, max_collisions);
        return result;

    }

    result.data = arena_alloc(arena, max_collisions << result.key_shift, 1);
    if (result.data == nullptr) {
        print_error(errno, "collision_create: arena_alloc");
        return empty_collisions_t;
    }

    result.interlaced = arena_alloc(arena, max_collisions, 16);
    if (result.interlaced == nullptr) {
        print_error(errno, "collision_create: arena_alloc");
        return empty_collisions_t;
    }

    return result;

}

void collisions_drop(arena_t* arena, collisions_t* collisions) {

    if (arena != nullptr || collisions == nullptr)
        return;

    if (collisions->data != nullptr)
        free(collisions->data);
    if (collisions->interlaced != nullptr)
        free(collisions->interlaced);   
    array_drop(arena, collisions->free_list);

}

uint32_t collisions_add(collisions_t* collisions, utf8_t data) {

    if (collisions == nullptr || data.data == nullptr || data.size == 0) {
        print_error(EINVAL, "collisions_add");
        return UINT32_MAX;
    }
  
    if ((array_size(collisions->free_list) - 1) < 0) {
        print_error(ENOMEM, "collisions_add");
        return UINT32_MAX;
    }

    uint32_t result = collisions->free_list[array_size(collisions->free_list)--];
    __builtin_prefetch(collisions->data + (result << collisions->key_shift), 1, 1); // NOTE(Joel Zbinden): might not be doing much, but I tried.

    collisions->interlaced[result] = data.data[0];
    memcpy(collisions->data + (result << collisions->key_shift), data.data, data.size);
    collisions->data[(result << collisions->key_shift) + data.size] = '\0';
    return result;

}

uint32_t collisions_find(arena_t* arena, collisions_t* collisions, utf8_t data) {

    if (collisions == nullptr || data.data == nullptr || data.size == 0) {
        print_error(EINVAL, "collisions_find");
        return UINT32_MAX;
    }
    
    uint32_t result         = UINT32_MAX;
    uint32_t passes         = collisions->max >> 4;
    uint32_t chars_left     = collisions->max & 15;
 
    for (uint32_t i = 0; i < passes; i++) {
        for (bitmask16_t mask = simd_match_16char(data.data[0], collisions->interlaced + (i << 4), 16, false); mask; mask &= mask - 1) {
            uint32_t id = (i << 4) + __builtin_ctz(mask);
            if (strcmp(data.data, collisions->data + (id << collisions->key_shift)) == 0)
                return id;
        }
    }

    for (bitmask16_t mask = simd_match_16char(data.data[0], collisions->interlaced + (passes << 4), chars_left, false); mask; mask &= mask - 1) {
        uint32_t id = (passes << 4) + __builtin_ctz(mask);
        if (strcmp(data.data, collisions->data + (id << collisions->key_shift)) == 0)
            return id;
    }

    return UINT32_MAX;

}

void collisions_remove(collisions_t* collisions, uint32_t id) {

    if (collisions == nullptr || id == UINT32_MAX) {
        print_error(EINVAL, "collisions_remove");
        return;
    }

    collisions->interlaced[id] = 0;
    collisions->free_list[array_size(collisions->free_list)++] = id;
    memset(collisions->data + (id << collisions->key_shift), 0, collisions->key_max);

}

uint32_t zenmap_fnv1_hash(utf8_t* string) {

    uint32_t hash = BASE_FNV1_HASH_BASE;
    for (size_t i = 0; i < string->size; i++) {
        hash = hash * BASE_FNV1_HASH_PRIME;
        hash = hash ^ string->data[i];
    }
    return hash;

}

zenmap_t zenmap_create (
    arena_t* arena, 
    uint32_t type_size, 
    uint16_t type_alignment, 
    uint32_t block_size, 
    uint32_t block_count, 
    uint16_t max_key_length, 
    zenmap_hfunc_t func
) {

    if (block_size == 0 || block_count == 0) {
        print_error(EINVAL, "zenmap_create");
        return empty_zenmap_t;
    }

    zenmap_t result             = empty_zenmap_t;
    result.block_count          = block_count;                          // NOTE(Joel Zbinden): ^2
    result.block_size           = block_size;                           // NOTE(Joel Zbinden): ^2
    result.max_key_length       = max_key_length;                       // NOTE(Joel Zbinden): ^2
    result.type_alignment       = type_alignment;
    result.type_size            = type_size;
    result.hashing_function     = func;

    uint64_t value_data_size    = block_count * block_size * type_size;
    uint64_t key_data_size      = block_count * block_size * sizeof(utf8_t);

    if (arena == nullptr) {

        result.key_data = aligned_alloc(8, key_data_size);
        if (result.key_data == nullptr) {
            print_error(errno, "zenmap_create: arena_alloc");
            return empty_zenmap_t;
        }
        memset(result.key_data, 0, key_data_size);

        result.value_data = aligned_alloc(type_alignment, value_data_size);
        if (result.value_data == nullptr) {
            print_error(errno, "zenmap_create: arena_alloc");
            return empty_zenmap_t;
        }
        memset(result.value_data, 0, value_data_size);

        result.collisions = aligned_alloc(8, sizeof(collisions_t) * block_size);
        if (result.collisions == nullptr) {
            print_error(errno, "zenmap_create: aligned_alloc");
            return empty_zenmap_t;
        }
        
        for (uint32_t i = 0; i < block_size; i++)
            result.collisions[i] = collisions_create(nullptr, block_count, max_key_length);
        return result;

    }

    result.key_data = arena_alloc(arena, key_data_size, 8);
    if (result.key_data == nullptr) {
        print_error(errno, "zenmap_create: arena_alloc");
        return empty_zenmap_t;
    }

    result.value_data = arena_alloc(arena, value_data_size, type_alignment);
    if (result.value_data == nullptr) {
        print_error(errno, "zenmap_create: arena_alloc");
        return empty_zenmap_t;
    }
    
    result.collisions = arena_alloc(arena, sizeof(collisions_t) * block_size, 8);
    if (result.collisions == nullptr) {
        print_error(errno, "zenmap_create: arena_alloc");
        return empty_zenmap_t;
    }
    
    for (uint32_t i = 0; i < block_size; i++)
        result.collisions[i] = collisions_create(arena, block_count, max_key_length);
    return result;

}

void_ptr_t zenmap_get_f(zenmap_t* map, utf8_t* key, bool create) {

    assert(0 && "TODO(Joel Zbinden): implement zenmap_get_f");
    return nullptr;

}

err_t zenmap_remove(zenmap_t* map, utf8_t* key) {

    assert(0 && "TODO(Joel Zbinden): implement zenmap_remove");
    return failure;

}

bool zenmap_has_key(zenmap_t* map, utf8_t* key) {

    assert(0 && "TODO(Joel Zbinden): implement zenmap_has_key");
    return false;

}

array_t(utf8_t) zenmap_get_keys(zenmap_t* map) {

    assert(0 && "TODO(Joel Zbinden): implement zenmap_get_keys");
    return nullptr;

}

array_t(void) zenmap_get_values(zenmap_t* map) {

    assert(0 && "TODO(Joel Zbinden): implement zenmap_get_values");
    return nullptr;

}

// NOTE(Joel Zbinden): does not compute padding
uint64_t zenmap_get_memsize(zenmap_t* map) {

    uint64_t collisions_free_list   = (sizeof(array_head_t) + map->block_size * sizeof(uint32_t)) * map->block_size;
    uint64_t collisions_data        = (map->block_count * map->max_key_length) * map->block_size;
    uint64_t collisions_interlaced  = (map->block_count) * map->block_size;

    uint64_t collisions_size = (
        (sizeof(collisions_t) * map->block_size) 
        + collisions_free_list 
        + collisions_data 
        + collisions_interlaced
    );

    uint64_t map_size           = sizeof(zenmap_t);
    uint64_t value_data_size    = map->block_count * map->block_size * map->type_size;
    uint64_t key_data_size      = map->block_count * map->block_size * sizeof(utf8_t);

    return (map_size + collisions_size + value_data_size + key_data_size);
}

uint64_t zenmap_get_size(zenmap_t* map) {

    return map->key_count;

}

void bmap_drop(arena_t* arena, zenmap_t* map) {

    if (arena != nullptr || map == nullptr)
        return;

    if (map->collisions != nullptr)
        free(map->collisions);
    if (map->key_data != nullptr)
        free(map->key_data);
    if (map->value_data != nullptr)
        free(map->value_data);

}

#endif // BASE_IMPLEMENTATION
#if defined(__cplusplus)
#undef nullptr   
}
#endif // __cplusplus
#endif // BASE_H