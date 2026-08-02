#define BASE_IMPLEMENTATION
#include <base.h>

// TODO(Joel Zbinden): Automate tests

err_t test_array(arena_t* arena) {

    printf("\nTesting arrays %s...\n", arena != nullptr ? "with arenas" : "");

    array_t(int) int_array = array_create(arena, int, 20);

    printf("capacity: %u, size: %u\n", array_capacity(int_array), array_size(int_array));
    for (size_t i = 0; i < array_capacity(int_array); i++) {
        printf("%d, ", int_array[i]);
        int_array[i] = i;
    }
    printf("\n");
    
    array_resize(arena, int_array, 10);

    printf("capacity: %u, size: %u\n", array_capacity(int_array), array_size(int_array));
    for (size_t i = 0; i < array_capacity(int_array); i++)
        printf("%d, ", int_array[i]);
    printf("\n");

    array_drop(arena, int_array);
    return EXIT_SUCCESS;

}

err_t test_string(arena_t* arena) {
    
    printf("\nTesting strings %s...\n", arena != nullptr ? "with arenas" : "");
    
    utf8_t lit = utf8_from_literal("Hello World\n");
    printf(strfmt"\n", strarg(lit));
    
    utf8_t view = utf8_from_view("Hello World", 5);
    printf(strfmt"\n", strarg(view));

    utf8_t alloc = utf8_from(arena, "Hello %s", "James");
    printf(strfmt"\n", strarg(alloc));
    utf8_drop(arena, &alloc);

    return EXIT_SUCCESS;

}

err_t test_arena(void) {

    printf("\nTesting arenas...\n");

    arena_t arena = arena_create(1024);
    i8_ptr_t string = arena_alloc(&arena, 13, 1);
    sprintf(string, "Hello World\n");

    printf("%s", string);
    
    if (test_array(&arena) != EXIT_SUCCESS) {
        arena_drop(&arena);
        return EXIT_FAILURE;
    }

    if (test_string(&arena) != EXIT_SUCCESS) {
        arena_drop(&arena);
        return EXIT_FAILURE;
    }

    arena_drop(&arena);
    return EXIT_SUCCESS;

}

err_t test_bitmask(void) {

    printf("\nTesting 128 bit bitmasks...\n");

    bitmask128_t bitmask = empty_bitmask128_t;

    array_t(char) result = bitmask_to_cstr(bitmask, 2);
    printf("result: %s\n", result);
    array_drop(nullptr, result);

    bitmask128_set(bitmask, 18, 1);

    result = bitmask_to_cstr(bitmask, 2);
    printf("result: %s\n\n", result);
    array_drop(nullptr, result);
    
    return EXIT_SUCCESS;

}

int main(void) {

    if (test_array(nullptr) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    if (test_string(nullptr) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    if (test_arena() != EXIT_SUCCESS)
        return EXIT_FAILURE;
  
    if (test_bitmask() != EXIT_SUCCESS)
        return EXIT_FAILURE;

    return 0;

}