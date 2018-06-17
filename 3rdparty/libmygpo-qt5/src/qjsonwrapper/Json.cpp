/* Copyright 2014, Uwe L. Korn <uwelk@xhochy.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Json.h"

// Qt version specific includes
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    #include <QJsonDocument>
    #include <QMetaProperty>
#else
    #include <qjson/parser.h>
    #include <qjson/qobjecthelper.h>
    #include <qjson/serializer.h>
#endif

namespace QJsonWrapper
{

QVariantMap
qobject2qvariant( const QObject* object )
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    QVariantMap map;
    if ( object == NULL )
    {
        return map;
    }

    const QMetaObject* metaObject = object->metaObject();
    for ( int i = 0; i < metaObject->propertyCount(); ++i )
    {
        QMetaProperty metaproperty = metaObject->property( i );
        if ( metaproperty.isReadable() )
        {
            map[ QLatin1String( metaproperty.name() ) ] = object->property( metaproperty.name() );
        }
    }
    return map;
#else
    return QJson::QObjectHelper::qobject2qvariant( object );
#endif
}


void
qvariant2qobject( const QVariantMap& variant, QObject* object )
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    for ( QVariantMap::const_iterator iter = variant.begin(); iter != variant.end(); ++iter )
    {
        QVariant property = object->property( iter.key().toLatin1() );
        Q_ASSERT( property.isValid() );
        if ( property.isValid() )
        {
            QVariant value = iter.value();
            if ( value.canConvert( property.type() ) )
            {
                value.convert( property.type() );
                object->setProperty( iter.key().toLatin1(), value );
            } else if ( QString( QLatin1String("QVariant") ).compare( QLatin1String( property.typeName() ) ) == 0 ) {
                object->setProperty( iter.key().toLatin1(), value );
            }
        }
    }
#else
    QJson::QObjectHelper::qvariant2qobject( variant, object );
#endif
}


QVariant
parseJson( const QByteArray& jsonData, bool* ok )
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson( jsonData, &error );
    if ( ok != NULL )
    {
        *ok = ( error.error == QJsonParseError::NoError );
    }
    return doc.toVariant();
#else
    QJson::Parser p;
    return p.parse( jsonData, ok );
#endif
}


QByteArray
toJson( const QVariant &variant, bool* ok )
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    QJsonDocument doc = QJsonDocument::fromVariant( variant );
    if ( ok != NULL )
    {
        *ok = !doc.isNull();
    }
    return doc.toJson( QJsonDocument::Compact );
#else
    QJson::Serializer serializer;
    return serializer.serialize( variant, ok );
#endif
}

}
