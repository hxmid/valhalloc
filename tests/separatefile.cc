#include "../valhalloc.h"

void do_an_allocation( void ) {
    void** a;
    VH_NEW( a, void* );
    VH_DELETE( a );
}
