macro(separate_debug_symbols TARGET)
  if (HAVE_BREAKPAD)
    add_custom_command(
      TARGET ${TARGET} POST_BUILD
      COMMAND objcopy
              --only-keep-debug 
              "$<TARGET_FILE:${TARGET}>"
              "$<TARGET_FILE:${TARGET}>.dbg"
      COMMAND objcopy
              --strip-debug
              "$<TARGET_FILE:${TARGET}>"
      COMMAND objcopy
              --add-gnu-debuglink="$<TARGET_FILE:${TARGET}>".dbg
              "$<TARGET_FILE:${TARGET}>"
    )
  endif()
endmacro()
