include(ExternalProject)

set(OCIO_ARGS
	-DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
	-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
	-DCMAKE_INSTALL_LIBDIR=lib
	-DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
	-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
	-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
	-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
	-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
	-DOCIO_BUILD_APPS=OFF
	-DOCIO_BUILD_NUKE=OFF
	-DOCIO_BUILD_DOCS=OFF
	-DOCIO_BUILD_TESTS=OFF
	-DOCIO_BUILD_PYGLUE=OFF
	-DOCIO_STATIC_JNIGLUE=OFF
	-DYAML_CPP_OBJECT_LIB_EMBEDDED=ON
	-DTINYXML_OBJECT_LIB_EMBEDDED=ON)
if(OCIO_SHARED_LIBS)
	set(OCIO_ARGS ${OCIO_ARGS} -DOCIO_BUILD_SHARED=ON -DOCIO_BUILD_STATIC=OFF)
else()
	set(OCIO_ARGS ${OCIO_ARGS} -DOCIO_BUILD_SHARED=OFF -DOCIO_BUILD_STATIC=ON)
endif()
cmake_host_system_information(RESULT HAS_SSE2 QUERY HAS_SSE2)
set(OCIO_ARGS ${OCIO_ARGS} -DOCIO_USE_SSE=${HAS_SSE2})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/OCIO/src/OCIOThirdParty-build/ext)
ExternalProject_Add(
    OCIOThirdParty
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OCIO
    URL "http://github.com/imageworks/OpenColorIO/archive/v1.1.1.tar.gz"
	PATCH_COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_SOURCE_DIR}/third-party/ocio-patch/src/core/CMakeLists.txt
		${CMAKE_CURRENT_BINARY_DIR}/OCIO/src/OCIOThirdParty/src/core/CMakeLists.txt
    CMAKE_ARGS ${OCIO_ARGS})

set(OCIO_FOUND TRUE)
set(OCIO_DEFINES)
if(NOT OCIO_SHARED_LIBS)
	set(OCIO_DEFINES OpenColorIO_STATIC)
endif()
set(OCIO_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
set(OCIO_LIBRARY ${CMAKE_INSTALL_PREFIX}/lib/static/${CMAKE_STATIC_LIBRARY_PREFIX}OpenColorIO${CMAKE_STATIC_LIBRARY_SUFFIX})
set(OCIO_LIBRARIES ${OCIO_LIBRARY})

if(OCIO_FOUND AND NOT TARGET OCIO::OCIO)
    add_library(OCIO::OCIO UNKNOWN IMPORTED)
    add_dependencies(OCIO::OCIO OCIOThirdParty)
    set_target_properties(OCIO::OCIO PROPERTIES
        IMPORTED_LOCATION "${OCIO_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OCIO_INCLUDE_DIRS}"
		INTERFACE_COMPILE_DEFINITIONS ${OCIO_DEFINES})
endif()
if(OCIO_FOUND AND NOT TARGET OCIO)
    add_library(OCIO INTERFACE)
    target_link_libraries(OCIO INTERFACE OCIO::OCIO)
endif()

