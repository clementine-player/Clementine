/* This file is part of Clementine.
   Copyright 2004, Max Howell <max.howell@methylblue.com>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Mark Furneaux <mark@romaco.ca>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Original Author:  Max Howell  <max.howell@methylblue.com>  2004
*/

#ifndef ANALYZERS_ANALYZERBASE_H_
#define ANALYZERS_ANALYZERBASE_H_

#ifdef __FreeBSD__
#include <sys/types.h>
#endif

#include "fht.h"
#include "engines/engine_fwd.h"
#include <QPixmap>
#include <QBasicTimer>
#include <QWidget>
#include <vector>

#include <QGLWidget>
#ifdef Q_WS_MACX
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
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

  virtual void framerateChanged() {}

 protected:
  explicit Base(QWidget*, uint scopeSize = 7);

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
  int current_chunk_;

  bool new_frame_;
  bool is_playing_;
};

void interpolate(const Scope&, Scope&);
void initSin(Scope&, const uint = 6000);

}  // END namespace Analyzer

#endif  // ANALYZERS_ANALYZERBASE_H_
