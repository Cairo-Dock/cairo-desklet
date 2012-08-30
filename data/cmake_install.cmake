# Install script for directory: /home/fab/CD2/cairo-desklet/data

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Debug")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/share/cairo-desklet/rotate-desklet.svg;/usr/share/cairo-desklet/no-input-desklet.png;/usr/share/cairo-desklet/depth-rotate-desklet.svg;/usr/share/cairo-desklet/cairo-dock-ok.svg;/usr/share/cairo-desklet/cairo-dock-cancel.svg;/usr/share/cairo-desklet/cairo-dock-logo.png;/usr/share/cairo-desklet/cairo-desklet.svg")
FILE(INSTALL DESTINATION "/usr/share/cairo-desklet" TYPE FILE FILES
    "/home/fab/CD2/cairo-desklet/data/rotate-desklet.svg"
    "/home/fab/CD2/cairo-desklet/data/no-input-desklet.png"
    "/home/fab/CD2/cairo-desklet/data/depth-rotate-desklet.svg"
    "/home/fab/CD2/cairo-desklet/data/cairo-dock-ok.svg"
    "/home/fab/CD2/cairo-desklet/data/cairo-dock-cancel.svg"
    "/home/fab/CD2/cairo-desklet/data/cairo-dock-logo.png"
    "/home/fab/CD2/cairo-desklet/data/cairo-desklet.svg"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/share/cairo-desklet/cairo-desklet.conf")
FILE(INSTALL DESTINATION "/usr/share/cairo-desklet" TYPE FILE FILES "/home/fab/CD2/cairo-desklet/data/cairo-desklet.conf")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/share/pixmaps/cairo-desklet.svg")
FILE(INSTALL DESTINATION "/usr/share/pixmaps" TYPE FILE FILES "/home/fab/CD2/cairo-desklet/data/cairo-desklet.svg")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

