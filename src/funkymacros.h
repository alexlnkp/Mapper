#ifndef   FUNKYMACROS_H
#define   FUNKYMACROS_H

#define MALLOCFUNC MemAlloc
#define MEMFREEFUNC MemFree

#ifndef   NDEBUG

#define MALLOC(self, size)   \
    self = MALLOCFUNC(size); \
    assert(self != NULL);

#else
#define MALLOC(self, size)   \
    self = MALLOCFUNC(size);

#endif /* NDEBUG */

#define FREE(self)     \
    MEMFREEFUNC(self);

#endif /* FUNKYMACROS_H */