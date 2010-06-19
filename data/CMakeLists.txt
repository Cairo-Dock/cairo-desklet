
########### install files ###############

install (FILES
	rotate-desklet.svg 
	no-input-desklet.png 	depth-rotate-desklet.svg
	cairo-dock-ok.svg 	cairo-dock-cancel.svg
	cairo-dock-logo.png 	cairo-desklet.svg
	cairo-desklet.conf
	DESTINATION ${pkgdatadir})

install (FILES
	cairo-desklet.svg
	DESTINATION ${datadir}/pixmaps)
