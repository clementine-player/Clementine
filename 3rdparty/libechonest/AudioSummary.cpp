/****************************************************************************************
 * Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>                                    *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "AudioSummary.h"

#include "AudioSummary_p.h"
#include "Config.h"
#include "Parsing_p.h"

#include <QNetworkReply>

Echonest::AudioSummary::AudioSummary()
    : d( new AudioSummaryData )
{

}

Echonest::AudioSummary::AudioSummary(const Echonest::AudioSummary& other)
    :d( other.d )
{

}

Echonest::AudioSummary::~AudioSummary()
{}

QDebug Echonest::operator<<(QDebug d, const Echonest::AudioSummary& summary)
{
//     d << summary
    return d.maybeSpace();
}

Echonest::AudioSummary& Echonest::AudioSummary::operator=(const Echonest::AudioSummary& audio)
{
    d = audio.d;
    return *this;
}

int Echonest::AudioSummary::analysisStatus() const
{
    return d->status;
}

void Echonest::AudioSummary::setAnalysisStatus(int status)
{
    d->status = status;
}

qreal Echonest::AudioSummary::analysisTime() const
{
    return d->analysis_time;
}

void Echonest::AudioSummary::setAnalysisTime(qreal time)
{
    d->analysis_time = time;
}

QString Echonest::AudioSummary::analyzerVersion() const
{
    return d->analyzer_version;
}

void Echonest::AudioSummary::setAnalyzerVersion(QString version)
{
    d->analyzer_version = version;
}

Echonest::BarList Echonest::AudioSummary::bars() const
{
    return d->bars;
}

void Echonest::AudioSummary::setBars(const Echonest::BarList& bars)
{
    d->bars = bars;
}

Echonest::BeatList Echonest::AudioSummary::beats() const
{
    return d->beats;
}

void Echonest::AudioSummary::setBeats(const Echonest::BeatList& beats)
{
    d->beats = beats;
}

QString Echonest::AudioSummary::detailedStatus() const
{
    return d->detailed_status;
}

void Echonest::AudioSummary::setDetailedStatus( const QString& status )
{
    d->detailed_status = status;
}

qreal Echonest::AudioSummary::duration() const
{
    return d->duration;
}

void Echonest::AudioSummary::setDuration(qreal duration)
{
    d->duration = duration;
}


qreal Echonest::AudioSummary::endOfFadeIn() const
{
    return d->end_of_fade_in;
}

void Echonest::AudioSummary::setEndOfFadeIn(qreal time)
{
    d->end_of_fade_in = time;
}

QNetworkReply* Echonest::AudioSummary::fetchFullAnalysis() const
{
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( QUrl( d->analysis_url ) ) );
}

int Echonest::AudioSummary::key() const
{
    return d->key;
}

void Echonest::AudioSummary::setKey(int key)
{
    d->key = key;
}

qreal Echonest::AudioSummary::keyConfidence() const
{
    return d->key_confidence;
}

void Echonest::AudioSummary::setKeyConfidence(qreal confidence)
{
    d->key_confidence = confidence;
}

qreal Echonest::AudioSummary::loudness() const
{
    return d->loudness;
}

void Echonest::AudioSummary::setLoudness(qreal loudness)
{
    d->loudness = loudness;
}

qreal Echonest::AudioSummary::modeConfidence() const
{
    return d->mode_confidence;
}

void Echonest::AudioSummary::setModeConfidence(qreal confidence)
{
    d->mode_confidence = confidence;
}

qint64 Echonest::AudioSummary::numSamples() const
{
    return d->num_samples;
}

void Echonest::AudioSummary::setNumSamples(qint64 num)
{
    d->num_samples = num;
}

void Echonest::AudioSummary::parseFullAnalysis( QNetworkReply* reply ) throw( Echonest::ParseError )
{
    Echonest::Parser::checkForErrors( reply );
    Echonest::Parser::parseDetailedAudioSummary( reply, *this );
    reply->deleteLater();
}

QString Echonest::AudioSummary::sampleMD5() const
{
    return d->sample_md5;
}

void Echonest::AudioSummary::setSampleMD5(const QString& md5)
{
    d->sample_md5 = md5;
}

qreal Echonest::AudioSummary::sampleRate() const
{
    return d->samplerate;
}

void Echonest::AudioSummary::setSampleRate(qreal sampleRate)
{
    d->samplerate = sampleRate;
}

Echonest::SectionList Echonest::AudioSummary::sections() const
{
    return d->sections;
}

void Echonest::AudioSummary::setSections(const Echonest::SectionList& sections)
{
    d->sections = sections;
}

Echonest::SegmentList Echonest::AudioSummary::segments() const
{
    return d->segments;
}

void Echonest::AudioSummary::setSegments(const Echonest::SegmentList& segments)
{
    d->segments = segments;
}

void Echonest::AudioSummary::setStartOfFadeOut(qreal time)
{
    d->start_of_fade_out = time;
}

Echonest::TatumList Echonest::AudioSummary::tatums() const
{
    return d->tatums;
}

void Echonest::AudioSummary::setTatums(const Echonest::TatumList& tatums)
{
    d->tatums = tatums;
}

qreal Echonest::AudioSummary::startOfFadeOut() const
{
    return d->start_of_fade_out;
}

qreal Echonest::AudioSummary::tempo() const
{
    return d->tempo;
}

void Echonest::AudioSummary::setTempo(qreal tempo)
{
    d->tempo = tempo;
}

qreal Echonest::AudioSummary::tempoConfidence() const
{
    return d->tempo_confidence;
}

void Echonest::AudioSummary::setTempoConfidence(qreal confidence)
{
    d->tempo_confidence = confidence;
}

int Echonest::AudioSummary::timeSignature() const
{
    return d->time_signature;
}

void Echonest::AudioSummary::setTimeSignature(int timeSignature)
{
    d->time_signature = timeSignature;
}

qreal Echonest::AudioSummary::timeSignatureConfidence() const
{
    return d->time_signature_confidence;
}

void Echonest::AudioSummary::setTimeSignatureConfidence(qreal confidence)
{
    d->time_signature_confidence = confidence;
}

void Echonest::AudioSummary::setTimestamp(qreal timestamp)
{
    d->timestamp = timestamp;
}

qreal Echonest::AudioSummary::timestamp() const
{
    return d->timestamp;
}

int Echonest::AudioSummary::mode() const
{
    return d->mode;
}

void Echonest::AudioSummary::setAnalysisUrl(const QUrl& analysisUrl)
{
    d->analysis_url = analysisUrl;
}

void Echonest::AudioSummary::setMode(int mode)
{
    d->mode = mode;
}

qreal Echonest::AudioSummary::danceability() const
{
    return d->danceability;
}

void Echonest::AudioSummary::setDanceability(qreal dance)
{
    d->danceability = dance;
}

qreal Echonest::AudioSummary::energy() const
{
    return d->energy;
}

void Echonest::AudioSummary::setEnergy(qreal energy)
{
    d->energy = energy;
}

