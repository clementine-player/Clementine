// Maintainer: Max Howell <max.howell@methylblue.com>, (C) 2004
// Copyright:  See COPYING file that comes with this distribution

#ifndef ANALYZERBASE_H
#define ANALYZERBASE_H

#ifdef __FreeBSD__
#include <sys/types.h>
#endif

#include "core/fht.h"  //stack allocated and convenience
#include "engines/engine_fwd.h"
#include <QPixmap>      //stack allocated and convenience
#include <QBasicTimer>  //stack allocated
#include <QWidget>      //baseclass
#include <vector>       //included for convenience

#include <QGLWidget>  //baseclass
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

class Base : public QWidget {
  Q_OBJECT

 public:
  ~Base() { delete m_fht; }

  uint timeout() const { return m_timeout; }

  void set_engine(EngineBase* engine) { m_engine = engine; }

  void changeTimeout(uint newTimeout) {
    m_timeout = newTimeout;
    if (m_timer.isActive()) {
      m_timer.stop();
      m_timer.start(m_timeout, this);
    }
  }

 protected:
  Base(QWidget*, uint scopeSize = 7);

  void hideEvent(QHideEvent*);
  void showEvent(QShowEvent*);
  void paintEvent(QPaintEvent*);
  void timerEvent(QTimerEvent*);

  void polishEvent();

  int resizeExponent(int);
  int resizeForBands(int);
  virtual void init() {}
  virtual void transform(Scope&);
  virtual void analyze(QPainter& p, const Scope&, bool new_frame) = 0;
  virtual void demo(QPainter& p);

 protected:
  QBasicTimer m_timer;
  uint m_timeout;
  FHT* m_fht;
  EngineBase* m_engine;
  Scope m_lastScope;
  int current_chunk;

  bool new_frame_;
  bool is_playing_;
};

void interpolate(const Scope&, Scope&);
void initSin(Scope&, const uint = 6000);

}  // END namespace Analyzer

using Analyzer::Scope;

#endif
