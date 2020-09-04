
#include <ScreenCapture/ScreenCapture.h>

#include <dxgi1_2.h>
#include <d3d11.h>

#include "image.h"
#include "definitions.h"
#include "screencaptureimpl.h"

struct FrameArgb : public Frame
{
    FrameArgb(const char* data, uint32_t width, uint32_t height, uint32_t pitch, Image::Format fmt,
              bool deep_copy, GetBuffer get_buf, ReleaseBuffer release_buf, void* allocator)
        : img(data, width, height, pitch, fmt, true, get_buf, release_buf, allocator)
    {
        this->buffer = img.GetData();
        this->format = FORMAT_ARGB;
        this->width = width;
        this->height = height;
        this->pitch = pitch;
    }
    ~FrameArgb(){}
    Image img; // 引用计数型
};


int CreateScreenCapture(int adapter_index, int display_index, ScreenCapture** cap)
{
    ScreenCapture* capture = new ScreenCapture(adapter_index, display_index);

    int ret = capture->ResetDxgiDup();

    if (!ret)
    {
        *cap = capture;
    }
    else
    {
        delete capture;
        *cap = NULL;
    }

    return ret;
}

void DestroyScreenCapture(ScreenCapture *instance)
{
    delete instance;
}

static void DrawCursor(ScreenCapture* instance, Frame *pic)
{
    if (!instance->curInfo.visiblity)
        return;

    ScreenCapture::CursorInfo* cur = &instance->curInfo;

    uint8_t* src = cur->buf->ptr;
    uint8_t* dst = pic->buffer->ptr;

    switch (cur->type) {
    case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
    {
        LONG offset_x = 0;
        LONG offset_y = 0;
        LONG w = cur->width;
        LONG h =  cur->height / 2;

        if (cur->posX < 0)
        {
            offset_x = -cur->posX;
        }
        if (cur->posY < 0)
        {
            offset_y = -cur->posY;
        }

        if (cur->posX + w > pic->width)
        {
            w = pic->width - cur->posX;
        }
        if (cur->posY + h > pic->height)
        {
            h = pic->height - cur->posY;
        }

        for (LONG yi = offset_y; yi < h; ++yi)
        {
            for (LONG xi = offset_x; xi < w; ++xi)
            {
                char andByte = src[yi*cur->pitch + xi/8];
                char xorByte = src[cur->pitch*h + yi*cur->pitch + xi/8];
                int bit = xi % 8;
                char mask = 1 <<(7-bit);

                int pixSize = 4;
                uint32_t* dst_color = (uint32_t*)(dst + pic->pitch * (cur->posY + yi) + (cur->posX + xi) * pixSize);

                if (!(mask & andByte))
                {
                    // AND mask 为0，取黑色
                    *dst_color = 0xff000000;
                }
                if (mask & xorByte) // XOR mask 为1，取反色
                {
                    *dst_color = (*dst_color ^ 0xffffff) | 0xff000000;
                }
            }
        }
    }
        break;
    case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR:
    {
        LONG offset_x = 0;
        LONG offset_y = 0;
        LONG w = cur->width;
        LONG h =  cur->height;

        if (cur->posX < 0)
        {
            offset_x = -cur->posX;
        }
        if (cur->posY < 0)
        {
            offset_y = -cur->posY;
        }

        if (cur->posX + w > pic->width)
        {
            w = pic->width - cur->posX;
        }
        if (cur->posY + h > pic->height)
        {
            h = pic->height - cur->posY;
        }

        int pixSize = 4;
        for (LONG yi = offset_y; yi< h; ++yi)
        {
            for (LONG xi = offset_x; xi < w; ++xi)
            {
                uint8_t* pixel = src + yi*cur->pitch + xi*pixSize;
                uint32_t* dst_color = (uint32_t*)(dst + pic->pitch * (cur->posY + yi) + (cur->posX + xi) * pixSize);
                uint32_t argb = *(uint32_t*)pixel;
                if (argb & 0xff000000)
                {
                    // alpha blending
                    uint32_t a = (argb >> 24) & 0xff;
                    uint32_t r = (((argb >> 16) & 0xff) * a + ((*dst_color >> 16)&0xff) * (255-a)) / 255;
                    uint32_t g = (((argb >>  8) & 0xff) * a + ((*dst_color >>  8)&0xff) * (255-a)) / 255;
                    uint32_t b = ((argb & 0xff) * a + (*dst_color&0xff) * (255-a)) / (float)255;

                    *dst_color = (0xff<<24) | (r<<16) | (g<<8) | b;
                }
            }
        }
    }
        break;
    case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
    {
        LONG offset_x = 0;
        LONG offset_y = 0;
        LONG w = cur->width;
        LONG h =  cur->height;

        if (cur->posX < 0)
        {
            offset_x = -cur->posX;
        }
        if (cur->posY < 0)
        {
            offset_y = -cur->posY;
        }

        if (cur->posX + w > pic->width)
        {
            w = pic->width - cur->posX;
        }
        if (cur->posY + h > pic->height)
        {
            h = pic->height - cur->posY;
        }
        int pixSize = 4;

        for (LONG yi = offset_y; yi< h; ++yi)
        {
            for (LONG xi = offset_x; xi < w; ++xi)
            {
                uint8_t* pixel = src + yi*cur->pitch + xi*pixSize;
                uint32_t mask = (*(uint32_t*)pixel) & 0xff000000;
                uint32_t rgb = (*(uint32_t*)pixel) | 0xff000000;
                uint32_t* dst_color = (uint32_t*)(dst + pic->pitch * (cur->posY + yi) + (cur->posX + xi) * pixSize);
                if (mask)
                {
                    *dst_color = (*dst_color ^ rgb) | 0xff000000;
                }
                else
                {
                    *dst_color = rgb;
                }
            }
        }
    }
        break;
    default:
        break;
    }

}

int TakeSnapshot(ScreenCapture *instance, uint32_t timeout_ms, Frame** frame)
{
    if (!frame || !instance)
        return E_INVALID_ARGUMENTS;

    IDXGIResource* resource = NULL;
    DXGI_OUTDUPL_FRAME_INFO frminfo;

    HRESULT hr = instance->dxgiDup->AcquireNextFrame(timeout_ms, &frminfo, &resource);
    if (!resource)
    {
        if (hr == DXGI_ERROR_ACCESS_LOST || hr == DXGI_ERROR_INVALID_CALL)
        {
            // 失效重置
            instance->ResetDxgiDup();
            return E_AGAIN;
        }
        else if (hr == DXGI_ERROR_WAIT_TIMEOUT)
        {
            // 超时重来
            return E_TIMEOUT;
        }
        else if (frminfo.LastPresentTime.LowPart == 0 && frminfo.LastPresentTime.HighPart == 0)
        {
            // 图像没有变化
            return E_NO_CHANGE;
        }
        else
        {
            return E_UNKNOWN;
        }
    }

    ID3D11Texture2D* img = NULL;
    hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&img);
    SAFE_RELEASE(resource);

    // 读取图像
    ID3D11Texture2D* texture = NULL;
    D3D11_TEXTURE2D_DESC desc;
    img->GetDesc(&desc);
    D3D11_TEXTURE2D_DESC tex_desc;
    ZeroMemory(&tex_desc, sizeof(tex_desc));
    tex_desc.Width = desc.Width;
    tex_desc.Height = desc.Height;
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.SampleDesc.Quality = 0;
    tex_desc.Usage = D3D11_USAGE_STAGING;
    tex_desc.Format = desc.Format;
    tex_desc.BindFlags = 0;
    tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    tex_desc.MiscFlags = 0;
    hr = instance->d3dDevice->CreateTexture2D(&tex_desc, NULL, &texture);
    instance->d3dContext->CopyResource(texture, img);
    SAFE_RELEASE(img);

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    hr = instance->d3dContext->Map(texture, 0, D3D11_MAP_READ, 0, &mappedSubresource);

    // 拷贝到用户buffer
    Frame* pic = NULL;
    if (instance->frame_allocator.get)
    {
        pic = instance->frame_allocator.get(instance->frame_allocator.opaque, FORMAT_ARGB, desc.Width, desc.Height);
        for(UINT i = 0; i<desc.Height; ++i)
        {
            memcpy(pic->buffer->ptr + pic->pitch*i, (char*)mappedSubresource.pData + desc.Width*4*i, desc.Width*4);
        }
    }
    else
    {
        pic = new FrameArgb((const char*)mappedSubresource.pData, desc.Width, desc.Height, desc.Width * 4, Image::Format_ARGB,
                            true, instance->allocator.get, instance->allocator.release, instance->allocator.opaque);
    }

    // 绘制鼠标
    if (frminfo.PointerShapeBufferSize == 0)
    {
        // 鼠标图形没有变化
        if (frminfo.LastMouseUpdateTime.LowPart != 0 || frminfo.LastMouseUpdateTime.HighPart != 0)
        {
            // 如果鼠标有位置变化，更新位置
            DXGI_OUTDUPL_POINTER_POSITION pos = frminfo.PointerPosition;
            instance->curInfo.posX = pos.Position.x;
            instance->curInfo.posY = pos.Position.y;
            instance->curInfo.visiblity = pos.Visible;
        }
        if (instance->showCursor)
        {
            // 绘制鼠标指针
            DrawCursor(instance, pic);
        }
    }
    else
    {
        // 鼠标图形有变化
        if (instance->curInfo.size < frminfo.PointerShapeBufferSize)
        {
            // 空间不够，用Allocator重新申请buffer
            if (instance->curInfo.buf)
                instance->allocator.release(instance->allocator.opaque, instance->curInfo.buf);
            instance->curInfo.buf = instance->allocator.get(instance->allocator.opaque, frminfo.PointerShapeBufferSize);
            instance->curInfo.size = frminfo.PointerShapeBufferSize;
        }
        UINT reqsize = 0;
        DXGI_OUTDUPL_POINTER_SHAPE_INFO ptrinfo = {0};
        hr = instance->dxgiDup->GetFramePointerShape(frminfo.PointerShapeBufferSize, instance->curInfo.buf->ptr, &reqsize, &ptrinfo);

        if (SUCCEEDED(hr))
        {
            // 更新鼠标信息
            DXGI_OUTDUPL_POINTER_POSITION pos = frminfo.PointerPosition;
            instance->curInfo.visiblity = pos.Visible;
            instance->curInfo.posX = pos.Position.x;
            instance->curInfo.posY = pos.Position.y;
            instance->curInfo.width = ptrinfo.Width;
            instance->curInfo.height = ptrinfo.Height;
            instance->curInfo.pitch = ptrinfo.Pitch;
            instance->curInfo.type = ptrinfo.Type;

            if (instance->showCursor)
            {
                // 绘制鼠标指针
                DrawCursor(instance, pic);
            }
        }
        else
        {
            switch (hr) {
            case DXGI_ERROR_ACCESS_LOST:
                break;
            case DXGI_ERROR_MORE_DATA:
                break;
            case DXGI_ERROR_INVALID_CALL:
                break;
            case E_INVALIDARG:
                break;
            default:
                break;
            }
        }
    }

    instance->d3dContext->Unmap(texture, 0);
    SAFE_RELEASE(texture);

    instance->dxgiDup->ReleaseFrame();

    *frame = pic;

    return 0;
}

void SetCursorVisibility(ScreenCapture *instance, int visible)
{
    instance->showCursor = visible!=0;
}

int SetMemAllocator(ScreenCapture *instance, GetBuffer get_buffer, ReleaseBuffer release_buffer, void *allocator)
{
    if (!get_buffer || !release_buffer)
    {
        return E_INVALIDARG;
    }
    instance->allocator.get = get_buffer;
    instance->allocator.release = release_buffer;
    instance->allocator.opaque = allocator;
    return E_OK;
}

int FrameRelease(ScreenCapture* instance, Frame *frame)
{
    if (!frame)
        return E_INVALIDARG;
    if (instance->frame_allocator.release)
    {
        instance->frame_allocator.release(instance->frame_allocator.opaque, frame);
    }
    else
    {
        FrameArgb* p = (FrameArgb*)frame;
        delete p;
    }
    return E_OK;
}

class VideoAdapterImpl : public VideoAdapter
{
public:
    VideoAdapterImpl()
    {
        index = -1;
        outputs = NULL;
        output_count = 0;
        memset(desc,  0, sizeof(desc));
    }

    void InitOutputs(int c)
    {
        if (outputs)
            delete[] outputs;
        outputs = new VideoOutput[c];
        output_count = c;
    }

    ~VideoAdapterImpl()
    {
        if(outputs)
        {
            delete[] outputs;
        }
    }
};

int EnumerateAdaptersAndOutputs(VideoAdapter **adapters, int *adapter_count)
{
    if (!adapters || !adapter_count)
    {
        return E_INVALID_ARGUMENTS;
    }

    IDXGIFactory1 *dxgiFactory;
    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&dxgiFactory));
    if (FAILED(hr))
    {
        return E_DEVICE_NOT_SURPPORT;
    }
    UINT count = 0;
    IDXGIAdapter1* dxgiadapter;
    while(dxgiFactory->EnumAdapters1(count, &dxgiadapter) != DXGI_ERROR_NOT_FOUND)
    {
        ++count;
    }
    if (count == 0)
    {
        return E_DEVICE_NOT_SURPPORT;
    }

    VideoAdapterImpl* adpts = new VideoAdapterImpl[count];
    *adapter_count = count;

    for(UINT i = 0; i<count; ++i)
    {
        hr = dxgiFactory->EnumAdapters1(i, &dxgiadapter);
        if(FAILED(hr))
        {
            goto again;
        }
        VideoAdapterImpl* adpt = &adpts[i];
        adpt->index = i;
        DXGI_ADAPTER_DESC desc;
        dxgiadapter->GetDesc(&desc);
        memcpy(adpt->desc, desc.Description, sizeof(desc.Description));

        UINT c = 0;
        IDXGIOutput* dxgi_output = NULL;
        while(dxgiadapter->EnumOutputs(c, &dxgi_output)!=DXGI_ERROR_NOT_FOUND)
        {
            ++c;
        }
        if (count == 0)
        {
            continue;
        }
        adpt->InitOutputs(c);
        for(int j = 0; j < c; ++j)
        {
            hr = dxgiadapter->EnumOutputs(j, &dxgi_output);
            if (FAILED(hr))
            {
                goto again;
            }
            adpt->outputs[j].index = j;
            DXGI_OUTPUT_DESC desc;
            dxgi_output->GetDesc(&desc);
            memcpy(adpt->outputs[j].name, desc.DeviceName, sizeof (desc.DeviceName));
            adpt->outputs[j].width = desc.DesktopCoordinates.right - desc.DesktopCoordinates.left;
            adpt->outputs[j].height = desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top;
        }

    }

    *adapters = adpts;

    return 0;

again:
    delete[] adpts;
    *adapters = NULL;
    *adapter_count = 0;
    return E_AGAIN;
}

void FreeVideoAdapters(VideoAdapter *adapters)
{
    VideoAdapterImpl* impl = (VideoAdapterImpl*)adapters;
    delete[] impl;
}

int SetFrameAllocator(ScreenCapture *instance, GetFrame get_frame, ReleaseFrame release_frame, void *allocator)
{
    if (!get_frame || !release_frame)
    {
        return E_INVALIDARG;
    }
    instance->frame_allocator.get = get_frame;
    instance->frame_allocator.release = release_frame;
    instance->frame_allocator.opaque = allocator;
    return 0;
}
