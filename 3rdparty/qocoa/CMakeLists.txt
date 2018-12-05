set(SOURCES)

set(HEADERS
    qsearchfield.h
    qbutton.h
    qprogressindicatorspinning.h
)

qt5_wrap_cpp(MOC_SOURCES ${HEADERS})

if(APPLE)
    list(APPEND SOURCES
        qsearchfield_mac.mm
        qbutton_mac.mm
        qprogressindicatorspinning_mac.mm
    )
else()
    list(APPEND SOURCES
        qsearchfield_nonmac.cpp
        qbutton_nonmac.cpp
        qprogressindicatorspinning_nonmac.cpp
    )
    set(RESOURCES
        qprogressindicatorspinning_nonmac.qrc
    )
    qt5_add_resources(RESOURCES_SOURCES ${RESOURCES})
endif()

add_library(Qocoa STATIC
    ${SOURCES} ${MOC_SOURCES} ${RESOURCES_SOURCES}
)
target_link_libraries(Qocoa Qt5::Widgets Qt5::MacExtras)
