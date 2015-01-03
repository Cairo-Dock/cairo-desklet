########### requirements ###############

cmake_minimum_required (VERSION 2.6)
find_package (PkgConfig)
include (CheckLibraryExists)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckSymbolExists)

########### project ###############

project ("cairo-desklet")
set (VERSION "3.1.0")
add_definitions (-std=c99 -Wall -Wextra -Werror-implicit-function-declaration) #-Wstrict-prototypes -Wextra -Wwrite-strings -Wuninitialized #-Wunreachable-code -Wno-unused-parameter -Wall 
if (NOT DEFINED CMAKE_BUILD_TYPE)
	add_definitions (-O3)
endif()
add_custom_target (uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

############ sources tarball ############

set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${VERSION}")
set (CPACK_SOURCE_IGNORE_FILES 
	"/build/;/.bzr/;/.bzrignore$;/.git/;/.gitignore$;/config.h$;/misc/;~$;${CPACK_SOURCE_IGNORE_FILES}")
include (CPack)

add_custom_target( dist
	COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
add_custom_target(dist-bzr
	COMMAND bzr export ${CMAKE_PROJECT_NAME}-${VERSION}.tar.gz
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

########### global variables ###############

set (PACKAGE ${CMAKE_PROJECT_NAME})
set (GETTEXT_PACKAGE ${PACKAGE})
set (prefix ${CMAKE_INSTALL_PREFIX})
set (exec_prefix ${prefix})
set (datadir ${prefix}/share)
set (pkgdatadir ${datadir}/cairo-desklet)
set (mandir ${datadir}/man)

if( CMAKE_SIZEOF_VOID_P EQUAL 8 AND "${LIB_SUFFIX}" STREQUAL "" AND "${FORCE_NOT_LIB64}" STREQUAL "" )
	set (libdir ${prefix}/lib64)
elseif( CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT "${FORCE_NOT_LIB64}" STREQUAL "" )
	set (libdir ${prefix}/lib${LIB_SUFFIX})
else()
	set (libdir ${prefix}/lib${LIB_SUFFIX})
endif()

set (includedir ${prefix}/include)
set (bindir ${prefix}/bin)

########### dependencies ###############

pkg_check_modules ("PACKAGE" REQUIRED "gldi")

check_include_files ("math.h" HAVE_MATH_H)
check_symbol_exists (LC_MESSAGES "locale.h" HAVE_LC_MESSAGES)
CHECK_LIBRARY_EXISTS (m sin "" HAVE_LIBM)

########### next steps ###############

add_subdirectory (src)
add_subdirectory (data)
#add_subdirectory (po)

########### variables defined at compil time ###############

set (CAIRO_DESKLET_SHARE_DATA_DIR ${pkgdatadir})
set (CAIRO_DESKLET_LOGO "cairo-dock-logo.png")
set (CAIRO_DESKLET_ICON "cairo-desklet.svg")
set (CAIRO_DESKLET_LOCALE_DIR ${datadir}/locale)
set (CAIRO_DESKLET_GETTEXT_PACKAGE ${GETTEXT_PACKAGE})
set (CAIRO_DESKLET_DATA_DIR "cairo-desklet")

########### file generation ###############

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/data/cairo-desklet.conf.in ${CMAKE_CURRENT_BINARY_DIR}/data/cairo-desklet.conf)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY)
