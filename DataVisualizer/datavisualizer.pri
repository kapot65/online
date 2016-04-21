QT += printsupport

INCLUDEPATH += $$PWD

SOURCES += $$PWD/qcustomplot.cpp \
    $$PWD/datavisualizerform.cpp \
    $$PWD/customplotzoom.cpp \
    $$PWD/apdfiledrawer.cpp \
    $$PWD/filedrawer.cpp \
    $$PWD/infofiledrawer.cpp \
    $$PWD/pointfiledrawer.cpp \
    $$PWD/voltagefiledrawer.cpp

HEADERS  += $$PWD/qcustomplot.h \
    $$PWD/datavisualizerform.h \
    $$PWD/customplotzoom.h \
    $$PWD/apdfiledrawer.h \
    $$PWD/filedrawer.h \
    $$PWD/infofiledrawer.h \
    $$PWD/pointfiledrawer.h \
    $$PWD/voltagefiledrawer.h

FORMS  += $$PWD/DataVisualizerForm.ui
