TEMPLATE = subdirs
unix: !macx{
	SUBDIRS = CCPC7_Server
}
win32{
SUBDIRS = CamacClient \
		  CCPC7_Server \
          DataVisualizer \
          HV_Server
}
