// #define BASE_IMPLEMENTATION
#include <base.h>

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

    return EXIT_SUCCESS;

}

err_t test_string(arena_t* arena) {
    
    printf("\nTesting strings %s...\n", arena != nullptr ? "with arenas" : "");
    
    string_t lit = string_from_literal("Hello World\n");
    printf(strfmt"\n", strarg(lit));
    
    string_t view = string_from_view("Hello World", 5);
    printf(strfmt"\n", strarg(view));

    string_t alloc = string_from(arena, "Hello %s", "James");
    printf(strfmt"\n", strarg(alloc));
    string_drop(arena, &alloc);

    return EXIT_SUCCESS;

}

err_t test_fstrman(arena_t* arena) {
    
    printf("\nTesting fstrman %s...\n", arena != nullptr ? "with arenas" : "");

    fstrman_t man = fstrman_create(arena, 10);

    fstr_t a = fstr_from_cstr(&man, "Hello World");
    string_t s = string_from_fstr(arena, &man, a);
    printf(strfmt"\n",  strarg(s));

    for (size_t i = 0; i < 8; i++)
        (void)fstr_from(&man, "Hello World %d", i);
    
    array_t(fstr_t) matches = fstr_matchn(arena, &man, &s, a, 10);
    printf("matches: %d, should be: 1\n", array_size(matches));

    fstrman_drop(arena, &man);
    return EXIT_SUCCESS;

}

err_t test_arena(void) {

    printf("\nTesting arenas...\n");

    arena_t arena = arena_create(1024);
    char_ptr_t string = arena_alloc(&arena, 13, 1);
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

    if (test_fstrman(&arena) != EXIT_SUCCESS) {
        arena_drop(&arena);
        return EXIT_FAILURE;
    }

    arena_drop(&arena);
    return EXIT_SUCCESS;

}

err_t test_bitmask(void) {

    printf("\nTesting 128 bit bitmasks...\n");

    bitmask128_t bitmask = empty_bitmask128_t;

    array_t(char) result = get_binary_as_cstr(bitmask, 16);
    printf("result: %s\n", result);
    array_drop(nullptr, result);

    bitmask128_set(bitmask, 18, 1);

    result = get_binary_as_cstr(bitmask, 16);
    printf("result: %s\n", result);
    array_drop(nullptr, result);

    return EXIT_SUCCESS;

}

void timed_test_hashmap(void) {

    // NOTE(Joel Zbinden): This is currently not working, 
    // need to restructure this implementation, it kinda
    // sucks fr.

    arena_t arena = arena_create(20 * KB);
    fstrman_t man = fstrman_create(&arena, 256);

    for (size_t i = 0; i < 255; i++)
        (void)fstr_from(&man, "kasjd;lkfjasdlk;fjlkasHello World %d", i);
    fstr_t a = fstr_from_cstr(&man, "Hello World");
    string_t s = string_from_fstr(&arena, &man, a);
    timer(TC_START);

    array_t(fstr_t) matches = fstr_matchn(&arena, &man, &s, 0, 256);

    printf("\ntime: %f\n", timer(TC_END));
    // printf("matches: %d, should be: 1\n", array_size(matches));

    // NOTE(Joel Zbinden): This is somehow faster, we need to fix that.

    array_t(string_t) strs = array_create(&arena, string_t, 255);

    for (size_t i = 0; i < 254; i++)
        strs[i] = string_from(&arena, "faksdj,mfnjksald;jflk;aHello World %d", i);
    strs[255 - 1] = string_from(&arena, "Hello World");
    timer(TC_START);
    
    size_t matches_slow = 0;
    for (size_t i = 0; i < 255; i++)
        if (strcmp("Hello World", strs[i].data) == 0)
            matches_slow++;

    printf("\ntime: %f\n", timer(TC_END));
    // printf("matches: %ld, should be: 1\n", matches_slow);
    
}

int main(void) {

    timed_test_hashmap();

    // if (test_array(nullptr) != EXIT_SUCCESS)
    //     return EXIT_FAILURE;

    // if (test_string(nullptr) != EXIT_SUCCESS)
    //     return EXIT_FAILURE;

    // if (test_fstrman(nullptr) != EXIT_SUCCESS)
    //     return EXIT_FAILURE;
            
    // if (test_arena() != EXIT_SUCCESS)
    //     return EXIT_FAILURE;
  
    // if (test_bitmask() != EXIT_SUCCESS)
    //     return EXIT_FAILURE;


    return 0;

}