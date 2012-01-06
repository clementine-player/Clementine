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

#ifndef CHROMAPRINT_IMAGE_BUILDER_H_
#define CHROMAPRINT_IMAGE_BUILDER_H_

#include <vector>
#include "utils.h"
#include "image.h"
#include "feature_vector_consumer.h"

namespace Chromaprint
{
	
	/**
	 * Accepts feature vectors and builds a 2D image out of them.
	 */
	class ImageBuilder : public FeatureVectorConsumer
	{
	public:
		ImageBuilder(Image *image = 0);
		~ImageBuilder();

		void Reset(Image *image)
		{
			set_image(image);
		}

		void Consume(std::vector<double> &features);

		Image *image() const
		{
			return m_image;
		}

		void set_image(Image *image)
		{
			m_image = image;
		}

	private:
		CHROMAPRINT_DISABLE_COPY(ImageBuilder);
	
		Image *m_image;
	};

};

#endif
