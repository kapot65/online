TEMPLATE = subdirs
unix: !macx{
    greaterThan(QT_MAJOR_VERSION, 4){
        SUBDIRS += CamacClient
}

}
win32{
    SUBDIRS = CamacClient
}
