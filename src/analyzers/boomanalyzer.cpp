// Author: Max Howell <max.howell@methylblue.com>, (C) 2004
// Copyright: See COPYING file that comes with this distribution

#include "amarok.h"
#include "boomanalyzer.h"
#include <cmath>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qslider.h>
#include <qspinbox.h>

BoomAnalyzer::BoomAnalyzer( QWidget *parent )
    : Analyzer::Base2D( parent, 10, 9 )
    , K_barHeight( 1.271 )//1.471
    , F_peakSpeed( 1.103 )//1.122
    , F( 1.0 )
    , bar_height( BAND_COUNT, 0 )
    , peak_height( BAND_COUNT, 0 )
    , peak_speed( BAND_COUNT, 0.01 )
    , barPixmap( COLUMN_WIDTH, 50 )
{
    QWidget *o, *box = new QWidget( this, 0, WType_TopLevel );
    QSpinBox *m;
    int v;

    (new QGridLayout( box, 2, 3 ))->setAutoAdd( true );

    v = int(K_barHeight*1000);
    new QLabel( "Bar fall-rate:", box );
    o = new QSlider( 100, 2000, 100, v, Qt::Horizontal, box );
   (m = new QSpinBox( 100, 2000, 1, box ))->setValue( v );
    connect( o, SIGNAL(valueChanged(int)), SLOT(changeK_barHeight( int )) );
    connect( o, SIGNAL(valueChanged(int)), m, SLOT(setValue( int )) );

    v = int(F_peakSpeed*1000);
    new QLabel( "Peak acceleration: ", box );
    o = new QSlider( 1000, 1300, 50, v, Qt::Horizontal, box );
   (m = new QSpinBox( 1000, 1300, 1, box ))->setValue( v );
    connect( o, SIGNAL(valueChanged(int)), SLOT(changeF_peakSpeed( int )) );
    connect( o, SIGNAL(valueChanged(int)), m, SLOT(setValue( int )) );

    //box->show();
}


void
BoomAnalyzer::changeK_barHeight( int newValue )
{
    K_barHeight = (double)newValue / 1000;
}

void
BoomAnalyzer::changeF_peakSpeed( int newValue )
{
    F_peakSpeed = (double)newValue / 1000;
}

void
BoomAnalyzer::init()
{
    const uint HEIGHT = height() - 2;
    const double h = 1.2 / HEIGHT;

    F = double(HEIGHT) / (log10( 256 ) * 1.1 /*<- max. amplitude*/);

    barPixmap.resize( COLUMN_WIDTH-2, HEIGHT );

    QPainter p( &barPixmap );
    for( uint y = 0; y < HEIGHT; ++y )
    {
        const double F = (double)y * h;

        p.setPen( QColor( 255 - int(229.0 * F), 255 - int(229.0 * F), 255 - int(191.0 * F) ) );
        p.drawLine( 0, y, COLUMN_WIDTH-2, y );
    }
}

void
BoomAnalyzer::transform( Scope &s )
{
    float *front = static_cast<float*>( &s.front() );

    m_fht->spectrum( front );
    m_fht->scale( front, 1.0 / 60 );

    Scope scope( 32, 0 );

    const uint xscale[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,19,24,29,36,43,52,63,76,91,108,129,153,182,216,255 };

    for( uint j, i = 0; i < 32; i++ )
        for( j = xscale[i]; j < xscale[i + 1]; j++ )
            if ( s[j] > scope[i] )
                scope[i] = s[j];

    s = scope;
}

void
BoomAnalyzer::analyze( const Scope &scope )
{
    eraseCanvas();

    QPainter p( canvas() );
    float h;
    const uint MAX_HEIGHT = height() - 1;

    for( uint i = 0, x = 0, y; i < BAND_COUNT; ++i, x += COLUMN_WIDTH+1 )
    {
        h = log10( scope[i]*256.0 ) * F;

        if( h > MAX_HEIGHT )
           h = MAX_HEIGHT;

        if( h > bar_height[i] )
        {
            bar_height[i] = h;

            if( h > peak_height[i] )
            {
                peak_height[i] = h;
                peak_speed[i]  = 0.01;
            }
            else goto peak_handling;
        }
        else
        {
            if( bar_height[i] > 0.0 )
            {
                bar_height[i] -= K_barHeight; //1.4
                if( bar_height[i] < 0.0 ) bar_height[i] = 0.0;
            }

        peak_handling:

            if( peak_height[i] > 0.0 )
            {
                peak_height[i] -= peak_speed[i];
                peak_speed[i]  *= F_peakSpeed; //1.12

                if( peak_height[i] < bar_height[i] ) peak_height[i] = bar_height[i];
                if( peak_height[i] < 0.0 ) peak_height[i] = 0.0;
            }
        }

        y = height() - uint(bar_height[i]);
        bitBlt( canvas(), x+1, y, &barPixmap, 0, y );
        p.setPen( Amarok::ColorScheme::Foreground );
        p.drawRect( x, y, COLUMN_WIDTH, height() - y );

        y = height() - uint(peak_height[i]);
        p.setPen( Amarok::ColorScheme::Text );
        p.drawLine( x, y, x+COLUMN_WIDTH-1, y );
    }
}

#include "boomanalyzer.moc"
