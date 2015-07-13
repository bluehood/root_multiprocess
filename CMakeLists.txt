############################################################################
# CMakeLists.txt file for building ROOT core/multiproc package
############################################################################
ROOT_USE_PACKAGE(core)
ROOT_USE_PACKAGE(net)

set(headers TJob.h TMultiProcess.h TNote.h TPool.h TServer.h TLocalServer.h)

set(sources TJob.cxx TMultiProcess.cxx TNote.cxx TPool.cxx TServer.cxx TLocalServer.cxx)

ROOT_GENERATE_DICTIONARY(G__MultiProc ${headers} MODULE MultiProc LINKDEF LinkDef.h)

ROOT_OBJECT_LIBRARY(MultiProcObjs ${sources} G__MultiProc.cxx)
ROOT_LINKER_LIBRARY(MultiProc $<TARGET_OBJECTS:MultiProcObjs> DEPENDENCIES Core Net)
ROOT_INSTALL_HEADERS(${installoptions})
