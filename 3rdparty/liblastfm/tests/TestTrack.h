/*
   This software is in the public domain, furnished "as is", without technical 
   support, and with no warranty, express or implied, as to its usefulness for 
   any purpose.
*/

#ifndef LASTFM_TESTTRACK_H
#define LASTFM_TESTTRACK_H

#include "Track.h"

#include <QtTest>

using lastfm::Track;

class TestTrack : public QObject
{
    Q_OBJECT
    
    Track example()
    {
        lastfm::MutableTrack t;
        t.setTitle( "Test Title" );
        t.setArtist( "Test Artist" );
        t.setAlbum( "Test Album" );
        return t;
    }
    
private slots:
    void testClone()
    {
        Track original = example();
        Track copy = original;
        
        #define TEST( x ) QVERIFY( original.x == copy.x )
        TEST( title() );
        TEST( artist() );
        TEST( album() );
        #undef TEST
    }
};

#endif
