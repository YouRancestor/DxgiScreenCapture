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

typedef struct ScreenCapture ScreenCapture;

DXGISCREENCAPTURE_PUBLIC int CreateScreenCapture(int adapter_index, int display_index, ScreenCapture** capture);
DXGISCREENCAPTURE_PUBLIC void DestroyScreenCapture(ScreenCapture* instance);

#define MAKE_FOURCC(a,b,c,d) \
( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )

#define FORMAT_ARGB MAKE_FOURCC('A','R','G','B')

typedef struct Frame
{
    Buffer* buffer;
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
} Frame;

DXGISCREENCAPTURE_PUBLIC void SetMemAllocator(ScreenCapture* instance, GetBuffer get_buffer, ReleaseBuffer release_buffer, void* allocator);

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
 * @param frame pointer to a frame object returned by @em TakeSnapshot()
 * @return
 */
DXGISCREENCAPTURE_PUBLIC int FrameRelease(Frame* frame);

#ifdef __cplusplus
}
#endif

#endif // SCREEPCAPTURE_H
