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

#include "fingerprinter_configuration.h"
#include "utils.h"

using namespace std;
using namespace Chromaprint;

static const int kChromaFilterSize = 5;
static const double kChromaFilterCoefficients[] = { 0.25, 0.75, 1.0, 0.75, 0.25 };

static const Classifier kClassifiersTest1[16] = {
	Classifier(Filter(0, 0, 3, 15), Quantizer(2.10543, 2.45354, 2.69414)),
	Classifier(Filter(1, 0, 4, 14), Quantizer(-0.345922, 0.0463746, 0.446251)),
	Classifier(Filter(1, 4, 4, 11), Quantizer(-0.392132, 0.0291077, 0.443391)),
	Classifier(Filter(3, 0, 4, 14), Quantizer(-0.192851, 0.00583535, 0.204053)),
	Classifier(Filter(2, 8, 2, 4), Quantizer(-0.0771619, -0.00991999, 0.0575406)),
	Classifier(Filter(5, 6, 2, 15), Quantizer(-0.710437, -0.518954, -0.330402)),
	Classifier(Filter(1, 9, 2, 16), Quantizer(-0.353724, -0.0189719, 0.289768)),
	Classifier(Filter(3, 4, 2, 10), Quantizer(-0.128418, -0.0285697, 0.0591791)),
	Classifier(Filter(3, 9, 2, 16), Quantizer(-0.139052, -0.0228468, 0.0879723)),
	Classifier(Filter(2, 1, 3, 6), Quantizer(-0.133562, 0.00669205, 0.155012)),
	Classifier(Filter(3, 3, 6, 2), Quantizer(-0.0267, 0.00804829, 0.0459773)),
	Classifier(Filter(2, 8, 1, 10), Quantizer(-0.0972417, 0.0152227, 0.129003)),
	Classifier(Filter(3, 4, 4, 14), Quantizer(-0.141434, 0.00374515, 0.149935)),
	Classifier(Filter(5, 4, 2, 15), Quantizer(-0.64035, -0.466999, -0.285493)),
	Classifier(Filter(5, 9, 2, 3), Quantizer(-0.322792, -0.254258, -0.174278)),
	Classifier(Filter(2, 1, 8, 4), Quantizer(-0.0741375, -0.00590933, 0.0600357))
};

FingerprinterConfigurationTest1::FingerprinterConfigurationTest1()
{
	set_classifiers(kClassifiersTest1, 16);
	set_filter_coefficients(kChromaFilterCoefficients, kChromaFilterSize);
	set_interpolate(false);
}

static const Classifier kClassifiersTest2[16] = {
    Classifier(Filter(0, 4, 3, 15), Quantizer(1.98215, 2.35817, 2.63523)),
    Classifier(Filter(4, 4, 6, 15), Quantizer(-1.03809, -0.651211, -0.282167)),
    Classifier(Filter(1, 0, 4, 16), Quantizer(-0.298702, 0.119262, 0.558497)),
    Classifier(Filter(3, 8, 2, 12), Quantizer(-0.105439, 0.0153946, 0.135898)),
    Classifier(Filter(3, 4, 4, 8), Quantizer(-0.142891, 0.0258736, 0.200632)),
    Classifier(Filter(4, 0, 3, 5), Quantizer(-0.826319, -0.590612, -0.368214)),
    Classifier(Filter(1, 2, 2, 9), Quantizer(-0.557409, -0.233035, 0.0534525)),
    Classifier(Filter(2, 7, 3, 4), Quantizer(-0.0646826, 0.00620476, 0.0784847)),
    Classifier(Filter(2, 6, 2, 16), Quantizer(-0.192387, -0.029699, 0.215855)),
    Classifier(Filter(2, 1, 3, 2), Quantizer(-0.0397818, -0.00568076, 0.0292026)),
    Classifier(Filter(5, 10, 1, 15), Quantizer(-0.53823, -0.369934, -0.190235)),
    Classifier(Filter(3, 6, 2, 10), Quantizer(-0.124877, 0.0296483, 0.139239)),
    Classifier(Filter(2, 1, 1, 14), Quantizer(-0.101475, 0.0225617, 0.231971)),
    Classifier(Filter(3, 5, 6, 4), Quantizer(-0.0799915, -0.00729616, 0.063262)),
    Classifier(Filter(1, 9, 2, 12), Quantizer(-0.272556, 0.019424, 0.302559)),
    Classifier(Filter(3, 4, 2, 14), Quantizer(-0.164292, -0.0321188, 0.0846339)),
};

FingerprinterConfigurationTest2::FingerprinterConfigurationTest2()
{
	set_classifiers(kClassifiersTest2, 16);
	set_filter_coefficients(kChromaFilterCoefficients, kChromaFilterSize);
	set_interpolate(false);
}

static const Classifier kClassifiersTest3[16] = {
    Classifier(Filter(0, 4, 3, 15), Quantizer(1.98215, 2.35817, 2.63523)),
    Classifier(Filter(4, 4, 6, 15), Quantizer(-1.03809, -0.651211, -0.282167)),
    Classifier(Filter(1, 0, 4, 16), Quantizer(-0.298702, 0.119262, 0.558497)),
    Classifier(Filter(3, 8, 2, 12), Quantizer(-0.105439, 0.0153946, 0.135898)),
    Classifier(Filter(3, 4, 4, 8), Quantizer(-0.142891, 0.0258736, 0.200632)),
    Classifier(Filter(4, 0, 3, 5), Quantizer(-0.826319, -0.590612, -0.368214)),
    Classifier(Filter(1, 2, 2, 9), Quantizer(-0.557409, -0.233035, 0.0534525)),
    Classifier(Filter(2, 7, 3, 4), Quantizer(-0.0646826, 0.00620476, 0.0784847)),
    Classifier(Filter(2, 6, 2, 16), Quantizer(-0.192387, -0.029699, 0.215855)),
    Classifier(Filter(2, 1, 3, 2), Quantizer(-0.0397818, -0.00568076, 0.0292026)),
    Classifier(Filter(5, 10, 1, 15), Quantizer(-0.53823, -0.369934, -0.190235)),
    Classifier(Filter(3, 6, 2, 10), Quantizer(-0.124877, 0.0296483, 0.139239)),
    Classifier(Filter(2, 1, 1, 14), Quantizer(-0.101475, 0.0225617, 0.231971)),
    Classifier(Filter(3, 5, 6, 4), Quantizer(-0.0799915, -0.00729616, 0.063262)),
    Classifier(Filter(1, 9, 2, 12), Quantizer(-0.272556, 0.019424, 0.302559)),
    Classifier(Filter(3, 4, 2, 14), Quantizer(-0.164292, -0.0321188, 0.0846339)),
};

FingerprinterConfigurationTest3::FingerprinterConfigurationTest3()
{
	set_classifiers(kClassifiersTest3, 16);
	set_filter_coefficients(kChromaFilterCoefficients, kChromaFilterSize);
	set_interpolate(true);
}

