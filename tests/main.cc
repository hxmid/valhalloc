#define VALHALLOC_IMPLEMENTATION

#include "../valhalloc.h"

void do_an_allocation( void );

int main( void ) {
    valhalloc_init( stderr );
    valhalloc_logset( "valhallocc.log" );
    char* allocation;
    VH_NEW_ARRAY( allocation, char, 10 );
    valhalloc_comment( allocation, "our first allocation" );
    strcpy( allocation, "Hi\n" );
    // allocation = (char*)VH_REALLOC( allocation, 20 );
    printf( allocation );
    VH_DELETE_ARRAY( allocation );
    do_an_allocation();
    valhalloc_deinit();
    return 0;
}
