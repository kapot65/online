TEMPLATE = subdirs
unix: !macx{
        SUBDIRS = CCPC7_Server \
            HV_Server

greaterThan(QT_MAJOR_VERSION, 4){
    SUBDIRS += documentation \
            CamacClient \
            DataVisualizer \
            drs-converter
}

}
win32{
       SUBDIRS =  CCPC7_Server \
        HV_Server \
        CamacClient \
        DataVisualizer \
        drs-converter
}
