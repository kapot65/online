cmake_minimum_required(VERSION 3.16)

project(online VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets PrintSupport Gui Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport Gui Network)

include(cmake/git-rev.cmake)

# add dependencies
include(cmake/CPM.cmake)

# add easyloggingpp
add_definitions(
    -DEL_CPP11 
    -DELPP_QT_LOGGING 
    -DELPP_FEATURE_ALL 
    -DELPP_STL_LOGGING
    -DELPP_STRICT_SIZE_CHECK
    -DELPP_UNICODE
    -DELPP_MULTI_LOGGER_SUPPORT
    -DELPP_THREAD_SAFE
)          
CPMAddPackage(
    NAME easyloggingpp
    GITHUB_REPOSITORY abumq/easyloggingpp
    GIT_TAG v9.97.1
)

add_definitions(-DUSE_QTJSON)
add_definitions(-DBIN_NAME="online")
add_definitions(-DAPP_REVISION="${GIT_REVISION}")

set(PROJECT_SOURCES
    src/online/camacclientform.cpp
    src/online/camacclientform.ui
    src/online/ccpc7handler.cpp
    src/online/ccpc7handlerform.cpp
    src/online/ccpc7handlerform.ui
    src/online/hvhandler.cpp
    src/online/hvhandlerform.cpp
    src/online/hvhandlerform.ui
    src/online/hvmonitor.cpp
    src/online/main.cpp
    src/online/online.cpp
    src/online/onlineform.cpp
    src/online/onlineform.ui
    src/online/serverhandler.cpp
    src/tcp/event.cpp 
    src/tcp/inimanager.cpp
    src/tcp/tcpbase.cpp
    src/tcp/tcpclient.cpp
    src/tcp/tcpprotocol.cpp
    src/tcp/tcpserver.cpp
    src/common/tempfolder.cpp
    src/common/resources.qrc
    ${easyloggingpp_SOURCE_DIR}/src/easylogging++.cc
)

include_directories(src/online)
include_directories(src/tcp)
include_directories(src/common)
include_directories(${easyloggingpp_SOURCE_DIR}/src)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(online
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET online APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(online SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(online
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(online PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.online)
endif()
set_target_properties(online PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS online
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(online)
endif()
