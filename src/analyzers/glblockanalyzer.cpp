#include "glblockanalyzer.h"

#include <algorithm>
#include <cmath>

#include <QtDebug>

#include <QApplication>
#include <QColor>
#include <QPalette>

const char* GLBlockAnalyzer::kName = "GL Block Analyzer";

GLBlockAnalyzer::GLBlockAnalyzer(QWidget* parent)
    : AnalyzerBase(parent),
      current_spectrum_(200, 0.0),
      rectangles_size_(0),
      shader_(this) {
}

void GLBlockAnalyzer::SpectrumAvailable(const QVector<float>& spectrum) {
  current_spectrum_ = spectrum;
  updateGL();
}

void GLBlockAnalyzer::initializeGL() {

  QColor background_color = QApplication::palette().color(QPalette::Window);
  qglClearColor(background_color);

  glDisable(GL_DEPTH_TEST);

  shader_.addShaderFromSourceFile(QGLShader::Vertex, ":shaders/glblock_vert.glsl");
  shader_.addShaderFromSourceFile(QGLShader::Fragment, ":shaders/glblock_frag.glsl");
  shader_.link();
  if (!shader_.isLinked()) {
    qWarning() << "Shader failed to link"
               << shader_.log();
  }
}

void GLBlockAnalyzer::resizeGL(int w, int h) {
  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  num_columns_ = w / kRectPixels;

  rectangles_size_ = num_columns_ * kNumRectangles * 4 * 3;
  rectangles_.reset(new float[rectangles_size_]);
  const float rect_height = 1.0 / kNumRectangles;
  const float rect_width = 1.0 / num_columns_;
  const int row_size = 3 * 4 * num_columns_;
  for (int y = 0; y < kNumRectangles; ++y) {
    for (int x = 0; x < num_columns_; ++x) {
      float* pos = &rectangles_[y * row_size + x * 3 * 4];
      float* bottom_left = pos;
      float* bottom_right = pos + 3;
      float* top_right = pos + 6;
      float* top_left = pos + 9;

      bottom_left[0] = x * rect_width;
      bottom_left[1] = y * rect_height;
      bottom_left[2] = 0.0;

      bottom_right[0] = (x + 1) * rect_width;
      bottom_right[1] = y * rect_height;
      bottom_right[2] = 0.0;

      top_right[0] = (x + 1) * rect_width;
      top_right[1] = (y + 1) * rect_height;
      top_right[2] = 0.0;

      top_left[0] = x * rect_width;
      top_left[1] = (y + 1) * rect_height;
      top_left[2] = 0.0;
    }
  }
}

namespace {
class ColourGenerator {
 public:
  ColourGenerator(const QColor& colour)
      : i_(0) {
    rgb_[0] = colour.redF();
    rgb_[1] = colour.greenF();
    rgb_[2] = colour.blueF();
  }
  float operator() () {
    return rgb_[i_++ % 3];
  }
 private:
  int i_;
  float rgb_[3];
};
}  // namespace

void GLBlockAnalyzer::paintGL() {
  colours_.reset(new float[rectangles_size_]);
  std::fill(colours_.get(), colours_.get() + rectangles_size_, 0.8);

  QColor bg_colour = QApplication::palette().color(QPalette::Window);
  std::generate(colours_.get(), colours_.get() + rectangles_size_,
      ColourGenerator(bg_colour.darker(120)));

  const int row_size = 3 * 4 * num_columns_;
  for (int x = 0; x < num_columns_; ++x) {
    int peak_rect = current_spectrum_[x] * 10;
    float* peak = &colours_[peak_rect * row_size + x * 3 * 4];
    const QColor& colour = Qt::blue;
    std::generate(peak, peak + 3 * 4, ColourGenerator(colour));
    for (int i = 0; i < peak_rect; ++i) {
      float* p = &colours_[i * row_size + x * 3 * 4];
      const QColor& c = colour.lighter(std::pow(double(i - peak_rect), 2) * 10 + 100);
      if (c.valueF() > bg_colour.valueF()) {
        std::generate(p, p + 3 * 4, ColourGenerator(bg_colour));
      } else {
        std::generate(p, p + 3 * 4, ColourGenerator(c));
      }
    }
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  // Now we have x 0.0 -> 1.0 and y 0.0 -> 1.0.
  glTranslatef(-1.0, -1.0, 0.0);
  glScalef(2.0, 2.0, 1.0);

  //shader_.bind();

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  // Draw rectangles.
  glVertexPointer(3, GL_FLOAT, 0, rectangles_.get());
  glColorPointer(3, GL_FLOAT, 0, colours_.get());

  glColor3f(1.0, 0.0, 0.0);
  glPolygonMode(GL_FRONT, GL_FILL);
  glDrawArrays(GL_QUADS, 0, rectangles_size_ / 3);

  glDisableClientState(GL_COLOR_ARRAY);
  // Draw outlines.
  qglColor(bg_colour);
  glPolygonMode(GL_FRONT, GL_LINE);
  glDrawArrays(GL_QUADS, 0, rectangles_size_ / 3);

  glDisableClientState(GL_VERTEX_ARRAY);
}
