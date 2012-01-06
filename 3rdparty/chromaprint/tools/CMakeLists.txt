include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${FFMPEG_LIBAVCODEC_INCLUDE_DIRS}
	${FFMPEG_LIBAVFORMAT_INCLUDE_DIRS}
	${FFMPEG_LIBAVUTIL_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${TAGLIB_INCLUDES}
)

if(BUILD_EXTRA_TOOLS)

	add_executable(resample resample.cpp)
	target_link_libraries(resample chromaprint_p
		${FFMPEG_LIBAVFORMAT_LIBRARIES}
		${FFMPEG_LIBAVCODEC_LIBRARIES}
		${FFMPEG_LIBAVUTIL_LIBRARIES})

	add_executable(decode decode.cpp)
	target_link_libraries(decode chromaprint_p
		${FFMPEG_LIBAVFORMAT_LIBRARIES}
		${FFMPEG_LIBAVCODEC_LIBRARIES}
		${FFMPEG_LIBAVUTIL_LIBRARIES})

	add_executable(chromagram chromagram.cpp)
	target_link_libraries(chromagram chromaprint_p
		${FFMPEG_LIBAVFORMAT_LIBRARIES}
		${FFMPEG_LIBAVCODEC_LIBRARIES}
		${FFMPEG_LIBAVUTIL_LIBRARIES}
		-lpng)

	add_executable(spectrogram spectrogram.cpp)
	target_link_libraries(spectrogram chromaprint_p
		${FFMPEG_LIBAVFORMAT_LIBRARIES}
		${FFMPEG_LIBAVCODEC_LIBRARIES}
		${FFMPEG_LIBAVUTIL_LIBRARIES}
		-lpng)

	add_executable(learn_filters learn_filters.cpp)
	target_link_libraries(learn_filters chromaprint_p
		${FFMPEG_LIBAVFORMAT_LIBRARIES}
		${FFMPEG_LIBAVCODEC_LIBRARIES}
		${FFMPEG_LIBAVUTIL_LIBRARIES}
		${Boost_SYSTEM_LIBRARY}
		${Boost_FILESYSTEM_LIBRARY})

	add_executable(fpeval fpeval.cpp)
	target_link_libraries(fpeval chromaprint_p
		${FFMPEG_LIBAVFORMAT_LIBRARIES}
		${FFMPEG_LIBAVCODEC_LIBRARIES}
		${FFMPEG_LIBAVUTIL_LIBRARIES}
		${Boost_SYSTEM_LIBRARY}
		${Boost_FILESYSTEM_LIBRARY})

endif(BUILD_EXTRA_TOOLS)

if(NOT MSVC)
add_executable(fpcollect fpcollect.cpp)
target_link_libraries(fpcollect chromaprint_p
	${FFMPEG_LIBAVFORMAT_LIBRARIES}
	${FFMPEG_LIBAVCODEC_LIBRARIES}
	${FFMPEG_LIBAVUTIL_LIBRARIES}
	${TAGLIB_LIBRARIES})
endif()
