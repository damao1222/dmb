#TEMPLATE = app
#CONFIG += console
#CONFIG -= app_bundle
CONFIG -= qt
#CONFIG += c99

SOURCES += \
    src/main.c
    
HEADERS += \ 
    src/dmbdefines.h

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
