#ifndef SCREEPCAPTURE_H
#define SCREEPCAPTURE_H

#include <ScreenCapture/BufferAlloc.h>
#include <ScreenCapture/ErrorCodes.h>

#include <stdint.h>

#ifdef DXGISCREENCAPTURE_STATIC
#define DXGISCREENCAPTURE_PUBLIC
#define DXGISCREENCAPTURE_LOCAL
#else
  #if defined _WIN32 || defined __CYGWIN__
    #ifdef BUILDING_DXGISCREENCAPTURE
      #ifdef __GNUC__
        #define DXGISCREENCAPTURE_PUBLIC __attribute__ ((dllexport))
      #else
        #define DXGISCREENCAPTURE_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
      #endif
    #else
      #ifdef __GNUC__
        #define DXGISCREENCAPTURE_PUBLIC __attribute__ ((dllimport))
      #else
        #define DXGISCREENCAPTURE_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
      #endif
    #endif
    #define DXGISCREENCAPTURE_LOCAL
  #else
    #if __GNUC__ >= 4
      #define DXGISCREENCAPTURE_PUBLIC __attribute__ ((visibility ("default")))
      #define DXGISCREENCAPTURE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
      #define DXGISCREENCAPTURE_PUBLIC
      #define DXGISCREENCAPTURE_LOCAL
    #endif
  #endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct VideoOutput {
    uint32_t index;
    wchar_t name[32];
    uint32_t width;
    uint32_t height;
} VideoOutput;

typedef struct VideoAdapter{
    uint32_t index;
    wchar_t desc[128];
    VideoOutput* outputs;
    uint32_t output_count;
} VideoAdapter;

typedef struct ScreenCapture ScreenCapture;

/**
 * @brief Enumerate video adapter and output devices.
 * @param adapters [out] pointer to an array of VideoAdapter
 * @param adapter_count [out] item count of array @em adapters
 * @return error code, see ErrorCodes.h
 * @remark call @em FreeVideoAdapters to release @em adapters when finish using it
 */
DXGISCREENCAPTURE_PUBLIC int EnumerateAdaptersAndOutputs(VideoAdapter** adapters, int* adapter_count);
/**
 * @brief FreeVideoAdapters
 * @param adapters pointer to a VideoAdapter array returned by @em EnumerateAdaptersAndOutputs
 */
DXGISCREENCAPTURE_PUBLIC void FreeVideoAdapters(VideoAdapter* adapters);

/**
 * @brief Create a ScreenCapture instance.
 * @param adapter_index VideoAdapter device's index to capture, you can get them by calling @em EnumerateAdaptersAndOutputs
 * @param display_index VideoOutput device's index to capture, you can get them by calling @em EnumerateAdaptersAndOutputs
 * @param capture [out] a ScreenCapture instance
 * @return error code, see ErrorCodes.h
 * @sa EnumerateAdaptersAndOutputs
 */
DXGISCREENCAPTURE_PUBLIC int CreateScreenCapture(int adapter_index, int display_index, ScreenCapture** capture);
/**
 * @brief Destroy ScreenCapture instance.
 * @param instance the pointer returned by @em CreateScreenCapture
 */
DXGISCREENCAPTURE_PUBLIC void DestroyScreenCapture(ScreenCapture* instance);

#define MAKE_FOURCC(a,b,c,d) \
( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )

#define FORMAT_ARGB MAKE_FOURCC('A','R','G','B')  // only support ARGB

typedef struct Frame
{
    Buffer* buffer;
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
} Frame;
typedef Frame* (*GetFrame)(void* allocator, uint32_t format, uint32_t width, uint32_t height);
typedef void (*ReleaseFrame)(void* allocator, Frame* frame);
/**
 * @brief SetFrameAllocator
 * @param instance a ScreenCapture instance
 * @param get_frame ScreenCapture will use the buffer allocated by this callback to receive screen image data, can not be null
 * @param release_frame callback to release a frame allocated by @em get_frame, can not be null
 * @param allocator the allocator context
 * @return error code
 * Frame allocator allows a memory usage with high efficiency, if you don't specify one, buffer allocator will be used, se @em SetMemAllocator.
 */
DXGISCREENCAPTURE_PUBLIC int SetFrameAllocator(ScreenCapture* instance, GetFrame get_frame, ReleaseFrame release_frame, void* allocator);

/**
 * @brief Set memory allocator.
 * @param instance a ScreenCapture instance
 * @param get_buffer ScreenCapture will use the buffer allocated by this callback to receive screen image data, can not be null
 * @param release_buffer callback to release a buffer allocated by @em get_buffer, can not be null
 * @param allocator the allocator context
 * @return error code
 * @remark Memory allocator allows a memory usage with high efficiency, if you don't specify one, \n
 *  a default allocator will be used, which performs as common new and delete.
 */
DXGISCREENCAPTURE_PUBLIC int SetMemAllocator(ScreenCapture* instance, GetBuffer get_buffer, ReleaseBuffer release_buffer, void* allocator);

/**
 * @brief Set cursor visibility.
 * @param instance ScreenCapture instance
 * @param visible 1: show cursor, 0: hide cursor
 * @remark the default value is 1
 */
DXGISCREENCAPTURE_PUBLIC void SetCursorVisibility(ScreenCapture* instance, int visible);

#define TIMEOUT_INFINITE 0xFFFFFFFF

/**
 * @brief TakeSnapshot
 * @param instance
 * @param timeout_ms timeout interval, 0: return immediately, TIMEOUT_INFINITE: never timeout
 * @param frame [out] a frame with a buffer allocated by @em get_buffer callback
 * @return error code
 */
DXGISCREENCAPTURE_PUBLIC int TakeSnapshot(ScreenCapture* instance, uint32_t timeout_ms, Frame** frame);
/**
 * @brief ReleaseFrame
 * @param instance a ScreenCapture instance
 * @param frame pointer to a frame object returned by @em TakeSnapshot()
 * @return error code
 */
DXGISCREENCAPTURE_PUBLIC int FrameRelease(ScreenCapture* instance, Frame* frame);

#ifdef __cplusplus
}
#endif

#endif // SCREEPCAPTURE_H
