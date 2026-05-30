cmake_minimum_required(VERSION 3.16)

project(Sistem_Monitorizare_Stocuri VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/LibrarieModele
        ${CMAKE_CURRENT_SOURCE_DIR}/ManagerStocare
        ${CMAKE_CURRENT_SOURCE_DIR}/NivelStocareDate
    )

    qt_add_executable(Sistem_Monitorizare_Stocuri
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        LibrarieModele/Produs.h
        LibrarieModele/Produs.cpp
        ManagerStocare/IStocare.h
        ManagerStocare/jsonstorage.h ManagerStocare/jsonstorage.cpp
        LibrarieModele/Tranzactie.h
        NivelStocareDate/warehousemanager.h NivelStocareDate/warehousemanager.cpp
        NivelStocareDate/Tranzactiemanager.h
        styles/styles.qss
        resources.qrc
        NivelUI/addproductdialog.h NivelUI/addproductdialog.cpp
        NivelUI/transactiondialog.h NivelUI/transactiondialog.cpp
        NivelUI/editproductdialog.h NivelUI/editproductdialog.cpp
        NivelUI/productdetailsdialog.h NivelUI/productdetailsdialog.cpp
        NivelUI/settingsdialog.h NivelUI/settingsdialog.cpp
        exportmanager.h exportmanager.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Sistem_Monitorizare_Stocuri APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Sistem_Monitorizare_Stocuri SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Sistem_Monitorizare_Stocuri
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Sistem_Monitorizare_Stocuri PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Sistem_Monitorizare_Stocuri)
endif()
set_target_properties(Sistem_Monitorizare_Stocuri PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Sistem_Monitorizare_Stocuri
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Sistem_Monitorizare_Stocuri)
endif()
