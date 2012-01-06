include_directories(
	${GTEST_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/../src
)

set(tests_SOURCES
	main.cpp
	test_api.cpp
	test_combined_buffer.cpp
	test_utils.cpp
	test_quantizer.cpp
	test_filter_utils.cpp
	test_integral_image.cpp
	test_lloyds.cpp
	test_audio_processor.cpp
	test_bit_string_reader.cpp
	test_bit_string_writer.cpp
	test_chromaprint.cpp
	test_chroma.cpp
	test_chroma_filter.cpp
	test_chroma_resampler.cpp
	test_fingerprint_compressor.cpp
	test_fingerprint_decompressor.cpp
	test_fingerprint_calculator.cpp
	test_filter.cpp
	test_silence_remover.cpp
	test_base64.cpp
)

add_executable(all_tests ${tests_SOURCES})
target_link_libraries(all_tests
	${GTEST_BOTH_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	chromaprint_p
)

add_custom_target(check
	./all_tests $ENV{GTEST_FLAGS}
	DEPENDS all_tests
)

