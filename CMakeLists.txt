#cmake_minimum_required(VERSION 3.16)
#
#project(CS_1D_BaseBall VERSION 0.1 LANGUAGES CXX)
#
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
#
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
#find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)
#
#add_subdirectory(App)
#
#set(PROJECT_SOURCES
#        main.cpp
#        main_window.cpp
#        main_window.h
#        main_window.ui
#        AdminPage/adminpage.cpp
#        AdminPage/adminpage.h
#        AdminPage/adminpage.ui
#        AdminPage/loginpage.cpp
#        AdminPage/loginpage.h
#        AdminPage/loginpage.ui
#        AdminPage/dashboardpage.cpp
#        AdminPage/dashboardpage.h
#        AdminPage/dashboardpage.ui
#        main_page/main_page.cpp
#        main_page/main_page.h
#        main_page/main_page.ui
#        browse_window/browse_window.cpp
#        browse_window/browse_window.h
#        browse_window/browse_window.ui
#        detail_window/detail_window.cpp
#        detail_window/detail_window.h
#        detail_window/detail_window.ui
#)
#
#if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
#    qt_add_executable(CS_1D_BaseBall
#        MANUAL_FINALIZATION
#        ${PROJECT_SOURCES}
#    )
#
#    # Add this right after your add_executable / qt_add_executable block
#    target_include_directories(CS_1D_BaseBall PRIVATE
#            ${CMAKE_CURRENT_SOURCE_DIR}
#            ${CMAKE_CURRENT_SOURCE_DIR}/AdminPage
#            ${CMAKE_CURRENT_SOURCE_DIR}/detail_window
#            ${CMAKE_CURRENT_SOURCE_DIR}/browse_window
#            ${CMAKE_CURRENT_SOURCE_DIR}/main_page
#    )
## Define target properties for Android with Qt 6 as:
##    set_property(TARGET CS_1D_BaseBall APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
##                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
## For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
#
#else()
#    if(ANDROID)
#        add_library(CS_1D_BaseBall SHARED
#            ${PROJECT_SOURCES}
#        )
## Define properties for Android with Qt 5 after find_package() calls as:
##    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    else()
#        add_executable(CS_1D_BaseBall
#            ${PROJECT_SOURCES}
#        )
#    endif()
#endif()
#
#
#target_link_libraries(CS_1D_BaseBall PRIVATE
#        Qt${QT_VERSION_MAJOR}::Widgets
#        Qt${QT_VERSION_MAJOR}::Sql
#        Qt${QT_VERSION_MAJOR}::Core
#        app
#)
#
## Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
## If you are developing for iOS or macOS you should consider setting an
## explicit, fixed bundle identifier manually though.
#if(${QT_VERSION} VERSION_LESS 6.1.0)
#  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CS_1D_BaseBall)
#endif()
#set_target_properties(CS_1D_BaseBall PROPERTIES
#    ${BUNDLE_ID_OPTION}
#    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#    MACOSX_BUNDLE TRUE
#     WIN32_EXECUTABLE FALSE #FALSE for testing only
#)
#
#include(GNUInstallDirs)
#install(TARGETS CS_1D_BaseBall
#    BUNDLE DESTINATION .
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)
#
#if(QT_VERSION_MAJOR EQUAL 6)
#    qt_finalize_executable(CS_1D_BaseBall)
#endif()

cmake_minimum_required(VERSION 3.16)

project(CS_1D_BaseBall VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

# =====================================================
# APP LIB (your backend logic)
# =====================================================
add_subdirectory(App)

# =====================================================
# GUI EXECUTABLE
# =====================================================
set(PROJECT_SOURCES
        main.cpp
        main_window.cpp
        main_window.h
        main_window.ui
        AdminPage/adminpage.cpp
        AdminPage/adminpage.h
        AdminPage/adminpage.ui
        AdminPage/loginpage.cpp
        AdminPage/loginpage.h
        AdminPage/loginpage.ui
        AdminPage/dashboardpage.cpp
        AdminPage/dashboardpage.h
        AdminPage/dashboardpage.ui
        main_page/main_page.cpp
        main_page/main_page.h
        main_page/main_page.ui
        browse_window/browse_window.cpp
        browse_window/browse_window.h
        browse_window/browse_window.ui
        detail_window/detail_window.cpp
        detail_window/detail_window.h
        detail_window/detail_window.ui
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(CS_1D_BaseBall
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
    )
else()
    add_executable(CS_1D_BaseBall ${PROJECT_SOURCES})
endif()

target_include_directories(CS_1D_BaseBall PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/AdminPage
        ${CMAKE_CURRENT_SOURCE_DIR}/main_page
        ${CMAKE_CURRENT_SOURCE_DIR}/browse_window
        ${CMAKE_CURRENT_SOURCE_DIR}/detail_window
)

target_link_libraries(CS_1D_BaseBall PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Core
        app
)

set_target_properties(CS_1D_BaseBall PROPERTIES
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE FALSE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CS_1D_BaseBall)
endif()

# =====================================================
# CLI TEST EXECUTABLE (IMPORTANT PART)
# =====================================================
add_executable(CS_1D_BaseBall_CLI
        App/services/trip_planner_cli.cpp
)

target_include_directories(CS_1D_BaseBall_CLI PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/App
)

target_link_libraries(CS_1D_BaseBall_CLI PRIVATE
        app
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Sql
)

# =====================================================
# INSTALL
# =====================================================
include(GNUInstallDirs)

install(TARGETS CS_1D_BaseBall
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)