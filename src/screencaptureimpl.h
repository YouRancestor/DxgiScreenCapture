#ifndef SCREENCAPTUREIMPL_H
#define SCREENCAPTUREIMPL_H

#include <ScreenCapture/ScreenCapture.h>
#include <dxgi1_2.h>
#include <D3D11.h>


struct ScreenCapture
{
    ScreenCapture(int adapter_index, int display_index);

    int ResetDxgiDup();

    ~ScreenCapture();

    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dContext;
    IDXGIOutputDuplication* dxgiDup;

    int adapterIndex;
    int displayIndex;
    bool showCursor;
    struct CursorInfo
    {
        Buffer* buf;
        UINT size;
        UINT type;
        UINT width;
        UINT height;
        UINT pitch;
        UINT posX;
        UINT posY;
        BOOL visiblity;
    } curInfo;

    struct FrameAlloc
    {
        GetFrame get;
        ReleaseFrame release;
        void* opaque;
    } frame_allocator;

    struct BuffAlloc
    {
        GetBuffer get;
        ReleaseBuffer release;
        void* opaque;
    } allocator;

private:
    static Buffer* DefaultAlloc(void * allocator, size_t size);

    static void DefaultFree(void * allocator, Buffer* buf);


};

#endif // SCREENCAPTUREIMPL_H
