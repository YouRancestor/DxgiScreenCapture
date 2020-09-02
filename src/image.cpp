#include "image.h"
#include <string.h>

Image::Image(const char* data, uint32_t width, uint32_t height, uint32_t pitch, Format format,
             bool deep_copy, GetBuffer get_buf, ReleaseBuffer release_buf, void* allocator)
    : refCount(new int(1))
    , buffer(NULL)
    , width(width)
    , height(height)
    , pitch(pitch)
    , format(format)
    , getBuffer(get_buf)
    , releaseBuffer(release_buf)
    , bufAllocator(allocator)
{
    if (deep_copy)
    {
        int size = 0;
        switch (format) {
        case Image::Format_ARGB:
            size = pitch * height;
            break;
        default:
            break;
        }
        buffer = getBuffer(bufAllocator, size);
        memcpy(buffer->ptr, data, size);
    }
}

Image::Image(const Image &obj)
{
    memcpy(this, &obj, sizeof(obj));
    ++*this->refCount;
}

Image::~Image()
{
    if (--*refCount == 0)
    {
        releaseBuffer(bufAllocator, buffer);
        delete refCount;
    }
}
