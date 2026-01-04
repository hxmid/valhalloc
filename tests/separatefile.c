#include "../valhalloc.h"

void do_an_allocation( void ) {
    void** a = malloc( sizeof * a );
    free( a );
}
