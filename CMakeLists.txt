list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(common)

set(SRC
        src/main.cpp
        src/MainWindow.cpp
        src/MainWindow.h
        src/MainWindow.ui
        src/RenderArea.cpp
        src/RenderArea.h
        src/Subject.h

        # thirdparty over GPL
        src/QCustomPlot/qcustomplot.cpp
        src/QCustomPlot/qcustomplot.h
        )

add_executable(covid-19 ${SRC})

find_package(Qt5 COMPONENTS Widgets PrintSupport XmlPatterns REQUIRED)

target_link_libraries(covid-19
        PRIVATE
        Qt5::Widgets
        Qt5::PrintSupport
        Qt5::XmlPatterns
        )

target_compile_features(covid-19 PRIVATE cxx_std_17)
