#на компьютере должен быть установлен git
#версия репозитория
REVISION = 1.$$system(git describe --always --tags)
DEFINES += APP_REVISION=\\\"$$REVISION\\\"
