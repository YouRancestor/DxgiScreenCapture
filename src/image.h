#ifndef IMAGE_H
#define IMAGE_H

#include <ScreenCapture/BufferAlloc.h>
#include <stdint.h>

class Image
{
public:
    enum Format{
        Format_ARGB,
    };
    Image(const char* data, uint32_t width, uint32_t height, uint32_t pitch, Format format,
          bool deep_copy, GetBuffer get_buf, ReleaseBuffer release_buf, void* allocator);
    Image(const Image& obj);
    ~Image();

    Buffer* GetData(){return buffer;}

private:
    int* refCount;
    Buffer* buffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    Format format;

    GetBuffer getBuffer;
    ReleaseBuffer releaseBuffer;
    void* bufAllocator;
};

#endif // IMAGE_H
