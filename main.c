#include "valhalloc.h"

int main( void ) {
    void* allocation = malloc( 10 );
    allocation = realloc( allocation, 20 );
    free( allocation );
    return 0;
}
