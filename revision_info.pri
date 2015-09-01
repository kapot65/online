#на компьютере должен быть установлен git
#версия репозитория
REVISION = 1.$$system(git describe --always --tags)
DEFINES += APP_REVISION=\\\"$$REVISION\\\"

#Особые параметры для компиляторов (убрать в другое место)
win32:msvc{
    DEFINES += NOMINMAX
    #Добавление поддержки openmp
    QMAKE_CXXFLAGS_RELEASE += /openmp
    QMAKE_CXXFLAGS_DEBUG += /openmp
}
