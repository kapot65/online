contains(CONFIG, -std=c++11){
    INCLUDEPATH *= $$PWD/3rdParty/easylogging/
    HEADERS  *= $$PWD/3rdParty/easylogging/easylogging++.h
    DEFINES += EL_CPP11
} else {
    contains(CONFIG, c++11){
        INCLUDEPATH *= $$PWD/3rdParty/easylogging/
        HEADERS  *= $$PWD/3rdParty/easylogging/easylogging++.h
        DEFINES += ELPP_THREAD_SAFE
        DEFINES += EL_CPP11
    } else {
        INCLUDEPATH *= $$PWD/3rdParty/easylogging/c++9/
        HEADERS  *= $$PWD/3rdParty/easylogging/c++9/easylogging++.h
        DEFINES += _ELPP_THREAD_SAFE
    }
}


