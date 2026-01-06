# valhalloc
the allocator from hell (because i made it)

## what is it
valhalloc is just a c/c++ allocator wrapper that aims to be much more verbose than the standard `malloc`/`new` (and their counterparts)

## limitations and notes
- this project is in a very early stage in development
- not multi-thread safe (hopefully **yet**)
- not compatible 1:1 with the standard c++ api's `new`/`new[]`/`delete`/`delete[]`
    - `VH_NEW`/`VH_NEW_ARRAY`/`VH_DELETE`/`VH_DELETE_ARRAY` call on their respective c++ counterparts, but they require arguments, which means there needs to be a bit of reworking in how you define it
    - because of that, i've added wrappers so they still work even when VALHALLOC_ENABLE isn't defined, but still just be aware that they're called weirdly (sorry, i'll hopefully think of something smarter in the future)
