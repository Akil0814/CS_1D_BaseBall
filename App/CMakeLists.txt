find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Sql)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "*.cpp")
list(FILTER APP_SOURCES EXCLUDE REGEX ".*/services/trip_planner_cli\\.cpp$")

add_library(app ${APP_SOURCES})

target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(app PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
)
