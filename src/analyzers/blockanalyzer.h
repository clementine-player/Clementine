// Maintainer: Max Howell <mac.howell@methylblue.com>, (C) 2003-5
// Copyright:  See COPYING file that comes with this distribution
//

#ifndef BLOCKANALYZER_H
#define BLOCKANALYZER_H

#include "analyzerbase.h"
#include <qcolor.h>

class QResizeEvent;
class QMouseEvent;
class QPalette;

/**
 * @author Max Howell
 */

class BlockAnalyzer : public Analyzer::Base {
  Q_OBJECT
 public:
  Q_INVOKABLE BlockAnalyzer(QWidget*);
  ~BlockAnalyzer();

  static const uint HEIGHT;
  static const uint WIDTH;
  static const uint MIN_ROWS;
  static const uint MIN_COLUMNS;
  static const uint MAX_COLUMNS;
  static const uint FADE_SIZE;

  static const char* kName;

 protected:
  virtual void transform(Scope&);
  virtual void analyze(QPainter& p, const Scope&, bool new_frame);
  virtual void resizeEvent(QResizeEvent*);
  virtual void paletteChange(const QPalette&);

  void drawBackground();
  void determineStep();

 private:
  QPixmap* bar() { return &m_barPixmap; }

  uint m_columns, m_rows;  // number of rows and columns of blocks
  uint m_y;                // y-offset from top of widget
  QPixmap m_barPixmap;
  QPixmap m_topBarPixmap;
  QPixmap m_background;
  QPixmap canvas_;
  Scope m_scope;               // so we don't create a vector every frame
  std::vector<float> m_store;  // current bar heights
  std::vector<float> m_yscale;

  // FIXME why can't I namespace these? c++ issue?
  std::vector<QPixmap> m_fade_bars;
  std::vector<uint> m_fade_pos;
  std::vector<int> m_fade_intensity;

  float m_step;  // rows to fall per frame
};

#endif
