#на компьютере должен быть установлен git
#версия репозитория

REVISION = $$system(git describe --always --tags)
DEFINES += "REP_VERSION=\\\"1\\\""
DEFINES += "APP_REVISION=QString(REP_VERSION)+\\\".\\\"+\\\"$$REVISION\\\""
