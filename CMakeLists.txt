cmake_minimum_required(VERSION 3.16)

project(VertigoDualCamera VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets MultimediaWidgets SerialPort core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets MultimediaWidgets SerialPort core Core5Compat)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        modifiedWidgets.h
        modifiedWidgets.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(VertigoDualCamera
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        videoexporter.h videoexporter.cpp videoexporter.ui
        assets.qrc
        resources.rc

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET VertigoDualCamera APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(VertigoDualCamera SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(VertigoDualCamera
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(VertigoDualCamera PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(VertigoDualCamera PRIVATE Qt${QT_VERSION_MAJOR}::MultimediaWidgets)
target_link_libraries(VertigoDualCamera PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)
target_link_libraries(VertigoDualCamera PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(VertigoDualCamera PRIVATE Qt${QT_VERSION_MAJOR}::Core5Compat)
# target_link_libraries(VertigoDualCamera PRIVATE Qt${QT_VERSION_MAJOR}::Gui)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.VertigoDualCamera)
endif()
set_target_properties(VertigoDualCamera PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS VertigoDualCamera
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(VertigoDualCamera)
endif()

add_custom_command(
        TARGET VertigoDualCamera POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/VideoStitcherPy.exe"
        "$<TARGET_FILE_DIR:VertigoDualCamera>/VideoStitcherPy.exe"
)

add_custom_command(
        TARGET VertigoDualCamera POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/VideoStitcherPy.dist"
        "$<TARGET_FILE_DIR:VertigoDualCamera>/VideoStitcherPy.dist"
)
