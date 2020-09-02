#ifndef BUFFERALLOCATOR_H
#define BUFFERALLOCATOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct Buffer {
    uint8_t* ptr;
    size_t size; /// avaliable buffer size, in bytes
} Buffer;

typedef Buffer* (*GetBuffer)(void* allocator, size_t size);
typedef void (*ReleaseBuffer)(void* allocator, Buffer* buffer);

#ifdef __cplusplus
}
#endif

#endif // BUFFERALLOCATOR_H
