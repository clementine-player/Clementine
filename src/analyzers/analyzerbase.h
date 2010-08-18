// Maintainer: Max Howell <max.howell@methylblue.com>, (C) 2004
// Copyright:  See COPYING file that comes with this distribution

#ifndef ANALYZERBASE_H
#define ANALYZERBASE_H


#ifdef __FreeBSD__
#include <sys/types.h>
#endif

#include "engines/engine_fwd.h"
#include <QPixmap> //stack allocated and convenience
#include <QBasicTimer>  //stack allocated
#include <QWidget> //baseclass

class QEvent;
class QPaintEvent;
class QResizeEvent;


namespace Analyzer {

typedef QVector<float> Scope;

class Base : public QWidget
{
  Q_OBJECT

public slots:
    void set_engine(Engine::Base* engine);
    void SpectrumAvailable(const QVector<float>& spectrum);

protected:
    Base(QWidget* parent);

    void paintEvent( QPaintEvent* );

    void polishEvent();

    virtual void init() {}
    virtual void analyze(QPainter& p, const Scope&) = 0;
    virtual void paused(QPainter& p);
    virtual void demo(QPainter& p);

protected:
    EngineBase* m_engine;
    Scope m_lastScope;
};

void interpolate( const Scope&, Scope& );
void initSin( Scope&, const uint = 6000 );

} //END namespace Analyzer

using Analyzer::Scope;

#endif
