/*
 * Chromaprint -- Audio fingerprinting toolkit
 * Copyright (C) 2010  Lukas Lalinsky <lalinsky@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "fingerprinter.h"
#include "chroma.h"
#include "chroma_normalizer.h"
#include "chroma_filter.h"
#include "fft.h"
#include "audio_processor.h"
#include "image_builder.h"
#include "fingerprint_calculator.h"
#include "fingerprinter_configuration.h"
#include "classifier.h"
#include "utils.h"

using namespace std;
using namespace Chromaprint;

static const int SAMPLE_RATE = 11025;
static const int FRAME_SIZE = 4096;
static const int OVERLAP = FRAME_SIZE - FRAME_SIZE / 3;
static const int MIN_FREQ = 28;
static const int MAX_FREQ = 3520;

Fingerprinter::Fingerprinter(FingerprinterConfiguration *config)
	: m_image(12)
{
	if (!config) {
		config = new FingerprinterConfigurationTest1();
	}
	m_image_builder = new ImageBuilder(&m_image);
	m_chroma_normalizer = new ChromaNormalizer(m_image_builder);
	m_chroma_filter = new ChromaFilter(config->filter_coefficients(), config->num_filter_coefficients(), m_chroma_normalizer);
	m_chroma = new Chroma(MIN_FREQ, MAX_FREQ, FRAME_SIZE, SAMPLE_RATE, m_chroma_filter);
	//m_chroma->set_interpolate(true);
	m_fft = new FFT(FRAME_SIZE, OVERLAP, m_chroma);
	m_audio_processor = new AudioProcessor(SAMPLE_RATE, m_fft);
	m_fingerprint_calculator = new FingerprintCalculator(config->classifiers(), config->num_classifiers());
	m_config = config;
}

Fingerprinter::~Fingerprinter()
{
	delete m_fingerprint_calculator;
	delete m_audio_processor;
	delete m_fft;
	delete m_chroma;
	delete m_chroma_filter;
	delete m_chroma_normalizer;
	delete m_image_builder;
	delete m_config;
}

bool Fingerprinter::Start(int sample_rate, int num_channels)
{
	if (!m_audio_processor->Reset(sample_rate, num_channels)) {
		// FIXME save error message somewhere
		return false;
	}
	m_fft->Reset();
	m_chroma->Reset();
	m_chroma_filter->Reset();
	m_chroma_normalizer->Reset();
	m_image = Image(12); // XXX
	m_image_builder->Reset(&m_image);
	return true;
}

void Fingerprinter::Consume(short *samples, int length)
{
	assert(length >= 0);
	m_audio_processor->Consume(samples, length);
}

vector<int32_t> Fingerprinter::Finish()
{
	m_audio_processor->Flush();
	return m_fingerprint_calculator->Calculate(&m_image);
}

