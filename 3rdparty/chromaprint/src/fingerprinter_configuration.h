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

#ifndef CHROMAPRINT_FINGERPRINTER_CONFIGURATION_H_
#define CHROMAPRINT_FINGERPRINTER_CONFIGURATION_H_

#include "classifier.h"
#include "chromaprint.h"

namespace Chromaprint
{
	class FingerprinterConfiguration
	{
	public:	

		FingerprinterConfiguration()
			: m_num_classifiers(0), m_classifiers(0)
		{
		}

		int num_filter_coefficients() const
		{
			return m_num_filter_coefficients;
		}

		const double *filter_coefficients() const
		{
			return m_filter_coefficients;
		}

		void set_filter_coefficients(const double *filter_coefficients, int size)
		{
			m_filter_coefficients = filter_coefficients;
			m_num_filter_coefficients = size;
		}

		int num_classifiers() const
		{
			return m_num_classifiers;
		}

		const Classifier *classifiers() const
		{
			return m_classifiers;
		}

		void set_classifiers(const Classifier *classifiers, int size)
		{
			m_classifiers = classifiers;
			m_num_classifiers = size;
		}

		bool interpolate() const 
		{
			return m_interpolate;
		}

		void set_interpolate(bool value)
		{
			m_interpolate = value;
		}

	private:
		int m_num_classifiers;
		const Classifier *m_classifiers;
		int m_num_filter_coefficients;
		const double *m_filter_coefficients;
		bool m_interpolate;
	};

	// Used for http://oxygene.sk/lukas/2010/07/introducing-chromaprint/
	// Trained on a randomly selected test data
	class FingerprinterConfigurationTest1 : public FingerprinterConfiguration
	{
	public:
		FingerprinterConfigurationTest1();
	};

	// Trained on 60k pairs based on eMusic samples (mp3)
	class FingerprinterConfigurationTest2 : public FingerprinterConfiguration
	{
	public:
		FingerprinterConfigurationTest2();
	};

	// Trained on 60k pairs based on eMusic samples with interpolation enabled (mp3)
	class FingerprinterConfigurationTest3 : public FingerprinterConfiguration
	{
	public:
		FingerprinterConfigurationTest3();
	};

	inline FingerprinterConfiguration *CreateFingerprinterConfiguration(int algorithm)
	{
		switch (algorithm) {
		case CHROMAPRINT_ALGORITHM_TEST1:
			return new FingerprinterConfigurationTest1();
		case CHROMAPRINT_ALGORITHM_TEST2:
			return new FingerprinterConfigurationTest2();
		case CHROMAPRINT_ALGORITHM_TEST3:
			return new FingerprinterConfigurationTest3();
		}
		return 0;
	}

};

#endif

