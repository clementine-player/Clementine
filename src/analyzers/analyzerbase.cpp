/***************************************************************************
                          viswidget.cpp  -  description
                             -------------------
    begin                : Die Jan 7 2003
    copyright            : (C) 2003 by Max Howell
    email                : markey@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "analyzerbase.h"

#include <cmath>        //interpolate()

#include <QEvent>     //event()
#include <QPainter>
#include <QPaintEvent>
#include <QtDebug>

#include "engines/enginebase.h"

// INSTRUCTIONS Base2D
// 1. do anything that depends on height() in init(), Base2D will call it before you are shown
// 2. otherwise you can use the constructor to initialise things
// 3. reimplement analyze(), and paint to canvas(), Base2D will update the widget when you return control to it
// 4. if you want to manipulate the scope, reimplement transform()
// 5. for convenience <vector> <qpixmap.h> <qwdiget.h> are pre-included
// TODO make an INSTRUCTIONS file
//can't mod scope in analyze you have to use transform


//TODO for 2D use setErasePixmap Qt function insetead of m_background

// make the linker happy only for gcc < 4.0
#if !( __GNUC__ > 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ >= 0 ) ) && !defined(Q_OS_WIN32)
template class Analyzer::Base<QWidget>;
#endif


Analyzer::Base::Base( QWidget *parent, uint scopeSize )
        : QWidget( parent )
        , m_timeout( 40 ) // msec
        , m_fht( new FHT(scopeSize) )
        , m_engine(nullptr)
        , m_lastScope(512)
        , new_frame_(false)
        , is_playing_(false)
{
}

void Analyzer::Base::hideEvent(QHideEvent *) {
  m_timer.stop();
}

void Analyzer::Base::showEvent(QShowEvent *) {
  m_timer.start(timeout(), this);
}

void Analyzer::Base::transform( Scope &scope ) //virtual
{
    //this is a standard transformation that should give
    //an FFT scope that has bands for pretty analyzers

    //NOTE resizing here is redundant as FHT routines only calculate FHT::size() values
    //scope.resize( m_fht->size() );

    float *front = static_cast<float*>( &scope.front() );

    float* f = new float[ m_fht->size() ];
    m_fht->copy( &f[0], front );
    m_fht->logSpectrum( front, &f[0] );
    m_fht->scale( front, 1.0 / 20 );

    scope.resize( m_fht->size() / 2 ); //second half of values are rubbish
    delete [] f;
}

void Analyzer::Base::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    p.fillRect(e->rect(), palette().color(QPalette::Window));

    switch( m_engine->state() )
    {
    case Engine::Playing:
    {
        const Engine::Scope &thescope = m_engine->scope();
        int i = 0;

        // convert to mono here - our built in analyzers need mono, but we the engines provide interleaved pcm
        for( uint x = 0; (int)x < m_fht->size(); ++x ) 
        {
           m_lastScope[x] = double(thescope[i] + thescope[i+1]) / (2*(1<<15));
           i += 2;
        }

        is_playing_ = true;
        transform( m_lastScope );
        analyze( p, m_lastScope, new_frame_ );

        //scope.resize( m_fht->size() );

        break;
    }
    case Engine::Paused:
        is_playing_ = false;
        analyze(p, m_lastScope, new_frame_);
        break;

    default:
        is_playing_ = false;
        demo(p);
    }


    new_frame_ = false;
}

int Analyzer::Base::resizeExponent( int exp )
{
    if ( exp < 3 )
        exp = 3;
    else if ( exp > 9 )
        exp = 9;

    if ( exp != m_fht->sizeExp() ) {
        delete m_fht;
        m_fht = new FHT( exp );
    }
    return exp;
}

int Analyzer::Base::resizeForBands( int bands )
{
    int exp;
    if ( bands <= 8 )
        exp = 4;
    else if ( bands <= 16 )
        exp = 5;
    else if ( bands <= 32 )
        exp = 6;
    else if ( bands <= 64 )
        exp = 7;
    else if ( bands <= 128 )
        exp = 8;
    else
        exp = 9;

    resizeExponent( exp );
    return m_fht->size() / 2;
}

void Analyzer::Base::demo(QPainter& p) //virtual
{
    static int t = 201; //FIXME make static to namespace perhaps

    if( t > 999 ) t = 1; //0 = wasted calculations
    if( t < 201 )
    {
        Scope s( 32 );

        const double dt = double(t) / 200;
        for( uint i = 0; i < s.size(); ++i )
            s[i] = dt * (sin( M_PI + (i * M_PI) / s.size() ) + 1.0);

        analyze( p, s, new_frame_ );
    }
    else analyze( p, Scope( 32, 0 ), new_frame_ );

    ++t;
}


void Analyzer::Base::polishEvent()
{
    init(); //virtual
}

void
Analyzer::interpolate( const Scope &inVec, Scope &outVec ) //static
{
    double pos = 0.0;
    const double step = (double)inVec.size() / outVec.size();

    for ( uint i = 0; i < outVec.size(); ++i, pos += step )
    {
        const double error = pos - std::floor( pos );
        const unsigned long offset = (unsigned long)pos;

        unsigned long indexLeft = offset + 0;

        if ( indexLeft >= inVec.size() )
            indexLeft = inVec.size() - 1;

        unsigned long indexRight = offset + 1;

        if ( indexRight >= inVec.size() )
            indexRight = inVec.size() - 1;

        outVec[i] = inVec[indexLeft ] * ( 1.0 - error ) +
                    inVec[indexRight] * error;
    }
}

void
Analyzer::initSin( Scope &v, const uint size ) //static
{
    double step = ( M_PI * 2 ) / size;
    double radian = 0;

    for ( uint i = 0; i < size; i++ )
    {
        v.push_back( sin( radian ) );
        radian += step;
    }
}

void Analyzer::Base::timerEvent(QTimerEvent* e) {
  QWidget::timerEvent(e);
  if (e->timerId() != m_timer.timerId())
    return;

  new_frame_ = true;
  update();
}
