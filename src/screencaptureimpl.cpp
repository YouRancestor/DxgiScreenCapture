#include "screencaptureimpl.h"


#include "definitions.h"

ScreenCapture::ScreenCapture(int adapter_index, int display_index)
    : d3dDevice(NULL)
    , d3dContext(NULL)
    , dxgiDup(NULL)
    , adapterIndex(adapter_index)
    , displayIndex(display_index)
    , showCursor(true)
    , curInfo{NULL, 0, 0, 0, 0, 0, 0, 0, 0}
    , frame_allocator{NULL, NULL, NULL}
    , allocator{DefaultAlloc, DefaultFree, NULL}
{}

int ScreenCapture::ResetDxgiDup()
{
    SAFE_RELEASE(dxgiDup);
    SAFE_RELEASE(d3dContext);
    SAFE_RELEASE(d3dDevice);

    HRESULT hr;
    IDXGIFactory1* dxgiFactory = NULL;
    hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&dxgiFactory));

    if (FAILED(hr))
    {
        return E_DEVICE_NOT_SURPPORT;
    }

    IDXGIAdapter1* adapter = NULL;
    hr = dxgiFactory->EnumAdapters1(this->adapterIndex, &adapter);
    SAFE_RELEASE(dxgiFactory);

    if (FAILED(hr))
    {
        return E_INVALID_ADAPTER_INDEX;
    }

    IDXGIOutput* dxgiOutput = NULL;
    hr = adapter->EnumOutputs(this->displayIndex, &dxgiOutput);

    if (FAILED(hr))
    {
        return E_INVALID_DISPLAY_INDEX;
    }

    IDXGIOutput1* dxgiOutput1 = NULL;
    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&dxgiOutput1);
    SAFE_RELEASE(dxgiOutput);

    if (FAILED(hr))
    {
        SAFE_RELEASE(adapter);
        return E_DEVICE_NOT_SURPPORT;
    }

    const D3D_FEATURE_LEVEL features[] = {D3D_FEATURE_LEVEL_11_1};
    D3D_FEATURE_LEVEL feature;
    hr = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, NULL, features, 1, D3D11_SDK_VERSION, &this->d3dDevice, &feature, &this->d3dContext);
    SAFE_RELEASE(adapter);

    if (FAILED(hr))
    {
        SAFE_RELEASE(dxgiOutput1);
        return E_DEVICE_NOT_SURPPORT;
    }

    hr = dxgiOutput1->DuplicateOutput(this->d3dDevice, &this->dxgiDup);
    SAFE_RELEASE(dxgiOutput1);

    if (FAILED(hr))
    {
        return E_DEVICE_NOT_SURPPORT;
    }

    return 0;
}

ScreenCapture::~ScreenCapture()
{
    if (curInfo.buf)
        allocator.release(allocator.opaque, curInfo.buf);

    SAFE_RELEASE(dxgiDup);
    SAFE_RELEASE(d3dContext);
    SAFE_RELEASE(d3dDevice);
}

Buffer *ScreenCapture::DefaultAlloc(void *allocator, size_t size)
{
    Buffer* buf = new Buffer;
    buf->ptr = new uint8_t[size];
    buf->size = size;
    return buf;
}

void ScreenCapture::DefaultFree(void *allocator, Buffer *buf)
{
    delete[] buf->ptr;
    delete buf;
}
