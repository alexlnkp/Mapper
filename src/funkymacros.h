#ifndef   FUNKYMACROS_H
#define   FUNKYMACROS_H

#define MALLOCFUNC MemAlloc
#define REALLOCFUNC MemRealloc
#define MEMFREEFUNC MemFree

#ifndef   NDEBUG

#define MALLOC(self, size)   \
    self = MALLOCFUNC(size); \
    assert(self != NULL);

#define REALLOC(self, new_size)         \
    self = REALLOCFUNC(self, new_size); \
    assert(self != NULL);

#else
#define MALLOC(self, size)   \
    self = MALLOCFUNC(size);

#define REALLOC(self, new_size)         \
    self = REALLOCFUNC(self, new_size);

#endif /* NDEBUG */

#define FREE(self)     \
    MEMFREEFUNC(self);

#endif /* FUNKYMACROS_H */