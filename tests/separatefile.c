#include "../valhalloc.h"

void do_an_allocation( void ) {
    void** a = VH_ALLOC( sizeof * a );
    VH_DEALLOC( a );
}
