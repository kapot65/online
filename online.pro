TEMPLATE = subdirs
unix: !macx{
        SUBDIRS = CCPC7_Server \
            HV_Server

greaterThan(QT_MAJOR_VERSION, 4){
    SUBDIRS +=  \
            CamacClient \
            DataVisualizer \
            documentation  \
#            drs-converter
}

}
win32{
       SUBDIRS =  CCPC7_Server \
        HV_Server \
        CamacClient \
        DataVisualizer \
#        drs-converter
}
