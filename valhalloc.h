#pragma once

#ifndef VALHALLOC_H
# define VALHALLOC_H
# include <stdlib.h>
# include <stdint.h>
# include <stdio.h>
# include <assert.h>
# include <string.h>
# include <stdbool.h>

extern void* valhalloc_alloc( uint64_t size, const char* file, uint64_t line );
extern void* valhalloc_realloc( void* allocation, uint64_t size, const char* file, uint64_t line );
extern void  valhalloc_dealloc( void* allocation, const char* file, uint64_t line );
extern void  valhalloc_comment( void* allocation, char* comment );

# ifdef VALHALLOC_ENABLE
#  ifdef VALHALLOC_IMPLEMENTATION
#  ifdef VALHALLOC_IMPLEMENTED
#   error "[VALHALLOC FATAL]: defined VALHALLOC_IMPLEMENTATION in multiple files!!"
#  endif // VALHALLOC_IMPLEMENTED
#  ifndef __cplusplus
#   ifdef malloc
#    undef malloc
#   endif // malloc
#   ifdef realloc
#    undef realloc
#   endif // realloc
#   ifdef free
#    undef free
#   endif // free
#  endif // __cplusplus

typedef struct {
    char* file;
    uint64_t line;
    char* comment;
    uint64_t prevsize;
    uint64_t size;
    void* pointer;
    bool freed;
} valhallocation_t;

typedef struct {
    valhallocation_t* allocations;
    uint64_t size;
    uint64_t capacity;
} valhallocations_t;

static valhallocations_t vh_valhallocations = { 0 };

static void valhalloc_init( void ) {
    vh_valhallocations.allocations = (valhallocation_t*)malloc( sizeof * vh_valhallocations.allocations );
    assert( vh_valhallocations.allocations );
    vh_valhallocations.capacity = 1;
    vh_valhallocations.size = 0;
}

static void valhalloc_deinit( void ) {
    for ( uint64_t i = 0; i < vh_valhallocations.size; i++ ) {
        valhallocation_t* a = &vh_valhallocations.allocations[i];
        if ( !a->freed ) {
            printf( "[VALHALLOC LEAK]:   %p (%s) was never freed (first allocated @ %s:%llu)!!\n", a->pointer, a->comment ? a->comment : "<no comment>", a->file, a->line );
        }
        if ( a->comment )
            free( (void*)a->comment );
        if ( a->file )
            free( (void*)a->file );
    }
    free( (void*)vh_valhallocations.allocations );
    vh_valhallocations.size = 0;
    vh_valhallocations.capacity = 0;
    vh_valhallocations.allocations = NULL;
}

static valhallocation_t* valhalloc_logadd( void* pointer, char* comment, uint64_t size, const char* file, uint64_t line ) {
    if ( !vh_valhallocations.capacity ) {
        fprintf( stderr, "[VALHALLOC FATAL]: you didn't call valhalloc_init()!!\n" );
        exit( 1 );
    }
    valhallocation_t allocation = { .pointer = pointer, .size = size, .freed = false, .prevsize = 0, .line = line, .comment = NULL, .file = NULL };
    if ( comment ) {
        allocation.comment = strdup( comment );
        assert( allocation.comment );
    }
    if ( file ) {
        allocation.file = strdup( file );
        assert( allocation.file );
    }

    if ( vh_valhallocations.capacity == vh_valhallocations.size ) {
        vh_valhallocations.capacity <<= 1;
        vh_valhallocations.allocations = (valhallocation_t*)realloc( vh_valhallocations.allocations, vh_valhallocations.capacity * ( sizeof * vh_valhallocations.allocations ) );
        assert( vh_valhallocations.allocations );
    }

    valhallocation_t* vh_a = &vh_valhallocations.allocations[vh_valhallocations.size];
    *vh_a = allocation;
    vh_valhallocations.size += 1;
    return vh_a;
}

static valhallocation_t* valhalloc_logammend( void* pointer, void* newpointer, char* comment, uint64_t size, bool freed ) {
    for ( uint64_t i = 0; i < vh_valhallocations.size; i++ ) {
        if ( vh_valhallocations.allocations[i].pointer == pointer ) {
            valhallocation_t* a = &vh_valhallocations.allocations[i];
            if ( newpointer && pointer != newpointer ) {
            #ifdef VALHALLOC_DEBUG
                printf( "[VALHALLOC DEBUG]:  old address: %p\n", a->pointer );
            #endif //VALHALLOC_DEBUG
                a->pointer = newpointer;
            #ifdef VALHALLOC_DEBUG
                printf( "[VALHALLOC DEBUG]:  new address: %p\n", a->pointer );
            #endif //VALHALLOC_DEBUG
            }
            if ( comment ) {
                if ( a->comment ) {
                #ifdef VALHALLOC_DEBUG
                    printf( "[VALHALLOC DEBUG]:  old comment: %s\n", a->comment );
                #endif //VALHALLOC_DEBUG
                    free( a->comment );
                }
                a->comment = strdup( comment );
                assert( a->comment );
            #ifdef VALHALLOC_DEBUG
                printf( "[VALHALLOC DEBUG]:  new comment: %s\n", a->comment );
            #endif //VALHALLOC_DEBUG
            }
            if ( size && size != a->size ) {
            #ifdef VALHALLOC_DEBUG
                printf( "[VALHALLOC DEBUG]:  old size: %lld\n", a->size );
            #endif //VALHALLOC_DEBUG
                a->prevsize = a->size;
                a->size = size;
            #ifdef VALHALLOC_DEBUG
                printf( "[VALHALLOC DEBUG]:  new size: %lld\n", a->size );
            #endif //VALHALLOC_DEBUG
            }
            if ( a->freed && freed ) {
                printf( "[VALHALLOC ERROR]:  double free!!\n" );
                exit( 1 );
            } else if ( freed && freed != a->freed ) {
            #ifdef VALHALLOC_DEBUG
                printf( "[VALHALLOC DEBUG]:  freed\n" );
            #endif //VALHALLOC_DEBUG
                a->freed = freed;
            } else if ( !( !freed && !a->freed ) ) {
                fprintf( stderr, "[VALHALLOC FATAL]: unreachable\n" );
            }
            return a;
        }
    }
    printf( "[VALHALLOC ERROR]:  attempted to ammend address %p!!\n", pointer );
    exit( 1 );
}

void valhalloc_comment( void* pointer, char* comment ) {
    valhalloc_logammend( pointer, NULL, comment, 0, false );
}

void* valhalloc_alloc( uint64_t size, const char* file, uint64_t line ) {
    void* allocation = malloc( size );
    assert( allocation );
    memset( (char*)allocation, 0xcd, size );
    printf( "[VALHALLOC STATUS]: allocating %llu bytes to %p @ %s:%llu\n", size, allocation, file, line );
    valhalloc_logadd( allocation, NULL, size, file, line );
    return allocation;
}

void* valhalloc_realloc( void* old, uint64_t size, const char* file, uint64_t line ) {
    if ( !old ) {
        printf( "[VALHALLOC WARN]:   called realloc(NULL, x), treating as malloc(x) @ %s:%llu\n", file, line );
        return valhalloc_alloc( size, file, line );
    }
    if ( !size ) {
        printf( "[VALHALLOC WARN]:   called realloc(x, 0), treating as free(x) @ %s:%llu\n", file, line );
        valhalloc_dealloc( old, file, line );
        return NULL;
    }
    const size_t old_address = (size_t)old;
    void* allocation = realloc( old, size );
    assert( allocation );
    printf( "[VALHALLOC STATUS]: reallocating %llu bytes from %p to %p @ %s:%llu\n", size, (void*)old_address, allocation, file, line );
    valhallocation_t* vh_a = valhalloc_logammend( (void*)old_address, allocation, NULL, size, false );
    if ( size > vh_a->prevsize ) {
        memset( (char*)allocation + vh_a->prevsize, 0xcd, size - vh_a->prevsize );
    }
    return allocation;
}

void valhalloc_dealloc( void* allocation, const char* file, uint64_t line ) {
    if ( !allocation ) {
        printf( "[VALHALLOC WARN]:   called free(NULL) @ %s:%llu\n", file, line );
        return;
    }
    printf( "[VALHALLOC STATUS]: deallocating %p @ %s:%llu\n", allocation, file, line );
    valhallocation_t* vh_a = valhalloc_logammend( allocation, NULL, NULL, 0, true );
    memset( allocation, 0xDD, vh_a->size );
    free( allocation );
}
#  endif // VALHALLOC_IMPLEMENTATION
#  ifndef __cplusplus
#   define malloc(size)               valhalloc_alloc(size, __FILE__, __LINE__)
#   define realloc(allocation, size)  valhalloc_realloc(allocation, size, __FILE__, __LINE__)
#   define free(allocation)           valhalloc_dealloc(allocation, __FILE__, __LINE__)
#  endif // __cplusplus
# endif // VALHALLOC_ENABLE
#endif // VALHALLOC_H
