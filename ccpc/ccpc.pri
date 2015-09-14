unix: !macx{
INCLUDEPATH += $$PWD/original/

SOURCES += \
    $$PWD/ooriginal/ccpc7.cpp \
    $$PWD/ooriginal/camacop.cpp \
    $$PWD/ooriginal/camacimpl.cpp \
    $$PWD/ooriginal/ccpcisa.cpp \

HEADERS  += \
        $$PWD/ooriginal/ccpc7.h \
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
