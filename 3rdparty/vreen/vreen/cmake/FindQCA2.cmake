#find qca2
include(CommonUtils)
find_module(qca2
    GLOBAL_HEADER qca.h
    HEADERS_DIR QtCrypto
    LIBRARY_HINTS ${QT_LIBRARY_DIR}
    INCLUDE_HINTS ${QT_INCLUDE_DIR}
)
