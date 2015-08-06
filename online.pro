TEMPLATE = subdirs
unix: !macx{
	SUBDIRS = CCPC7_Server/CCPC7_Server.pro 
}
win32{
	contains(QT_MAJOR_VERSION, 4){
		SUBDIRS = \
				  CCPC7_Server/CCPC7_Server.pro  \
	}
	contains(QT_MAJOR_VERSION, 5){
                SUBDIRS = CamacClient/CamacClient.pro \
                                  #CCPC7_Server/CCPC7_Server.pro  \
                                  #DataVisualizer/DataVisualizer.pro  \
                                  HV_Server/HV_Server.pro
	}
}
