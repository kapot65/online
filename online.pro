TEMPLATE = subdirs
unix: !macx{
        SUBDIRS = CCPC7_Server \
            HV_Server \
            CamacClient \
            DataVisualizer

greaterThan(QT_MAJOR_VERSION, 4){
    SUBDIRS += documentation
}

}
win32{
       SUBDIRS =  CCPC7_Server \
        HV_Server \
        CamacClient \
        DataVisualizer \
}

