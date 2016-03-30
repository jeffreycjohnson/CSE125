
# Locate Fmod
#
#   FMOD_FOUND       - True if Fmod found.
#   FMOD_INCLUDE_DIR - Where to find includes
#   FMOD_LIBRARIES   - List of libraries when using Fmod.
#

SET( FMOD_SEARCH_PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)

FIND_PATH( FMOD_INCLUDE_DIR
	NAMES fmod.hpp
	PATH_SUFFIXES include include/fmod
	PATHS ${FMOD_SEARCH_PATHS}
)

FIND_LIBRARY( FMOD_LIBRARY
	NAMES fmod
	PATH_SUFFIXES lib lib64 lib/x86_64-linux-gnu lib64/x86_64-linux-gnu
	PATHS ${FMOD_SEARCH_PATHS}
)

IF( FMOD_LIBRARY )
    SET( FMOD_LIBRARIES "${FMOD_LIBRARY}")
ENDIF()

IF(FMOD_INCLUDE_DIR AND FMOD_LIBRARIES)
	SET(FMOD_FOUND TRUE)
ELSE(FMOD_INCLUDE_DIR AND FMOD_LIBRARIES)
	message (STATUS "Could not find Fmod.")
	SET(FMOD_FOUND FALSE)
ENDIF(FMOD_INCLUDE_DIR AND FMOD_LIBRARIES)

INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( Fmod DEFAULT_MSG FMOD_LIBRARIES FMOD_INCLUDE_DIR )
MARK_AS_ADVANCED( FMOD_INCLUDE_DIR FMOD_LIBRARY )
