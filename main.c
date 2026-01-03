#define VALHALLOC_IMPLEMENTATION

#include "valhalloc.h"

int main( void ) {
    valhalloc_init();
    void* allocation = malloc( 10 );
    valhalloc_comment( allocation, "our first allocation" );
    allocation = realloc( allocation, 20 );
    free( allocation );
    valhalloc_deinit();
    return 0;
}
