# Locate Assimp
#
#   Assimp_FOUND       - True if Assimp found.
#   Assimp_INCLUDE_DIR - Where to find includes
#   Assimp_LIBRARIES   - List of libraries when using Assimp.
#

SET( Assimp_SEARCH_PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)

FIND_PATH( Assimp_INCLUDE_DIR
	NAMES assimp/cimport.h Assimp/cimport.h
	PATH_SUFFIXES include
	PATHS ${Assimp_SEARCH_PATHS}
)

FIND_LIBRARY( Assimp_LIBRARY
	NAMES Assimp assimp
	PATH_SUFFIXES lib lib64 lib/x86_64-linux-gnu lib64/x86_64-linux-gnu
	PATHS ${Assimp_SEARCH_PATHS}
)

IF( Assimp_LIBRARY )
    SET( Assimp_LIBRARIES "${Assimp_LIBRARY}")
ENDIF()

IF(Assimp_INCLUDE_DIR AND Assimp_LIBRARIES)
	SET(Assimp_FOUND TRUE)
ELSE(Assimp_INCLUDE_DIR AND Assimp_LIBRARIES)
	message (STATUS "Could not find Assimp.")
	SET(Assimp_FOUND FALSE)
ENDIF(Assimp_INCLUDE_DIR AND Assimp_LIBRARIES)

INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( Assimp DEFAULT_MSG Assimp_LIBRARIES Assimp_INCLUDE_DIR )
MARK_AS_ADVANCED( Assimp_INCLUDE_DIR Assimp_LIBRARY )
