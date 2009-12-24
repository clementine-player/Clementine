// Maintainer: Max Howell <max.howell@methylblue.com>, (C) 2004
// Copyright:  See COPYING file that comes with this distribution

#ifndef ANALYZERBASE_H
#define ANALYZERBASE_H


#ifdef __FreeBSD__
#include <sys/types.h>
#endif

#include "engine_fwd.h"
#include "fht.h"     //stack allocated and convenience
#include <QPixmap> //stack allocated and convenience
#include <QTimer>  //stack allocated
#include <QWidget> //baseclass
#include <vector>    //included for convenience

#include <QGLWidget>     //baseclass
#ifdef Q_WS_MACX
#include <OpenGL/gl.h>   //included for convenience
#include <OpenGL/glu.h>  //included for convenience
#else
#include <GL/gl.h>   //included for convenience
#include <GL/glu.h>  //included for convenience
#endif

class QEvent;
class QPaintEvent;
class QResizeEvent;


namespace Analyzer {

typedef std::vector<float> Scope;

class Base : public QWidget
{
public:
    uint timeout() const { return m_timeout; }

    void set_engine(EngineBase* engine) { m_engine = engine; }

protected:
    Base( QWidget*, uint timeout, uint scopeSize = 7 );
    ~Base() { delete m_fht; }

    void hideEvent(QHideEvent *);
    void showEvent(QShowEvent *);
    void paintEvent( QPaintEvent* );

    void polishEvent();

    int  resizeExponent( int );
    int  resizeForBands( int );
    virtual void init() {}
    virtual void transform( Scope& );
    virtual void analyze( QPainter& p, const Scope& ) = 0;
    virtual void paused(QPainter& p);
    virtual void demo(QPainter& p);

    void changeTimeout( uint newTimeout )
    {
        m_timer.setInterval( newTimeout );
        m_timeout = newTimeout;
    }

protected:
    QTimer m_timer;
    uint   m_timeout;
    FHT    *m_fht;
    EngineBase* m_engine;
    Scope m_lastScope;
};


void interpolate( const Scope&, Scope& );
void initSin( Scope&, const uint = 6000 );

} //END namespace Analyzer

using Analyzer::Scope;

#endif
