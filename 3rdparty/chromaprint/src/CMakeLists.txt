include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(chromaprint_SRCS
	chromaprint.cpp
	audio_processor.cpp
	chroma.cpp
	chroma_resampler.cpp
	chroma_filter.cpp
	integral_image.cpp
	spectrum.cpp
	spectral_centroid.cpp
	filter.cpp
	fft.cpp
	fingerprinter.cpp
	image_builder.cpp
	lloyds.cpp
	silence_remover.cpp
	fingerprint_calculator.cpp
	fingerprint_compressor.cpp
	fingerprint_decompressor.cpp
	fingerprinter_configuration.cpp
	base64.cpp
	avresample/resample2.c
)

if(WITH_AVFFT)
	set(chromaprint_SRCS fft_lib_avfft.cpp ${chromaprint_SRCS})
	set(chromaprint_LINK_LIBS
		${FFMPEG_LIBAVCODEC_LIBRARIES}
		${FFMPEG_LIBAVUTIL_LIBRARIES}
	)
	include_directories(
		${FFMPEG_LIBAVCODEC_INCLUDE_DIRS}
		${FFMPEG_LIBAVFORMAT_INCLUDE_DIRS}
		${FFMPEG_LIBAVUTIL_INCLUDE_DIRS}
	)
endif(WITH_AVFFT)

if(WITH_FFTW3)
	set(chromaprint_SRCS fft_lib_fftw3.cpp ${chromaprint_SRCS})
	set(chromaprint_LINK_LIBS ${FFTW3_LIBRARIES})
	include_directories(${FFTW3_INCLUDE_DIR})
endif(WITH_FFTW3)

if(WITH_VDSP)
	set(chromaprint_SRCS fft_lib_vdsp.cpp ${chromaprint_SRCS})
	set(chromaprint_LINK_LIBS ${ACCELERATE_LIBRARIES})
endif()

add_library(chromaprint_p STATIC ${chromaprint_SRCS})
target_link_libraries(chromaprint_p ${chromaprint_LINK_LIBS})
