unix: !macx{
INCLUDEPATH += $$PWD/original/

SOURCES += \
    $$PWD/original/ccpc7.cpp \
    $$PWD/original/camacop.cpp \
    $$PWD/original/camacimpl.cpp \
    $$PWD/original/ccpcisa.cpp \

HEADERS  += \
        $$PWD/original/ccpc7.h \
}

win32{
INCLUDEPATH += $$PWD/virtual

SOURCES += $$PWD/virtual/ccpc7.cpp \
        $$PWD/virtual/ccpc7base.cpp \

HEADERS  += $$PWD/virtual/ccpc7.h \
        $$PWD/virtual/ccpc7base.h \
}

#SOURCES += $$PWD/camacalgoritm.cpp

#HEADERS += $$PWD/camacalgoritm.h

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/ccpccommands.h

SOURCES += \
    $$PWD/ccpccommands.cpp
