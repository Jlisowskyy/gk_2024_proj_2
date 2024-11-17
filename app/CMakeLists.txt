cmake_minimum_required(VERSION 3.16)

project(app VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_RESOURCES
        images.qrc
        data.qrc
)

set(PROJECT_SOURCES
        include/main.cpp
        include/mainwindow.cpp
        include/mainwindow.h
        include/mainwindow.ui
        src/ToolBar.cpp
        include/UiObjects/ToolBar.h
        src/DrawingWidget.cpp
        include/GraphicObjects/DrawingWidget.h
        include/Constants.h
        src/Constants.cpp
        src/DoubleSlider.cpp
        include/UiObjects/DoubleSlider.h
        src/TextButton.cpp
        include/UiObjects/TextButton.h
        src/StateMgr.cpp
        include/ManagingObjects/StateMgr.h
        src/BezierPoint3D.cpp
        include/GraphicObjects/BezierPoint3D.h
        include/PrimitiveData/Vertex.h
        include/PrimitiveData/Triangle.h
        src/Vertex.cpp
        include/PrimitiveData/ActiveEdge.h
        src/BitMap.cpp
        include/Rendering/BitMap.h
        include/Rendering/Mesh.h
        src/Mesh.cpp
        src/Texture.cpp
        include/Rendering/Texture.h
        include/Intf.h
        include/ManagingObjects/SceneMgr.h
        src/SceneMgr.cpp
)

if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(app
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            ${PROJECT_RESOURCES}
    )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET app APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else ()
    if (ANDROID)
        add_library(app SHARED
                ${PROJECT_SOURCES}
                ${PROJECT_RESOURCES}
        )
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else ()
        add_executable(app
                ${PROJECT_SOURCES}
                ${PROJECT_RESOURCES}
        )
    endif ()
endif ()

target_link_libraries(app PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

find_package(OpenMP)

if(OpenMP_CXX_FOUND)
    target_link_libraries(app PRIVATE OpenMP::OpenMP_CXX)
endif()


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if (${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.app)
endif ()
set_target_properties(app PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (BUILD_TYPE_UPPER STREQUAL "RELEASE")
        target_compile_options(app PUBLIC
                -O3
                -march=native
                -fopenmp
                -funroll-loops
                -march=native
                -funroll-loops
        )
    elseif (BUILD_TYPE_UPPER STREQUAL "DEBUG")
        target_compile_options(app PUBLIC
                -fopenmp
                -march=native
        )
    else()
        message(FATAL_ERROR "UNKNOWN BUILD TYPE: ${BUILD_TYPE_UPPER}")
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (BUILD_TYPE_UPPER STREQUAL "RELEASE")
        target_compile_options(app PUBLIC
                -O3
                -march=native
                -fopenmp
                -march=native
                -funroll-loops
        )
    elseif (BUILD_TYPE_UPPER STREQUAL "DEBUG")
        target_compile_options(app PUBLIC
                -fopenmp
                -march=native
        )
    else()
        message(FATAL_ERROR "UNKNOWN BUILD TYPE: ${BUILD_TYPE_UPPER}")
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel" OR CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
    if (BUILD_TYPE_UPPER STREQUAL "RELEASE")
        target_compile_options(app PUBLIC
                -O3
                -march=native
                -fopenmp
                -fno-tracer
                -march=native
        )
    elseif (BUILD_TYPE_UPPER STREQUAL "DEBUG")
        target_compile_options(app PUBLIC
                -fopenmp
                -march=native
        )
    else()
        message(FATAL_ERROR "UNKNOWN BUILD TYPE: ${BUILD_TYPE_UPPER}")
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if (BUILD_TYPE_UPPER STREQUAL "RELEASE")
        target_compile_options(app PUBLIC
                /O2
                /openmp
                /arch:AVX2
        )
    elseif (BUILD_TYPE_UPPER STREQUAL "DEBUG")
        target_compile_options(app PUBLIC
                /openmp
                /arch:AVX2
        )
    else()
        message(FATAL_ERROR "UNKNOWN BUILD TYPE: ${BUILD_TYPE_UPPER}")
    endif()

else()
    message(FATAL_ERROR "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

include(GNUInstallDirs)
install(TARGETS app
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(app)
endif ()
