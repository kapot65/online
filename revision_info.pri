#�� ���������� ������ ���� ��������� git
REVISION = $$system(git describe --always --tags)
DEFINES += APP_REVISION=\\\"$$REVISION\\\"
