find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen.config
    ${CMAKE_CURRENT_BINARY_DIR}/doxygen.config @ONLY)
  add_custom_target(doc ${DOXYGEN_EXECUTABLE}
    ${CMAKE_CURRENT_BINARY_DIR}/doxygen.config
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif(DOXYGEN_FOUND)


include(CMakeParseArguments)

find_or_add_hunter_package(Boost)
find_or_add_hunter_package(Eigen PACKAGE Eigen3)
find_or_add_hunter_package(hdf5 PACKAGE HDF5)
find_or_add_hunter_package(GSL)
find_or_add_hunter_package(F2C)

if(dobenchmarks)
  find_or_add_hunter_package(GBenchmark)
endif()

if(dompi AND "$ENV{CRAYOS_VERSION}" STREQUAL "")
  find_package(MPI REQUIRED)

  if(NOT DEFINED BLA_VENDOR OR BLA_VENDOR STREQUAL "All" OR BLA_VENDOR MATCHES "Intel")
    find_package(MKL COMPONENTS ScaLAPACK)
    set(scalapack_FOUND ${MKL_ScaLAPACK_FOUND})
    set(SCALAPACK_LIBRARIES -Wl,--start-group ${MKL_LIBRARIES} -Wl,--end-group ${MPI_CXX_LIBRARIES})
  endif()

  if(NOT MKL_FOUND)
    find_package(scalapack REQUIRED)
    set(SCALAPACK_LIBRARIES scalapack)
  endif()

  if (scalapack_FOUND)
    message(STATUS "Using ScaLAPACK libraries: ${SCALAPACK_LIBRARIES}")
  endif()
elseif(NOT "$ENV{CRAYOS_VERSION}" STREQUAL "")
  unset(SCALAPACK_LIBRARIES)
endif()
if(dompi AND NOT MPIEXEX_MAX_NUMPROCS)
  set(MPIEXEC_MAX_NUMPROCS 6)
endif()
# GMRes and other solvers
if(dompi)
  find_package(Belos)
  set(OPTIMET_BELOS ${Belos_FOUND})
  if(NOT Belos_FOUND)
    message(STATUS "Compiling without Belos solvers")
  endif()
endif()
