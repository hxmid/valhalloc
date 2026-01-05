# valhalloc
the allocator from hell (because i made it)

## what is it
valhalloc is just a c/c++ allocator wrapper that aims to be much more verbose than the standard `malloc`/`new` (and their counterparts)

## limitations and notes
- this project is in a very early stage in development
- not multi-thread safe (hopefully **yet**)
- VH_DELETE in c++ mode loses `__FILE__` and `__LINE__` info
    - i added `VH_STATUS()` specifically for this reason, it's not as clean as the c counterpart, but debug info is debug info

