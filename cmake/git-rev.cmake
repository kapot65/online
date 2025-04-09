find_package(Git QUIET)

if(GIT_FOUND)
    # Выполнить команду git для получения хеша последнего коммита
    # Используйте CMAKE_SOURCE_DIR, если CMakeLists.txt в корне проекта,
    # или PROJECT_SOURCE_DIR, если используется project()
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_REV_HASH_RAW
        ERROR_QUIET       # Подавить вывод ошибок git (если не репозиторий и т.д.)
        RESULT_VARIABLE GIT_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE # Удалить перенос строки в конце вывода
    )

    # Проверить, успешно ли выполнилась команда git
    if(GIT_RESULT EQUAL 0)
        set(GIT_REVISION ${GIT_REV_HASH_RAW})
        message(STATUS "Git Revision (HEAD): ${GIT_REVISION}")

        # Опционально: получить короткий хеш
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_SHORT_REV_HASH_RAW
            RESULT_VARIABLE GIT_SHORT_RESULT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(GIT_SHORT_RESULT EQUAL 0)
            set(GIT_SHORT_REVISION ${GIT_SHORT_REV_HASH_RAW})
            message(STATUS "Git Short Revision (HEAD): ${GIT_SHORT_REVISION}")
        else()
            set(GIT_SHORT_REVISION "unknown-short")
        endif()

        # Опционально: получить более описательную версию (тег или хеш)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --always --dirty --tags
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_DESCRIBE_RAW
            RESULT_VARIABLE GIT_DESCRIBE_RESULT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(GIT_DESCRIBE_RESULT EQUAL 0)
           set(GIT_DESCRIBE ${GIT_DESCRIBE_RAW})
           message(STATUS "Git Describe: ${GIT_DESCRIBE}")
        else()
           set(GIT_DESCRIBE "unknown-describe")
        endif()

    else()
        # Команда git не удалась (например, это не git репозиторий)
        message(WARNING "Failed to get Git revision. Not a git repository or git error.")
        set(GIT_REVISION "unknown")
        set(GIT_SHORT_REVISION "unknown-short")
        set(GIT_DESCRIBE "unknown-describe")
    endif()
else()
    # Git не найден в системе
    message(WARNING "Git executable not found. Cannot determine Git revision.")
    set(GIT_REVISION "no-git")
    set(GIT_SHORT_REVISION "no-git-short")
    set(GIT_DESCRIBE "no-git-describe")
endif()
