cmake_minimum_required(VERSION 3.21)

project(ComprehensiveCalculationTool
    VERSION 2.1.0
    DESCRIPTION "综合计算工具 — 数学计算内核、命令行、Web 前端与 Qt 界面"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# 默认只构建不依赖任何第三方库的部分。
# 内核、命令行与 Web 服务只需要 C++17 编译器；Web 前端是原生 HTML/CSS/JS，
# 无需 npm 或构建步骤。这样在任意平台上都能构建，不受 Qt SDK 体积限制。
option(BUILD_GUI "构建 Qt 图形界面（需要 Qt 6）" OFF)
option(BUILD_CLI "构建命令行界面" ON)
option(BUILD_TESTS "构建单元测试" ON)
option(BUILD_SERVER "构建 Web 服务端" ON)

# ---------------------------------------------------------------- 核心算法库
add_library(calc_core STATIC
    src/core/Numeric.cpp
    src/core/NumberTheory.cpp
    src/core/Statistics.cpp
    src/core/Algebra.cpp
    src/core/Geometry.cpp
    src/core/InputParsing.cpp
    src/core/LinearAlgebra.cpp
    src/core/Sequences.cpp
    src/core/ComplexMath.cpp
    src/core/Json.cpp
    src/core/HttpServer.cpp
)

# HTTP 服务需要套接字库。
if(WIN32)
    target_link_libraries(calc_core PUBLIC ws2_32)
endif()

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

# ---------------------------------------------------------------- Web 服务端
if(BUILD_SERVER)
    add_executable(calc_server
        src/server/main.cpp
        src/api/ApiRoutes.cpp
        src/api/ApiRoutes.h
    )
    target_link_libraries(calc_server PRIVATE calc_core)
    target_include_directories(calc_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    if(MSVC)
        target_compile_options(calc_server PRIVATE /W4 /utf-8)
    endif()

    # 前端资源随构建产物一起部署，方便直接从 build 目录启动。
    # 用独立的目标而非 POST_BUILD，这样即使可执行文件没有重新链接，
    # 前端改动也会被同步过去。
    add_custom_target(web_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/web" "$<TARGET_FILE_DIR:calc_server>/web"
        COMMENT "同步 Web 前端资源到构建目录")
    add_dependencies(calc_server web_assets)
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
        tests/tst_extended.cpp
        tests/tst_json.cpp
        tests/tst_http.cpp
    )
    target_link_libraries(calc_tests PRIVATE calc_core)
    if(MSVC)
        target_compile_options(calc_tests PRIVATE /W4 /utf-8)
    endif()

    add_test(NAME calc_tests COMMAND calc_tests)

    # API 端到端测试：启动服务 → 调用各端点 → 校验返回值。
    # 需要 Python 3；没有时跳过该用例而不是让构建失败。
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND AND BUILD_SERVER)
        # 用一个独立的 fixture 脚本负责启停服务，避免测试依赖图出现环。
        add_test(NAME api_test
                 COMMAND ${Python3_EXECUTABLE}
                         "${CMAKE_CURRENT_SOURCE_DIR}/scripts/api_test.py"
                         8099
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

        # 服务由 api_test.py 自行拉起与关闭，这里只登记超时。
        set_tests_properties(api_test PROPERTIES TIMEOUT 180)
    endif()
endif()
