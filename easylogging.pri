contains(CONFIG, -std=c++11){
    INCLUDEPATH *= $$PWD/3rdParty/easylogging/
    HEADERS  *= $$PWD/3rdParty/easylogging/easylogging++.h
} else {
    contains(CONFIG, c++11){
        INCLUDEPATH *= $$PWD/3rdParty/easylogging/
        HEADERS  *= $$PWD/3rdParty/easylogging/easylogging++.h
        DEFINES += ELPP_THREAD_SAFE
    } else {
        INCLUDEPATH *= $$PWD/3rdParty/easylogging/c++9/
        HEADERS  *= $$PWD/3rdParty/easylogging/c++9/easylogging++.h
        DEFINES += _ELPP_THREAD_SAFE #не проверено!
    }
}
