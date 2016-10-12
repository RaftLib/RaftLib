prefix=${CMAKE_INSTALL_PREFIX}
libdir=${CMAKE_INSTALL_PREFIX}/lib
bindir=${CMAKE_INSTALL_PREFIX}/bin
pkglibdir=
includedir=${CMAKE_INSTALL_PREFIX}/include

Name: ${CMAKE_PROJECT_NAME} 
URL: http://raftlib.io
Version: ${version}
Requires: ${_PKG_CONFIG_REQUIRES}
Conflicts: ${_PKG_CONFIG_CONFLICTS}
Libs: ${_PKG_CONFIG_LIBS}
Libs.private: ${_PKG_CONFIG_LIBS_PRIVATE}
Cflags: ${_PKG_CONFIG_CFLAGS}

${PKG_CONFIG_EXTRA}
