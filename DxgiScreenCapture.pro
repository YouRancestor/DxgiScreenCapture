TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

TARGET = dxgi_screen_capture

DEFINES += BUILDING_DXGISCREENCAPTURE

INCLUDEPATH += include

SOURCES += \
    ScreenCapture.cpp \
    image.cpp \
    screencaptureimpl.cpp

HEADERS += \
    definitions.h \
    include/ScreenCapture/ErrorCodes.h \
    include/ScreenCapture/BufferAlloc.h \
    include/ScreenCapture/ScreenCapture.h \
    image.h \
    screencaptureimpl.h

LIBS += DXGI.lib D3D11.lib Ole32.lib
