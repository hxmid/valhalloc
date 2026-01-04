#define VALHALLOC_IMPLEMENTATION

#include "../valhalloc.h"

void do_an_allocation( void );

int main( void ) {
    valhalloc_init();
    char* allocation = malloc( 10 );
    valhalloc_comment( allocation, "our first allocation" );
    strcpy( allocation, "Hi\n" );
    allocation = realloc( allocation, 20 );
    printf( allocation );
    free( allocation );
    do_an_allocation();
    valhalloc_deinit();
    return 0;
}
