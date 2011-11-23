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

#include "Song.h"
#include <QSharedData>
#include <QString>

class DynamicPlaylistData : public QSharedData
{
public:
    DynamicPlaylistData() {}
    DynamicPlaylistData(const DynamicPlaylistData& other) : QSharedData(other)
    {
        sessionId = other.sessionId;
        currentSong = other.currentSong;
    }
    
    QByteArray sessionId;
    Echonest::Song currentSong;
};

