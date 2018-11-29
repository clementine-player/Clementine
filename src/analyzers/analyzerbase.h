/* This file is part of Clementine.
   Copyright 2004, Max Howell <max.howell@methylblue.com>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014-2015, Mark Furneaux <mark@furneaux.ca>
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

#include "config.h"

#ifdef __FreeBSD__
#include <sys/types.h>
#endif

#include "fht.h"
#include "engines/engine_fwd.h"
#include "engines/enginebase.h"
#include <QPixmap>
#include <QBasicTimer>
#include <QWidget>
#include <vector>

#ifdef HAVE_OPENGL
#include <QGLWidget>
#ifdef Q_WS_MACX
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif

class QEvent;
class QPaintEvent;
class QResizeEvent;

namespace Analyzer {

typedef std::vector<float> Scope;

class Base : public QWidget {
  Q_OBJECT

 public:
  ~Base() { delete fht_; }

  uint timeout() const { return timeout_; }

  void set_engine(EngineBase* engine) { engine_ = engine; }

  void changeTimeout(uint newTimeout) {
    timeout_ = newTimeout;
    if (timer_.isActive()) {
      timer_.stop();
      timer_.start(timeout_, this);
    }
  }

  virtual void framerateChanged() {}
  virtual void psychedelicModeChanged(bool);

 protected:
  explicit Base(QWidget*, uint scopeSize = 7);

  void hideEvent(QHideEvent*);
  void showEvent(QShowEvent*);
  void paintEvent(QPaintEvent*);
  void timerEvent(QTimerEvent*);

  void polishEvent();

  int resizeExponent(int);
  int resizeForBands(int);
  int BandFrequency(int) const;
  void updateBandSize(const int);
  QColor getPsychedelicColor(const Scope&, const int, const int);
  virtual void init() {}
  virtual void transform(Scope&);
  virtual void analyze(QPainter& p, const Scope&, bool new_frame) = 0;
  virtual void demo(QPainter& p);

 protected:
  static const int kSampleRate =
      44100;  // we shouldn't need to care about ultrasonics

  QBasicTimer timer_;
  uint timeout_;
  FHT* fht_;
  EngineBase* engine_;
  Scope lastScope_;

  bool new_frame_;
  bool is_playing_;

  QVector<uint> barkband_table_;
  double prev_colors_[10][3];
  int prev_color_index_;
  int bands_;
  bool psychedelic_enabled_;
};

void interpolate(const Scope&, Scope&);
void initSin(Scope&, const uint = 6000);

}  // END namespace Analyzer

#endif  // ANALYZERS_ANALYZERBASE_H_
