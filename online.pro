TEMPLATE = subdirs
unix: !macx{
        SUBDIRS = CCPC7_Server \
            HV_Server \
            #CamacClient \
            DataVisualizer  \
}
win32{
       SUBDIRS =  CCPC7_Server \
        HV_Server \
        CamacClient \
        DataVisualizer \
}
