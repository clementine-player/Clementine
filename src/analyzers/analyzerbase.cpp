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
#include "engines/enginebase.h"
#include <cmath>        //interpolate()
#include <QEvent>     //event()
#include <QPainter>
#include <QPaintEvent>
#include <QtDebug>


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


Analyzer::Base::Base(QWidget *parent)
        : QWidget(parent)
        , m_engine(NULL)
        , m_lastScope(200)
{
}

void Analyzer::Base::set_engine(Engine::Base* engine) {
  m_engine = engine;
  connect(m_engine, SIGNAL(SpectrumAvailable(const QVector<float>&)),
                    SLOT(SpectrumAvailable(const QVector<float>&)));
}

void Analyzer::Base::SpectrumAvailable(const QVector<float>& spectrum) {
  m_lastScope = spectrum;
  update();
}

void Analyzer::Base::paintEvent(QPaintEvent * e)
{
    EngineBase *engine = m_engine;

    QPainter p(this);
    p.fillRect(e->rect(), palette().color(QPalette::Window));

    switch( engine->state() )
    {
    case Engine::Playing:
    {
        analyze(p, m_lastScope);
        break;
    }
    case Engine::Paused:
        analyze(p, m_lastScope);
        break;

    default:
        demo(p);
    }
}

void Analyzer::Base::paused(QPainter&) //virtual
{}

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

        analyze( p, s );
    }
    else analyze( p, Scope( 32, 0 ) );

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
