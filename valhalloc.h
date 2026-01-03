// note: these 2 lines are just here for development. i'll remove them once i'm done
#define VALHALLOC_ENABLE
#define VALHALLOC_IMPLEMENTATION

#pragma once

#ifndef VALHALLOC_H
#define VALHALLOC_H
# include <stdlib.h>
# include <stdint.h>
# include <stdio.h>
# include <assert.h>
# include <string.h>
# include <stdbool.h>

extern void  valhalloc_init( void );
extern void  valhalloc_deinit( void );
extern void* valhalloc_alloc( uint64_t size, const char* file, uint64_t line );
extern void* valhalloc_realloc( void* allocation, uint64_t size, const char* file, uint64_t line );
extern void  valhalloc_dealloc( void* allocation, const char* file, uint64_t line );

# ifdef VALHALLOC_ENABLE
#  ifdef VALHALLOC_IMPLEMENTATION
void* valhalloc_alloc( uint64_t size, const char* file, uint64_t line ) {
    void* allocation = malloc( size );
    memset( allocation, 0xcd, size );
    printf( "[VALHALLOC]: allocated %llu bytes to %p @ %s:%llu\n", size, allocation, file, line );
    return allocation;
}

void* valhalloc_realloc( void* old, uint64_t size, const char* file, uint64_t line ) {
    size_t old_address = (size_t)old;
    void* allocation = realloc( old, size );
    printf( "[VALHALLOC]: reallocated %llu bytes from %p to %p @ %s:%llu\n", size, (void*)old_address, allocation, file, line );
    return allocation;
}

void valhalloc_dealloc( void* allocation, const char* file, uint64_t line ) {
    printf( "[VALHALLOC]: deallocated %p @ %s:%llu\n", allocation, file, line );
    free( allocation );
}
#  endif // VALHALLOC_IMPLEMENTATION
#  define malloc(size)               valhalloc_alloc(size, __FILE__, __LINE__)
#  define realloc(allocation, size)  valhalloc_realloc(allocation, size, __FILE__, __LINE__)
#  define free(allocation)           valhalloc_dealloc(allocation, __FILE__, __LINE__)
# endif // VALHALLOC_ENABLE
#endif // VALHALLOC_H
