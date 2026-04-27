find_package(QT NAMES Qt6 Qt5  Sql)

file(GLOB_RECURSE APP CONFIGURE_DEPENDS "*.cpp")
add_library(app ${APP}
        services/trip_planner_cli.cpp
)
target_link_libraries(app PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt6::Sql)
