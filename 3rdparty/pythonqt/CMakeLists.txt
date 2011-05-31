include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${PYTHON_INCLUDE_DIRS})

set(SOURCES
  generated_cpp/com_trolltech_qt_core_builtin/com_trolltech_qt_core_builtin0.cpp
  generated_cpp/com_trolltech_qt_core_builtin/com_trolltech_qt_core_builtin_init.cpp
  generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core0.cpp
  generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core1.cpp
  generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core2.cpp
  generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core_init.cpp
  generated_cpp/com_trolltech_qt_gui_builtin/com_trolltech_qt_gui_builtin0.cpp
  generated_cpp/com_trolltech_qt_gui_builtin/com_trolltech_qt_gui_builtin_init.cpp
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui0.cpp
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui10.cpp
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui1.cpp
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui2.cpp
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui3.cpp
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui4.cpp
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui5.cpp
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui6.cpp
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui7.cpp
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui8.cpp
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui9.cpp
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui_init.cpp
  generated_cpp/com_trolltech_qt_network/com_trolltech_qt_network0.cpp
  generated_cpp/com_trolltech_qt_network/com_trolltech_qt_network1.cpp
  generated_cpp/com_trolltech_qt_network/com_trolltech_qt_network_init.cpp
  src/gui/PythonQtScriptingConsole.cpp
  src/PythonQtClassInfo.cpp
  src/PythonQtClassWrapper.cpp
  src/PythonQtConversion.cpp
  src/PythonQt.cpp
  src/PythonQtImporter.cpp
  src/PythonQtInstanceWrapper.cpp
  src/PythonQtMethodInfo.cpp
  src/PythonQtMisc.cpp
  src/PythonQtObjectPtr.cpp
  src/PythonQtQFileImporter.cpp
  src/PythonQtSignalReceiver.cpp
  src/PythonQtSlot.cpp
  src/PythonQtStdDecorators.cpp
  src/PythonQtStdOut.cpp
)

set(HEADERS
  generated_cpp/com_trolltech_qt_core_builtin/com_trolltech_qt_core_builtin0.h
  generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core0.h
  generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core1.h
  generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core2.h
  generated_cpp/com_trolltech_qt_gui_builtin/com_trolltech_qt_gui_builtin0.h
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui0.h
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui10.h
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui1.h
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui2.h
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui3.h
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui4.h
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui5.h
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui6.h
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui7.h
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui8.h
  generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui9.h
  generated_cpp/com_trolltech_qt_network/com_trolltech_qt_network0.h
  generated_cpp/com_trolltech_qt_network/com_trolltech_qt_network1.h
  src/gui/PythonQtScriptingConsole.h
  src/PythonQt.h
  src/PythonQtSignalReceiver.h
  src/PythonQtStdDecorators.h
)

qt4_wrap_cpp(MOC ${HEADERS})

if(STATIC_PYTHONQT)
  set(TYPE STATIC)
else(STATIC_PYTHONQT)
  set(TYPE SHARED)
endif(STATIC_PYTHONQT)

add_definitions(-DPYTHONQT_EXPORTS)

add_library(pythonqt ${TYPE}
  ${SOURCES}
  ${MOC}
)
set_target_properties(pythonqt PROPERTIES COMPILE_FLAGS "-Wno-write-strings -Wno-format-nonliteral -Wno-format-security")

target_link_libraries(pythonqt
  ${PYTHON_LIBRARIES}
  ${QT_LIBRARIES}
)
