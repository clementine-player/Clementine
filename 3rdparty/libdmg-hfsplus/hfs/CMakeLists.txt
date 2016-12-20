link_directories (${PROJECT_BINARY_DIR}/common)
add_library(hfs btree.c catalog.c extents.c fastunicodecompare.c flatfile.c hfslib.c rawfile.c utility.c volume.c)
target_link_libraries(hfs common)

add_executable(hfsplus hfs.c)
target_link_libraries (hfsplus hfs)

install(TARGETS hfsplus DESTINATION .)

