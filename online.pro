TEMPLATE = subdirs
unix: !macx{
	SUBDIRS = CCPC7_Server/CCPC7_Server.pro 
}
win32{
SUBDIRS = CamacClient/CamacClient.pro \
		  CCPC7_Server/CCPC7_Server.pro  \
          DataVisualizer/DataVisualizer.pro  \
          HV_Server/HV_Server.pro 
}
