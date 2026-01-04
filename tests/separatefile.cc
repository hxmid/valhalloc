#include "../valhalloc.h"

void do_an_allocation( void ) {
    int* a = VH_NEW int;
    VH_DELETE a;
}
