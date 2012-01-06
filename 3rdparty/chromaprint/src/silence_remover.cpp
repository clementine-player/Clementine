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

#include <assert.h>
#include <algorithm>
#include "debug.h"
#include "silence_remover.h"

using namespace std;
using namespace Chromaprint;

SilenceRemover::SilenceRemover(AudioConsumer *consumer)
    : m_start(true),
	  m_consumer(consumer)
{
}

bool SilenceRemover::Reset(int sample_rate, int num_channels)
{
	if (num_channels != 1) {
		DEBUG() << "Chromaprint::SilenceRemover::Reset() -- Expecting mono audio signal.\n";
		return false;
	}
	m_start = true;
	return true;
}

void SilenceRemover::Consume(short *input, int length)
{
	if (m_start) {
		while (length) {
			if (*input != 0) {
				m_start = false;
				break;
			}
			input++;
			length--;
		}
	}
	if (length) {
		m_consumer->Consume(input, length);
	}
}

void SilenceRemover::Flush()
{
}

