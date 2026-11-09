cmake_minimum_required(VERSION 3.21)

project(ComprehensiveCalculationTool
    VERSION 2.0.0
    DESCRIPTION "综合计算工具 — 数学计算核心与 Qt 界面"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(BUILD_GUI "构建 Qt 图形界面" ON)
option(BUILD_CLI "构建命令行界面" ON)
option(BUILD_TESTS "构建单元测试" ON)

# ---------------------------------------------------------------- 核心算法库
add_library(calc_core STATIC
    src/core/Numeric.cpp
    src/core/NumberTheory.cpp
    src/core/Statistics.cpp
    src/core/Algebra.cpp
    src/core/Geometry.cpp
    src/core/InputParsing.cpp
)

target_include_directories(calc_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(MSVC)
    target_compile_options(calc_core PRIVATE /W4 /permissive- /utf-8)
    target_compile_definitions(calc_core PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(calc_core PRIVATE -Wall -Wextra)
endif()

# ---------------------------------------------------------------- 命令行界面
if(BUILD_CLI)
    add_executable(calc_cli src/cli/main.cpp)
    target_link_libraries(calc_cli PRIVATE calc_core)
    if(MSVC)
        target_compile_options(calc_cli PRIVATE /W4 /utf-8)
    endif()
endif()

# ---------------------------------------------------------------- 图形界面
if(BUILD_GUI)
    set(CMAKE_AUTOMOC ON)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    add_executable(calc_gui
        src/ui/main.cpp
        src/ui/MainWindow.cpp
        src/ui/MainWindow.h
        src/ui/UiCommon.cpp
        src/ui/UiCommon.h
        src/ui/PrimeFactorWidget.cpp
        src/ui/PrimeFactorWidget.h
        src/ui/StatisticsWidget.cpp
        src/ui/StatisticsWidget.h
        src/ui/EquationWidget.cpp
        src/ui/EquationWidget.h
        src/ui/GeometryWidget.cpp
        src/ui/GeometryWidget.h
        src/ui/TriangleWidget.cpp
        src/ui/TriangleWidget.h
    )

    target_link_libraries(calc_gui PRIVATE calc_core Qt6::Widgets)
    if(MSVC)
        target_compile_options(calc_gui PRIVATE /W4 /utf-8)
    endif()

    # GUI 运行时需要 Qt 插件目录，便于从构建目录直接启动。
    if(WIN32)
        set_target_properties(calc_gui PROPERTIES
            WIN32_EXECUTABLE TRUE
            VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    # 离屏渲染冒烟测试：无需显示器即可验证各页面能正确构建、计算并绘制。
    if(BUILD_TESTS)
        add_executable(gui_smoke tests/gui_smoke.cpp
            src/ui/MainWindow.cpp
            src/ui/UiCommon.cpp
            src/ui/PrimeFactorWidget.cpp
            src/ui/StatisticsWidget.cpp
            src/ui/EquationWidget.cpp
            src/ui/GeometryWidget.cpp
            src/ui/TriangleWidget.cpp
        )
        target_link_libraries(gui_smoke PRIVATE calc_core Qt6::Widgets)
        target_include_directories(gui_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ui)
        if(MSVC)
            target_compile_options(gui_smoke PRIVATE /W4 /utf-8)
        endif()

        add_test(NAME gui_smoke
                 COMMAND gui_smoke ${CMAKE_CURRENT_BINARY_DIR}/screenshots)
        set_tests_properties(gui_smoke PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    endif()
endif()

# ---------------------------------------------------------------- 单元测试
if(BUILD_TESTS)
    enable_testing()

    add_executable(calc_tests
        tests/tst_numbertheory.cpp
        tests/tst_statistics.cpp
        tests/tst_algebra.cpp
        tests/tst_geometry.cpp
        tests/tst_inputparsing.cpp
        tests/tst_core.cpp
    )
    target_link_libraries(calc_tests PRIVATE calc_core)
    if(MSVC)
        target_compile_options(calc_tests PRIVATE /W4 /utf-8)
    endif()

    add_test(NAME calc_tests COMMAND calc_tests)
endif()
