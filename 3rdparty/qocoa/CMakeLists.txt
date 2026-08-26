set(SOURCES)

set(HEADERS
    qsearchfield.h
    qbutton.h
    qprogressindicatorspinning.h
)

qt6_wrap_cpp(MOC_SOURCES ${HEADERS})

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
    qt6_add_resources(RESOURCES_SOURCES ${RESOURCES})
endif()

add_library(Qocoa STATIC
    ${SOURCES} ${MOC_SOURCES} ${RESOURCES_SOURCES}
)

# Qt6 has no MacExtras module; nothing in this library's .mm files actually
# used its API (it was just linked, unused).
target_link_libraries(Qocoa Qt6::Widgets)
