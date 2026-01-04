#define VALHALLOC_IMPLEMENTATION

#include "valhalloc.h"

int main( void ) {
    valhalloc_init();
    char* allocation = VH_NEW char[10];
    valhalloc_comment( allocation, "our first allocation" );
    strcpy( allocation, "Hi\n" );
    allocation = (char*)VH_REALLOC( allocation, 20 );
    printf( allocation );
    VH_DELETE[] allocation;
    valhalloc_deinit();
    return 0;
}
