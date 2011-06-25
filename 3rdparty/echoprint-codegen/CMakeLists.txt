set(ECHOPRINT-SOURCES
    src/AudioBufferInput.cxx
    src/AudioStreamInput.cxx
    src/Base64.cxx
    src/Codegen.cxx
    src/Fingerprint.cxx
    src/MatrixUtility.cxx
    src/Metadata.cxx
    src/SubbandAnalysis.cxx
    src/Whitening.cxx
)

include_directories(${Boost_INCLUDE_DIRS})
add_library(echoprint STATIC ${ECHOPRINT-SOURCES})

set_target_properties(echoprint PROPERTIES
    COMPILE_DEFINITIONS "BOOST_UBLAS_NDEBUG;NDEBUG"
    COMPILE_FLAGS "-O3 -fPIC")
