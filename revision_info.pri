#на компьютере должен быть установлен git
#версия репозитория
REP_VERSION = 1
REVISION = $$system(git describe --always --tags)
DEFINES += APP_REVISION=\\\"$$REP_VERSION.$$REVISION\\\"
