unix: !macx{
    INCLUDEPATH += $$PWD/3rdParty/easylogging/c++9/
    HEADERS  += $$PWD/3rdParty/easylogging/c++9/easylogging++.h
}
win32{
contains(QT_MAJOR_VERSION, 4){
    INCLUDEPATH += $$PWD/3rdParty/easylogging/c++9/
    HEADERS  += $$PWD/3rdParty/easylogging/c++9/easylogging++.h
}
contains(QT_MAJOR_VERSION, 5){
    CONFIG += c++11
    INCLUDEPATH += $$PWD/3rdParty/easylogging/
    HEADERS  += $$PWD/3rdParty/easylogging/easylogging++.h
}
}
