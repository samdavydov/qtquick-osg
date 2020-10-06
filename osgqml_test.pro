QT += opengl widgets quick
CONFIG += c++11

DESTDIR = $$PWD/../osgqml_test_bin
CONFIG(debug, debug|release):DESTDIR = $$join(DESTDIR,,,_debug)

OSG = $$PWD/../../sukhoi/platform

win32:OSGROOT = $$OSG/osg363/win64/
linux:OSGROOT = $$OSG/osg363/astra/
OSGLIB = $$OSGROOT

CONFIG(release, debug|release):OSGLIB = $$join(OSGLIB,,,release)
CONFIG(debug  , debug|release):OSGLIB = $$join(OSGLIB,,,debug)

OSG_CORE     = osg
OSG_VIEWER   = osgViewer
OSG_DB       = osgDB
OSG_GA       = osgGA
OSG_FX       = osgFX
OSG_THREADS  = OpenThreads
OSG_TEXT     = osgText
OSG_UTIL     = osgUtil
OSG_PARTICLE = osgParticle
OSG_SHADOW   = osgShadow

CONFIG(debug, debug|release) {
    OSG_CORE     = $$join(OSG_CORE,,,d)
    OSG_VIEWER   = $$join(OSG_VIEWER,,,d)
    OSG_DB       = $$join(OSG_DB,,,d)
    OSG_GA       = $$join(OSG_GA,,,d)
    OSG_FX       = $$join(OSG_FX,,,d)
    OSG_THREADS  = $$join(OSG_THREADS,,,d)
    OSG_TEXT     = $$join(OSG_TEXT,,,d)
    OSG_UTIL     = $$join(OSG_UTIL,,,d)
    OSG_PARTICLE = $$join(OSG_PARTICLE,,,d)
    OSG_SHADOW   = $$join(OSG_SHADOW,,,d)
}

LIBS *= \
    -L"$$OSGLIB" \
    -l$$OSG_CORE \
    -l$$OSG_VIEWER \
    -l$$OSG_DB \
    -l$$OSG_GA \
    -l$$OSG_FX \
    -l$$OSG_THREADS \
    -l$$OSG_TEXT \
    -l$$OSG_UTIL \
    -l$$OSG_PARTICLE \
    -l$$OSG_SHADOW \

INCLUDEPATH *= \
    $$OSGROOT/include \

DEPENDPATH *= \
    INCLUDEPATH

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/widget.cpp \
    $$PWD/OsgWidget.cpp \
    $$PWD/ImageSource.cpp \

HEADERS += \
    $$PWD/widget.h \
    $$PWD/OsgWidget.h \
    $$PWD/ImageSource.h \
