cmake_minimum_required(VERSION 2.6)

# Source files
set(CHARDET-SOURCES
    CharDistribution.cpp
    JpCntx.cpp
    LangBulgarianModel.cpp
    LangCyrillicModel.cpp
    LangGreekModel.cpp
    LangHebrewModel.cpp
    LangHungarianModel.cpp
    LangThaiModel.cpp
    nsBig5Prober.cpp
    nsCharSetProber.cpp
    nsEscCharsetProber.cpp
    nsEscSM.cpp
    nsEUCJPProber.cpp
    nsEUCKRProber.cpp
    nsEUCTWProber.cpp
    nsGB2312Prober.cpp
    nsHebrewProber.cpp
    nsLatin1Prober.cpp
    nsMBCSGroupProber.cpp
    nsMBCSSM.cpp
    nsSBCharSetProber.cpp
    nsSBCSGroupProber.cpp
    nsSJISProber.cpp
    nsUniversalDetector.cpp
    nsUTF8Prober.cpp
)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_library(chardet STATIC
   ${CHARDET-SOURCES}
)
