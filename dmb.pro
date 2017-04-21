#TEMPLATE = app
#CONFIG += console
#CONFIG -= app_bundle
CONFIG -= qt
#CONFIG += c99

INCLUDEPATH += src

SOURCES += \
    src/main.c \
    src/core/dmbbinlist.c \
    src/core/dmballoc.c \
    src/core/dmbdict.c \
    src/core/dmblist.c \
    src/core/dmbskiplist.c \
    src/utils/dmbsysutil.c \
    src/utils/dmblog.c \
    src/utils/dmbtime.c \
    src/utils/dmbfilesystem.c \
    src/tests/dmbbinlist_test.c \
    src/core/dmbstring.c \
    src/tests/dmbstring_test.c \
    src/base/dmbcommand.c \
    src/base/dmbobject.c \
    src/base/dmbdllist.c \
    src/tests/dmbdllist_test.c \
    src/network/dmbnetwork.c
    
HEADERS += \ 
    src/dmbdefines.h \
    src/core/dmbbinlist.h \
    src/core/dmballoc.h \
    src/core/dmbdict.h \
    src/core/dmblist.h \
    src/core/dmbskiplist.h \
    src/utils/dmbsysutil.h \
    src/utils/dmblog.h \
    src/thread/dmbatomic.h \
    src/utils/dmbtime.h \
    src/utils/dmbfilesystem.h \
    src/tests/dmbbinlist_test.h \
    src/core/dmbstring.h \
    src/tests/dmbstring_test.h \
    src/base/dmbcommand.h \
    src/base/dmbobject.h \
    src/base/dmbdllist.h \
    src/tests/dmbdllist_test.h \
    src/network/dmbnetwork.h \
    src/utils/dmbioutil.h

DEFINES += DMB_USE_JEMALLOC
DEFINES += DMB_DEBUG

#debug {
#QMAKE_CFLAGS = -std=gnu99  -O0  -g3
#DEFINES += DMB_DEBUG
#}

##-U_FORTIFY_SOURCE open compile error
#release {
#QMAKE_CFLAGS = -std=gnu99  -O3  -g3 -U_FORTIFY_SOURCE
#}

LIBS += -L ../dmb.git/trunk/libs  \
-ldl \
-ljemalloc \
-lm \
-pthread \
-lz
