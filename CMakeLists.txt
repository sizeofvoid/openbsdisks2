cmake_minimum_required(VERSION 3.16.0)

# Version setup
set(OPENBSDISKS_VERSION_MAJOR "0")
set(OPENBSDISKS_VERSION_MINOR "1")
set(OPENBSDISKS_VERSION_PATCH "0")
set(OPENBSDISKS_VERSION "${OPENBSDISKS_VERSION_MAJOR}.${OPENBSDISKS_VERSION_MINOR}")
#set(OPENBSDISKS_VERSION "${OPENBSDISKS_VERSION_MAJOR}.${OPENBSDISKS_VERSION_MINOR}.${OPENBSDISKS_VERSION_PATCH}")

project(openbsdisks2
        VERSION ${OPENBSDISKS_VERSION}
        DESCRIPTION "UDisks2 service implementation for OpenBSD"
        HOMEPAGE_URL "https://rsadowski.de/")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
            "Choose the type of build, options are: Debug Release RelWithDebInfo Profile"
            FORCE)
endif()

include(GNUInstallDirs)

option(INSTALL_POLKIT_RULES "Install polkit rules" OFF)
option(INSTALL_MAN_PAGES "Install man pages" ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(QT5COMPONENTS Core DBus)
find_package(Qt5 COMPONENTS ${QT5COMPONENTS} REQUIRED)
find_package(Threads REQUIRED)

set(SOURCE
    src/main.cpp
    src/adaptors.cpp
    src/block.cpp
    src/blockfilesystem.cpp
    src/blockpartition.cpp
    src/blockparttable.cpp
    src/drive.cpp
    src/manageradaptor.cpp
    src/objectmanager.cpp

    src/disk_thread.cpp
    src/disk_label.cpp
)
add_executable(openbsdisks2 ${SOURCE})

target_link_libraries(openbsdisks2 Qt5::DBus Qt5::Core util)

target_compile_features(openbsdisks2 PRIVATE cxx_std_17)
target_compile_options(openbsdisks2 PRIVATE "-Wall")
target_compile_options(openbsdisks2 PRIVATE "-pipe")
target_compile_options(openbsdisks2 PRIVATE "-fexceptions")

install(TARGETS openbsdisks2
        RUNTIME DESTINATION bin COMPONENT Runtime)

configure_file(org.freedesktop.UDisks2.service.in org.freedesktop.UDisks2.service)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.UDisks2.service
    DESTINATION share/dbus-1/system-services
)
install(FILES org.freedesktop.UDisks2.conf
    DESTINATION share/dbus-1/system.d
)

if(INSTALL_POLKIT_RULES)
    install(FILES org.freedesktop.udisks2.policy
        DESTINATION share/polkit-1/actions
    )
endif()
