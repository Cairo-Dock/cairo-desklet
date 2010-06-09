
SET(cairo_desklet_SRCS
	cairo-desklet-menu.c 					cairo-desklet-menu.h
	cairo-desklet-gui-simple.c					cairo-desklet-gui-simple.h
	cairo-desklet-user-interaction.c			cairo-desklet-user-interaction.h
	cairo-desklet.c
)

########### compilation ###############

# Make sure the compiler can find include files from the libraries.
include_directories(
	${PACKAGE_INCLUDE_DIRS})

# Make sure the linker can find the libraries.
link_directories(
	${PACKAGE_LIBRARY_DIRS})

# Add executable that is built from the source files.
add_executable("cairo-desklet"
	${cairo_desklet_SRCS})

# Link the executable to the librairies.
target_link_libraries("cairo-desklet"
	${PACKAGE_LIBRARIES})

# install the program once it is built.
install(
	TARGETS "cairo-desklet"
	DESTINATION bin)
