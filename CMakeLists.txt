########### requirements ###############

cmake_minimum_required (VERSION 2.6)
find_package (PkgConfig)
include (CheckLibraryExists)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckSymbolExists)

########### project ###############

project ("cairo-desklet")
set (VERSION "2.2.0-0beta0")
set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${VERSION}")
set (CPACK_SOURCE_IGNORE_FILES "/build/;/.bzr/;/.deps/;Makefile$;Makefile.in;.gmo$;/.libs/;.lo$;.la$;POTFILES;stamp-it;aclocal.m4;config.guess$;/config.h$;/config.h.in$;/config.log$;/config.status$;/config.sub$;/configure$;/depcomp$;/install-sh$;/libtool$;/ltmain.sh$;/missing$;/stamp-h1$;/compile;/CMakeFiles/;/_CPack_Packages/;/autom4te.cache/;/install_manifest.txt$;.tar.gz$;/CMakeCache.txt$;/cairo-dock$;/scripts/;/doc/;/data/themes.conf$;/data/cairo-dock.conf$;/data/main-dock.conf$;/data/cairo-dock-simple.conf$;/cairo-dock.pc$;.cmake$;~$;${CPACK_SOURCE_IGNORE_FILES}")
include (CPack)  # a inclure apres les set.
add_custom_target(dist
    COMMAND bzr export ${CMAKE_PROJECT_NAME}-${VERSION}.tar.gz
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
#add_custom_target( dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)  # pour pouvoir l'appeler avec "make dist"
add_definitions (-std=c99 -Wstrict-prototypes -Wextra -Wwrite-strings -Wuninitialized -Werror-implicit-function-declaration) #-Wunreachable-code -Wno-unused-parameter -Wall 
if (NOT ${CMAKE_BUILD_TYPE})
	add_definitions (-O3)
endif()
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "cmake_uninstall.cmake")


########### global variables ###############

set (PACKAGE ${CMAKE_PROJECT_NAME})
set (GETTEXT_PACKAGE ${PACKAGE})
set (prefix ${CMAKE_INSTALL_PREFIX})
set (exec_prefix ${prefix})
set (datadir ${prefix}/share)
set (pkgdatadir ${datadir}/cairo-dock)
set (mandir ${datadir}/man)
set (libdir ${prefix}/lib)
set (includedir ${prefix}/include)
set (bindir ${prefix}/bin)

########### dependencies ###############

pkg_check_modules ("PACKAGE" REQUIRED "gtk+-2.0" "gthread-2.0" "cairo" "librsvg-2.0" "dbus-1" "dbus-glib-1" "libxml-2.0" "gtkglext-1.0" "xrender" "libcurl" "gldi")

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
set (CAIRO_DESKLET_ICON "cairo-dock.svg")
set (CAIRO_DESKLET_LOCALE_DIR ${datadir}/locale)
set (CAIRO_DESKLET_GETTEXT_PACKAGE ${GETTEXT_PACKAGE})
set (CAIRO_DESKLET_DATA_DIR "cairo-desklet")

########### file generation ###############

configure_file (config.h.cmake.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
configure_file (data/cairo-desklet.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/data/cairo-desklet.conf)
#configure_file ("cmake_uninstall.cmake.in" "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
