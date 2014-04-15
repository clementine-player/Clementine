include(CheckCXXCompilerFlag)

macro(UPDATE_CXX_COMPILER_FLAG target flag name)
    check_cxx_compiler_flag(${flag} COMPILER_SUPPORTS_${name}_FLAG)
    if(COMPILER_SUPPORTS_${name}_FLAG)
        add_definitions(${flag})
    endif()
    set(${name} TRUE)
endmacro()
