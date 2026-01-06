#pragma once

#ifndef VALHALLOC_H
# define VALHALLOC_H
# ifndef __cplusplus
#  include <stdlib.h>
#  include <stdint.h>
#  include <stdio.h>
#  include <assert.h>
#  include <string.h>
#  include <stdbool.h>
#  include <stddef.h>
#  define VH_EXTERN_C
# else // __cplusplus
#  include <cstdlib>
#  include <cstdio>
#  include <cstdint>
#  include <cassert>
#  include <cstring>
#  include <cstddef>
#  define VH_EXTERN_C extern "C"
# endif //__cplusplus

# ifdef VALHALLOC_ENABLE
typedef struct {
    char* file;
    uint64_t line;
    char* comment;
    uint64_t prevsize;
    uint64_t size;
    void* pointer;
    bool freed;
} valhallocation_t;
VH_EXTERN_C void valhalloc_init( FILE* file );
VH_EXTERN_C void valhalloc_logset( const char* filename );
VH_EXTERN_C void valhalloc_deinit( void );
VH_EXTERN_C void* valhalloc_alloc( uint64_t size, const char* file, uint64_t line );
VH_EXTERN_C void* valhalloc_realloc( void* allocation, uint64_t size, const char* file, uint64_t line );
VH_EXTERN_C void valhalloc_dealloc( void* allocation, const char* file, uint64_t line );
VH_EXTERN_C void valhalloc_comment( void* allocation, const char* comment );
VH_EXTERN_C void valhalloc_status( const char* file, uint64_t line );
VH_EXTERN_C valhallocation_t* valhalloc_logammend( void* pointer, void* newpointer, const char* comment, uint64_t size, bool freed );
VH_EXTERN_C valhallocation_t* valhalloc_logadd( void* pointer, char* comment, uint64_t size, const char* file, uint64_t line );
#  ifdef __cplusplus
void valhalloc_new( void* allocation, uint64_t size, const char* file, uint64_t line );
void valhalloc_delete( void* allocation, const char* file, uint64_t line );
#  endif // __cplusplus
#  undef VH_EXTERN_C

typedef struct {
    valhallocation_t* allocations;
    uint64_t size;
    uint64_t capacity;
    FILE* file;
} valhallocations_t;

extern valhallocations_t vh_valhallocations;

#  ifdef VALHALLOC_IMPLEMENTATION
#   ifdef VALHALLOC_IMPLEMENTED
#    error "[VALHALLOC FATAL]: defined VALHALLOC_IMPLEMENTATION in multiple files!!"
#   endif // VALHALLOC_IMPLEMENTED
valhallocations_t vh_valhallocations;

#   ifdef __cplusplus
extern "C" {
#   endif // __cplusplus

    void valhalloc_init( FILE* file ) {
        vh_valhallocations.allocations = (valhallocation_t*)calloc( 1, sizeof * vh_valhallocations.allocations );
        assert( vh_valhallocations.allocations );
        vh_valhallocations.capacity = 1;
        vh_valhallocations.size = 0;
        vh_valhallocations.file = file ? file : stderr;
    }

    void valhalloc_logset( const char* filename ) {
        if ( !filename || !strlen( filename ) ) {
            fprintf( vh_valhallocations.file, "[VALHALLOC WARN]:   did not pass a valid log file into valhalloc_logset()\n" );
            return;
        }
        if ( vh_valhallocations.file != stderr && vh_valhallocations.file != stdout ) {
            fflush( vh_valhallocations.file );
            fclose( vh_valhallocations.file );
        }
        vh_valhallocations.file = fopen( filename, "w" );
        assert( vh_valhallocations.file );
    }

    static void valhalloc_panic( int exitcode ) {
        fprintf( vh_valhallocations.file, "[VALHALLOC PANIC]:  PANIC!!\n" );
        fflush( vh_valhallocations.file );
        if ( vh_valhallocations.file != stderr )
            fclose( vh_valhallocations.file );
        exit( exitcode );
    }

    void valhalloc_deinit( void ) {
        for ( uint64_t i = 0; i < vh_valhallocations.size; i++ ) {
            valhallocation_t* a = &vh_valhallocations.allocations[i];
            if ( !a->freed ) {
                fprintf( vh_valhallocations.file, "[VALHALLOC LEAK]:   %p (%s) was never freed (first allocated @ %s:%llu)!!\n", a->pointer, a->comment ? a->comment : "<no comment>", a->file, a->line );
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
        fflush( vh_valhallocations.file );
        if ( vh_valhallocations.file && vh_valhallocations.file != stderr && vh_valhallocations.file != stdout ) {
            fclose( vh_valhallocations.file );
        }
    }

    static valhallocation_t* valhalloc_logsearch( void* pointer ) {
        for ( uint64_t i = 0; i < vh_valhallocations.size; i++ ) {
            valhallocation_t* a = &vh_valhallocations.allocations[i];
            if ( pointer == a->pointer )
                return a;
        }
        return NULL;
    }

    valhallocation_t* valhalloc_logadd( void* pointer, char* comment, uint64_t size, const char* file, uint64_t line ) {
        if ( !vh_valhallocations.capacity ) {
            fprintf( stderr, "[VALHALLOC FATAL]: you didn't call valhalloc_init()!!\n" );
            valhalloc_panic( EXIT_FAILURE );
        }
        valhallocation_t allocation;
        allocation.pointer = pointer;
        allocation.size = size;
        allocation.freed = false;
        allocation.prevsize = 0;
        allocation.line = line;
        allocation.comment = NULL;
        allocation.file = NULL;
        if ( comment ) {
            allocation.comment = (char*)malloc( strlen( comment ) + 1 );
            assert( allocation.comment );
            memcpy( allocation.comment, comment, strlen( comment ) + 1 );
        }
        if ( file ) {
            allocation.file = (char*)malloc( strlen( file ) + 1 );
            assert( allocation.file );
            memcpy( allocation.file, file, strlen( file ) + 1 );
        }

        valhallocation_t* vh_a = valhalloc_logsearch( pointer );
        if ( vh_a ) {
            if ( !vh_a->freed ) {
                fprintf( stderr, "[VALHALLOC FATAL]: allocator returned already-live address %p\n", pointer );
                valhalloc_panic( EXIT_FAILURE );
            }
            if ( vh_a->comment )
                free( (void*)vh_a->comment );
            if ( vh_a->file )
                free( (void*)vh_a->file );
            *vh_a = allocation;
            return vh_a;
        }

        if ( vh_valhallocations.capacity == vh_valhallocations.size ) {
            vh_valhallocations.capacity <<= 1;
            vh_valhallocations.allocations = (valhallocation_t*)realloc( vh_valhallocations.allocations, vh_valhallocations.capacity * ( sizeof * vh_valhallocations.allocations ) );
            assert( vh_valhallocations.allocations );
        }

        vh_a = &vh_valhallocations.allocations[vh_valhallocations.size];
        *vh_a = allocation;
        vh_valhallocations.size += 1;
        return vh_a;
    }

    valhallocation_t* valhalloc_logammend( void* pointer, void* newpointer, const char* comment, uint64_t size, bool freed ) {
        valhallocation_t* a = valhalloc_logsearch( pointer );
        if ( !a ) {
            fprintf( vh_valhallocations.file, "[VALHALLOC ERROR]:  attempted to ammend address %p!!\n", pointer );
            valhalloc_panic( EXIT_FAILURE );
        }
        if ( newpointer && pointer != newpointer ) {
        #ifdef VALHALLOC_DEBUG
            fprintf( vh_valhallocations.file, "[VALHALLOC DEBUG]:  old address: %p\n", a->pointer );
        #endif //VALHALLOC_DEBUG
            a->pointer = newpointer;
        #ifdef VALHALLOC_DEBUG
            fprintf( vh_valhallocations.file, "[VALHALLOC DEBUG]:  new address: %p\n", a->pointer );
        #endif //VALHALLOC_DEBUG
        }
        if ( comment ) {
            if ( a->comment ) {
            #ifdef VALHALLOC_DEBUG
                fprintf( vh_valhallocations.file, "[VALHALLOC DEBUG]:  old comment: %s\n", a->comment );
            #endif //VALHALLOC_DEBUG
                free( a->comment );
            }
            a->comment = (char*)malloc( strlen( comment ) + 1 );
            assert( a->comment );
            memcpy( a->comment, comment, strlen( comment ) + 1 );
        #ifdef VALHALLOC_DEBUG
            fprintf( vh_valhallocations.file, "[VALHALLOC DEBUG]:  new comment: %s\n", a->comment );
        #endif //VALHALLOC_DEBUG
        }
        if ( size && size != a->size ) {
        #ifdef VALHALLOC_DEBUG
            fprintf( vh_valhallocations.file, "[VALHALLOC DEBUG]:  old size: %lld\n", a->size );
        #endif //VALHALLOC_DEBUG
            a->prevsize = a->size;
            a->size = size;
        #ifdef VALHALLOC_DEBUG
            fprintf( vh_valhallocations.file, "[VALHALLOC DEBUG]:  new size: %lld\n", a->size );
        #endif //VALHALLOC_DEBUG
        }
        if ( a->freed && freed ) {
            fprintf( vh_valhallocations.file, "[VALHALLOC ERROR]:  double free!!\n" );
            valhalloc_panic( EXIT_FAILURE );
        } else if ( freed && freed != a->freed ) {
        #ifdef VALHALLOC_DEBUG
            fprintf( vh_valhallocations.file, "[VALHALLOC DEBUG]:  freed\n" );
        #endif //VALHALLOC_DEBUG
            a->freed = freed;
        } else if ( !( !freed && !a->freed ) ) {
            fprintf( stderr, "[VALHALLOC FATAL]: unreachable\n" );
        }
        return a;
    }

    void valhalloc_comment( void* pointer, const char* comment ) {
        valhalloc_logammend( pointer, NULL, comment, 0, false );
    }

    void* valhalloc_alloc( uint64_t size, const char* file, uint64_t line ) {
        void* allocation = malloc( size );
        assert( allocation );
        memset( (char*)allocation, 0xcd, size );
        fprintf( vh_valhallocations.file, "[VALHALLOC STATUS]: allocating %llu bytes to %p @ %s:%llu\n", size, allocation, file, line );
        valhalloc_logadd( allocation, NULL, size, file, line );
        return allocation;
    }

    void* valhalloc_realloc( void* old, uint64_t size, const char* file, uint64_t line ) {
    # ifdef __cplusplus
        fprintf( vh_valhallocations.file, "[VALHALLOC WARN]:   realloc in c++ when using new/delete/new[]/delete[] is undefined behaviour @ %s:%llu\n", file, line );
    # endif // __cplusplus
        if ( !old ) {
            fprintf( vh_valhallocations.file, "[VALHALLOC WARN]:   called realloc(NULL, x), treating as malloc(x) @ %s:%llu\n", file, line );
            return valhalloc_alloc( size, file, line );
        }
        if ( !size ) {
            fprintf( vh_valhallocations.file, "[VALHALLOC WARN]:   called realloc(x, 0), treating as free(x) @ %s:%llu\n", file, line );
            valhalloc_dealloc( old, file, line );
            return NULL;
        }
        const size_t old_address = (size_t)old;
        void* allocation = realloc( old, size );
        assert( allocation );
        fprintf( vh_valhallocations.file, "[VALHALLOC STATUS]: reallocating %llu bytes from %p to %p @ %s:%llu\n", size, (void*)old_address, allocation, file, line );
        valhallocation_t* vh_a = valhalloc_logammend( (void*)old_address, allocation, NULL, size, false );
        if ( size > vh_a->prevsize ) {
            memset( (char*)allocation + vh_a->prevsize, 0xcd, size - vh_a->prevsize );
        }
        return allocation;
    }

    void valhalloc_dealloc( void* allocation, const char* file, uint64_t line ) {
        if ( !allocation ) {
            fprintf( vh_valhallocations.file, "[VALHALLOC WARN]:   called free(NULL) @ %s:%llu\n", file, line );
            return;
        }
        fprintf( vh_valhallocations.file, "[VALHALLOC STATUS]: deallocating %p @ %s:%llu\n", allocation, file, line );
        valhallocation_t* vh_a = valhalloc_logammend( allocation, NULL, NULL, 0, true );
        memset( allocation, 0xDD, vh_a->size );
        free( allocation );
    }

    void valhalloc_status( const char* file, uint64_t line ) {
        fprintf( vh_valhallocations.file, "[VALHALLOC STATUS]: @ %s:%llu\n", file, line );
    }

#   ifdef __cplusplus
}
void valhalloc_new( void* allocation, uint64_t size, const char* file, uint64_t line ) {
    assert( allocation );
    fprintf( vh_valhallocations.file, "[VALHALLOC STATUS]: allocated %llu bytes to %p @ %s:%llu\n", size, allocation, file, line );
    valhalloc_logadd( allocation, NULL, size, file, line );
}

void valhalloc_delete( void* allocation, const char* file, uint64_t line ) {
    if ( !allocation ) {
        fprintf( vh_valhallocations.file, "[VALHALLOC WARN]:   called free(NULL) @ %s:%llu\n", file, line );
        return;
    }
    fprintf( vh_valhallocations.file, "[VALHALLOC STATUS]: deallocating %p @ %s:%llu\n", allocation, file, line );
    valhalloc_logammend( allocation, NULL, NULL, 0, true );
}
#   endif // __cplusplus

#  endif // VALHALLOC_IMPLEMENTATION

#  define VH_ALLOC( size)                                  valhalloc_alloc(size, __FILE__, __LINE__)
#  define VH_REALLOC( allocation, size )                   valhalloc_realloc(allocation, size, __FILE__, __LINE__)
#  define VH_DEALLOC( allocation )                         valhalloc_dealloc(allocation, __FILE__, __LINE__)
#  define VH_STATUS()                                      valhalloc_status(__FILE__, __LINE__)

#  ifdef __cplusplus
#   define VH_NEW( destination, constructor )              do { destination = new constructor; valhalloc_new(destination, sizeof * destination, __FILE__, __LINE__); } while ( 0 )
#   define VH_NEW_ARRAY( destination, constructor, count ) do { destination = new constructor[count]; valhalloc_new(destination, (sizeof * destination) * count, __FILE__, __LINE__); } while ( 0 )
#   define VH_DELETE( allocation )                         do { valhalloc_delete(allocation, __FILE__, __LINE__); delete allocation; } while ( 0 )
#   define VH_DELETE_ARRAY( allocation )                   do { valhalloc_delete(allocation, __FILE__, __LINE__); delete[] allocation; } while ( 0 )
#  endif // __cplusplus

# else // !VALHALLOC_ENABLE
#  define valhalloc_init( file )                           (void)NULL
#  define valhalloc_logset( filename )                     (void)NULL
#  define valhalloc_deinit()                               (void)NULL
#  define valhalloc_comment( allocation, comment )         (void)NULL
#  define VH_STATUS()                                      (void)NULL
#  define VH_REALLOC( allocation, size )                   realloc( allocation, size )
#  define VH_ALLOC( size )                                 malloc( size )
#  define VH_DEALLOC( allocation )                         free( allocation )
#  ifdef __cplusplus
#   define VH_NEW( destination, constructor )              destination = new constructor
#   define VH_NEW_ARRAY( destination, constructor, count ) destination = new constructor[count]
#   define VH_DELETE( allocation )                         delete allocation
#   define VH_DELETE_ARRAY( allocation )                   delete[] allocation
#  endif // __cplusplus
#  define VH_NEW_REGRESSION new
# endif // VALHALLOC_ENABLE
#endif // VALHALLOC_H
