TEMPLATE = subdirs
unix: !macx{
        SUBDIRS = CCPC7_Server/CCPC7_Server.pro \
            HV_Server/HV_Server.pro \
            #CamacClient/CamacClient.pro \
            DataVisualizer/DataVisualizer.pro  \
}
win32{
       SUBDIRS =  CPC7_Server/CCPC7_Server.pro \
        HV_Server/HV_Server.pro \
        CamacClient/CamacClient.pro \
        DataVisualizer/DataVisualizer.pro  \
}
