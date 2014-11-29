/* This file is part of Clementine.
   Copyright 2003-2005, Max Howell <max.howell@methylblue.com>
   Copyright 2005, Mark Kretschmann <markey@web.de>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
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

/* Original Author:  Max Howell  <max.howell@methylblue.com>  2003-2005
 * Original Author:  Mark Kretschmann  <markey@web.de>  2005
 */

#include "blockanalyzer.h"

#include <cmath>

#include <QMouseEvent>
#include <QResizeEvent>
#include <cstdlib>
#include <QPainter>

const uint BlockAnalyzer::HEIGHT = 2;
const uint BlockAnalyzer::WIDTH = 4;
const uint BlockAnalyzer::MIN_ROWS = 3;       // arbituary
const uint BlockAnalyzer::MIN_COLUMNS = 32;   // arbituary
const uint BlockAnalyzer::MAX_COLUMNS = 256;  // must be 2**n
const uint BlockAnalyzer::FADE_SIZE = 90;

const char* BlockAnalyzer::kName =
    QT_TRANSLATE_NOOP("AnalyzerContainer", "Block analyzer");

BlockAnalyzer::BlockAnalyzer(QWidget* parent)
    : Analyzer::Base(parent, 9),
      m_columns(0),
      m_rows(0),
      m_y(0),
      m_barPixmap(1, 1),
      m_topBarPixmap(WIDTH, HEIGHT),
      m_scope(MIN_COLUMNS),
      m_store(1 << 8, 0),
      m_fade_bars(FADE_SIZE),
      m_fade_pos(1 << 8, 50),
      m_fade_intensity(1 << 8, 32) {
  setMinimumSize(MIN_COLUMNS * (WIDTH + 1) - 1,
                 MIN_ROWS * (HEIGHT + 1) - 1);
  // -1 is padding, no drawing takes place there
  setMaximumWidth(MAX_COLUMNS * (WIDTH + 1) - 1);

  // mxcl says null pixmaps cause crashes, so let's play it safe
  for (uint i = 0; i < FADE_SIZE; ++i) m_fade_bars[i] = QPixmap(1, 1);
}

BlockAnalyzer::~BlockAnalyzer() {}

void BlockAnalyzer::resizeEvent(QResizeEvent* e) {
  QWidget::resizeEvent(e);

  m_background = QPixmap(size());
  canvas_ = QPixmap(size());

  const uint oldRows = m_rows;

  // all is explained in analyze()..
  // +1 to counter -1 in maxSizes, trust me we need this!
  m_columns = qMax(static_cast<uint>(static_cast<double>(width() + 1) / (WIDTH + 1)), MAX_COLUMNS);
  m_rows = static_cast<uint>(static_cast<double>(height() + 1) / (HEIGHT + 1));

  // this is the y-offset for drawing from the top of the widget
  m_y = (height() - (m_rows * (HEIGHT + 1)) + 2) / 2;

  m_scope.resize(m_columns);

  if (m_rows != oldRows) {
    m_barPixmap = QPixmap(WIDTH, m_rows * (HEIGHT + 1));

    for (uint i = 0; i < FADE_SIZE; ++i)
      m_fade_bars[i] = QPixmap(WIDTH, m_rows * (HEIGHT + 1));

    m_yscale.resize(m_rows + 1);

    const uint PRE = 1,
               PRO = 1;  // PRE and PRO allow us to restrict the range somewhat

    for (uint z = 0; z < m_rows; ++z)
      m_yscale[z] = 1 - (log10(PRE + z) / log10(PRE + m_rows + PRO));

    m_yscale[m_rows] = 0;

    determineStep();
    paletteChange(palette());
  }

  drawBackground();
}

void BlockAnalyzer::determineStep() {
  // falltime is dependent on rowcount due to our digital resolution (ie we have
  // boxes/blocks of pixels)
  // I calculated the value 30 based on some trial and error

  // the fall time of 30 is too slow on framerates above 50fps
  const double fallTime = timeout() < 20 ? 20 * m_rows : 30 * m_rows;

  m_step = static_cast<double>(m_rows * timeout()) / fallTime;
}

void BlockAnalyzer::framerateChanged() {  // virtual
  determineStep();
}

void BlockAnalyzer::transform(Analyzer::Scope& s) {
  for (uint x = 0; x < s.size(); ++x) s[x] *= 2;

  float* front = static_cast<float*>(&s.front());

  m_fht->spectrum(front);
  m_fht->scale(front, 1.0 / 20);

  // the second half is pretty dull, so only show it if the user has a large
  // analyzer
  // by setting to m_scope.size() if large we prevent interpolation of large
  // analyzers, this is good!
  s.resize(m_scope.size() <= MAX_COLUMNS / 2 ? MAX_COLUMNS / 2
                                             : m_scope.size());
}

void BlockAnalyzer::analyze(QPainter& p, const Analyzer::Scope& s,
                            bool new_frame) {
  // y = 2 3 2 1 0 2
  //     . . . . # .
  //     . . . # # .
  //     # . # # # #
  //     # # # # # #
  //
  // visual aid for how this analyzer works.
  // y represents the number of blanks
  // y starts from the top and increases in units of blocks

  // m_yscale looks similar to: { 0.7, 0.5, 0.25, 0.15, 0.1, 0 }
  // if it contains 6 elements there are 5 rows in the analyzer

  if (!new_frame) {
    p.drawPixmap(0, 0, canvas_);
    return;
  }

  QPainter canvas_painter(&canvas_);

  Analyzer::interpolate(s, m_scope);

  // Paint the background
  canvas_painter.drawPixmap(0, 0, m_background);

  for (uint y, x = 0; x < m_scope.size(); ++x) {
    // determine y
    for (y = 0; m_scope[x] < m_yscale[y]; ++y)
      continue;

    // this is opposite to what you'd think, higher than y
    // means the bar is lower than y (physically)
    if (static_cast<float>y > m_store[x])
      y = static_cast<int>(m_store[x] += m_step);
    else
      m_store[x] = y;

    // if y is lower than m_fade_pos, then the bar has exceeded the height of
    // the fadeout
    // if the fadeout is quite faded now, then display the new one
    if (y <= m_fade_pos[x] /*|| m_fade_intensity[x] < FADE_SIZE / 3*/) {
      m_fade_pos[x] = y;
      m_fade_intensity[x] = FADE_SIZE;
    }

    if (m_fade_intensity[x] > 0) {
      const uint offset = --m_fade_intensity[x];
      const uint y = m_y + (m_fade_pos[x] * (HEIGHT + 1));
      canvas_painter.drawPixmap(x * (WIDTH + 1), y, m_fade_bars[offset], 0, 0,
                                WIDTH, height() - y);
    }

    if (m_fade_intensity[x] == 0) m_fade_pos[x] = m_rows;

    // REMEMBER: y is a number from 0 to m_rows, 0 means all blocks are glowing,
    // m_rows means none are
    canvas_painter.drawPixmap(x * (WIDTH + 1), y * (HEIGHT + 1) + m_y, *bar(),
                              0, y * (HEIGHT + 1), bar()->width(),
                              bar()->height());
  }

  for (uint x = 0; x < m_store.size(); ++x)
    canvas_painter.drawPixmap(x * (WIDTH + 1),
                              static_cast<int>(m_store[x]) * (HEIGHT + 1) + m_y,
                              m_topBarPixmap);

  p.drawPixmap(0, 0, canvas_);
}

static inline void adjustToLimits(int& b, int& f, uint& amount) {
  // with a range of 0-255 and maximum adjustment of amount,
  // maximise the difference between f and b

  if (b < f) {
    if (b > 255 - f) {
      amount -= f;
      f = 0;
    } else {
      amount -= (255 - f);
      f = 255;
    }
  } else {
    if (f > 255 - b) {
      amount -= f;
      f = 0;
    } else {
      amount -= (255 - f);
      f = 255;
    }
  }
}

/**
 * Clever contrast function
 *
 * It will try to adjust the foreground color such that it contrasts well with
 *the background
 * It won't modify the hue of fg unless absolutely necessary
 * @return the adjusted form of fg
 */
QColor ensureContrast(const QColor& bg, const QColor& fg, uint _amount = 150) {
  class OutputOnExit {
   public:
    explicit OutputOnExit(const QColor& color) : c(color) {}
    ~OutputOnExit() {
      int h, s, v;
      c.getHsv(&h, &s, &v);
    }

   private:
    const QColor& c;
  };

  OutputOnExit allocateOnTheStack(fg);

  int bh, bs, bv;
  int fh, fs, fv;

  bg.getHsv(&bh, &bs, &bv);
  fg.getHsv(&fh, &fs, &fv);

  int dv = abs(bv - fv);

  // value is the best measure of contrast
  // if there is enough difference in value already, return fg unchanged
  if (dv > static_cast<int>(_amount)) return fg;

  int ds = abs(bs - fs);

  // saturation is good enough too. But not as good. TODO adapt this a little
  if (ds > static_cast<int>(_amount)) return fg;

  int dh = abs(bh - fh);

  if (dh > 120) {
    // a third of the colour wheel automatically guarentees contrast
    // but only if the values are high enough and saturations significant enough
    // to allow the colours to be visible and not be shades of grey or black

    // check the saturation for the two colours is sufficient that hue alone can
    // provide sufficient contrast
    if (ds > static_cast<int>(_amount) / 2 && (bs > 125 && fs > 125))
      return fg;
    else if (dv > static_cast<int>(_amount) / 2 && (bv > 125 && fv > 125))
      return fg;
  }

  if (fs < 50 && ds < 40) {
    // low saturation on a low saturation is sad
    const int tmp = 50 - fs;
    fs = 50;
    if (static_cast<int>(_amount) > tmp)
      _amount -= tmp;
    else
      _amount = 0;
  }

  // test that there is available value to honor our contrast requirement
  if (255 - dv < static_cast<int>(_amount)) {
    // we have to modify the value and saturation of fg
    // adjustToLimits( bv, fv, amount );
    // see if we need to adjust the saturation
    if (static_cast<int>(_amount) > 0) adjustToLimits(bs, fs, _amount);

    // see if we need to adjust the hue
    if (static_cast<int>(_amount) > 0) fh += static_cast<int>(_amount);  // cycles around;

    return QColor::fromHsv(fh, fs, fv);
  }

  if (fv > bv && bv > static_cast<int>(_amount))
    return QColor::fromHsv(fh, fs, bv - static_cast<int>(_amount));

  if (fv < bv && fv > static_cast<int>(_amount))
    return QColor::fromHsv(fh, fs, fv - static_cast<int>(_amount));

  if (fv > bv && (255 - fv > static_cast<int>(_amount)))
    return QColor::fromHsv(fh, fs, fv + static_cast<int>(_amount));

  if (fv < bv && (255 - bv > static_cast<int>(_amount)))
    return QColor::fromHsv(fh, fs, bv + static_cast<int>(_amount));

  return Qt::blue;
}

void BlockAnalyzer::paletteChange(const QPalette&) {
  const QColor bg = palette().color(QPalette::Background);
  const QColor fg = ensureContrast(bg, palette().color(QPalette::Highlight));

  m_topBarPixmap.fill(fg);

  const double dr = 15 * static_cast<double>(bg.red() - fg.red()) / (m_rows * 16);
  const double dg = 15 * static_cast<double>(bg.green() - fg.green()) / (m_rows * 16);
  const double db = 15 * static_cast<double>(bg.blue() - fg.blue()) / (m_rows * 16);
  const int r = fg.red(), g = fg.green(), b = fg.blue();

  bar()->fill(bg);

  QPainter p(bar());
  for (int y = 0; static_cast<uint>y < m_rows; ++y)
    // graduate the fg color
    p.fillRect(0, y * (HEIGHT + 1), WIDTH, HEIGHT,
    QColor(r + static_cast<int>(dr * y), g + static_cast<int>(dg * y),
    b + static_cast<int>(db * y)));

  {
    const QColor bg = palette().color(QPalette::Background).dark(112);

    // make a complimentary fadebar colour
    // TODO(John Maguire): dark is not always correct, dumbo!
    int h, s, v;
    palette().color(QPalette::Background).dark(150).getHsv(&h, &s, &v);
    const QColor fg(QColor::fromHsv(h + 120, s, v));

    const double dr = fg.red() - bg.red();
    const double dg = fg.green() - bg.green();
    const double db = fg.blue() - bg.blue();
    const int r = bg.red(), g = bg.green(), b = bg.blue();

    // Precalculate all fade-bar pixmaps
    for (uint y = 0; y < FADE_SIZE; ++y) {
      m_fade_bars[y].fill(palette().color(QPalette::Background));
      QPainter f(&m_fade_bars[y]);
      for (int z = 0; static_cast<uint>z < m_rows; ++z) {
        const double Y = 1.0 - (log10(FADE_SIZE - y) / log10(FADE_SIZE));
        f.fillRect(0, z * (HEIGHT + 1), WIDTH, HEIGHT,
                   QColor(r + static_cast<int>(dr * Y), g + static_cast<int>(dg * Y), b + static_cast<int>(db * Y)));
      }
    }
  }

  drawBackground();
}

void BlockAnalyzer::drawBackground() {
  const QColor bg = palette().color(QPalette::Background);
  const QColor bgdark = bg.dark(112);

  m_background.fill(bg);

  QPainter p(&m_background);
  for (int x = 0; (uint)x < m_columns; ++x)
    for (int y = 0; (uint)y < m_rows; ++y)
      p.fillRect(x * (WIDTH + 1), y * (HEIGHT + 1) + m_y, WIDTH, HEIGHT,
                 bgdark);
}
