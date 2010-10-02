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

#include "Util.h"

#include <QString>

Echonest::Analysis::AnalysisStatus Echonest::statusToEnum(const QString& status)
{
    if( status == QLatin1String("unknown") ) {
        return Echonest::Analysis::Unknown;
    } else if( status == QLatin1String("pending") ) {
        return Echonest::Analysis::Pending;
    } else if( status == QLatin1String("complete") ) {
        return Echonest::Analysis::Complete;
    } else if( status == QLatin1String("error" )) {
        return Echonest::Analysis::Error;
    }
    return Echonest::Analysis::Unknown;
}

QString Echonest::statusToString(Echonest::Analysis::AnalysisStatus status)
{
    switch( status )
    {
        case Echonest::Analysis::Unknown:
            return QLatin1String( "unknown" );
        case Echonest::Analysis::Pending:
            return QLatin1String( "pending" );
        case Echonest::Analysis::Complete:
            return QLatin1String( "complete" );
        case Echonest::Analysis::Error:
            return QLatin1String( "error" );
    }
    return QString();
}