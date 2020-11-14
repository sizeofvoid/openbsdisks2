cmake_minimum_required(VERSION 3.0)
project(bsdisks)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)
find_package(Qt5DBus)

option(INSTALL_POLKIT_RULES "Install polkit rules" OFF)
option(INSTALL_MAN_PAGES "Install man pages" ON)

add_definitions(-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")

add_executable(bsdisks
    main.cpp
    adaptors.cpp
    block.cpp
    blockfilesystem.cpp
    blockpartition.cpp
    blockparttable.cpp
    drive.cpp
    geomprober.cpp
    filesystemprober.cpp
    camcontrolprober.cpp
    zfsprober.cpp
    manageradaptor.cpp
    objectmanager.cpp
    devdthread.cpp

    camcontrol.c
)

target_link_libraries(bsdisks Qt5::DBus Qt5::Core)
target_link_libraries(bsdisks cam geom)

install(TARGETS bsdisks
    RUNTIME DESTINATION bin
)

configure_file(org.freedesktop.UDisks2.service.in org.freedesktop.UDisks2.service)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.UDisks2.service
    DESTINATION share/dbus-1/system-services
)
install(FILES org.freedesktop.UDisks2.conf
    DESTINATION etc/dbus-1/system.d
)

install(FILES bsdisks.conf.sample
    DESTINATION etc/
)

install(FILES bsdisks-syslog.conf.sample
    DESTINATION etc/syslog.d/
)
install(FILES bsdisks-newsyslog.conf.sample
    DESTINATION etc/newsyslog.conf.d/
)

install(FILES bsdisks.8
    DESTINATION man/man8/
)
install(FILES bsdisks.conf.5
    DESTINATION man/man5/
)

if(INSTALL_POLKIT_RULES)
    install(FILES org.freedesktop.udisks2.policy
        DESTINATION share/polkit-1/actions
    )
endif()
