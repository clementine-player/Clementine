#include "glblockanalyzer.h"

#include <QtDebug>

const char* GLBlockAnalyzer::kName = "GL Block Analyzer";

GLBlockAnalyzer::GLBlockAnalyzer(QWidget* parent)
    : AnalyzerBase(parent),
      rectangles_size_(0),
      shader_(this) {
  qDebug() << Q_FUNC_INFO;
}

void GLBlockAnalyzer::SpectrumAvailable(const QVector<float>& spectrum) {
  updateGL();
  current_spectrum_ = spectrum;
}

void GLBlockAnalyzer::initializeGL() {
  qDebug() << Q_FUNC_INFO;
  glClearColor(0.0, 0.0, 0.0, 1.0);
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
  qDebug() << Q_FUNC_INFO;
  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void GLBlockAnalyzer::paintGL() {
  if (current_spectrum_.size() != rectangles_size_) {
    rectangles_.reset(new float[current_spectrum_.size() * 3 * 4]);
    rectangles_size_ = current_spectrum_.size();
    std::fill(rectangles_.get(), rectangles_.get() + rectangles_size_ * 3 * 4, 0.0f);

    tex_coords_.reset(new float[rectangles_size_ * 2 * 4]);
    for (int i = 0; i < rectangles_size_ * 2 * 4; i += 2 * 4) {
      tex_coords_[i] = 0.0;
      tex_coords_[i+1] = 0.0;

      tex_coords_[i+2] = 1.0;
      tex_coords_[i+3] = 0.0;

      tex_coords_[i+4] = 1.0;
      tex_coords_[i+5] = 1.0;

      tex_coords_[i+6] = 0.0;
      tex_coords_[i+7] = 1.0;
    }
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  const float width = 2.0 / current_spectrum_.size();

  // Now we have x 0.0 -> 1.0 and y 0.0 -> 1.0.
  glTranslatef(-1.0, -1.0, 0.0);
  glScalef(2.0, 2.0, 1.0);


  for (int i = 0; i < current_spectrum_.size(); ++i) {
    const float x = width * i;
    const float height = current_spectrum_[i] + 0.2;

    float* current_rectangle = rectangles_.get() + i*4*3;  // 4 points of size 3.
    float* bottom_left = current_rectangle;
    float* bottom_right = current_rectangle + 3;
    float* top_right = current_rectangle + 6;
    float* top_left = current_rectangle + 9;

    bottom_left[0] = x;
    bottom_left[1] = 0.0;

    bottom_right[0] = x + width;
    bottom_right[1] = 0.0;

    top_right[0] = x + width;
    top_right[1] = height;

    top_left[0] = x;
    top_left[1] = height;
  }

  shader_.bind();

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  // Draw rectangles.
  glVertexPointer(3, GL_FLOAT, 0, rectangles_.get());
  glTexCoordPointer(2, GL_FLOAT, 0, tex_coords_.get());

  glColor3f(1.0, 0.0, 0.0);
  glPolygonMode(GL_FRONT, GL_FILL);
  glDrawArrays(GL_QUADS, 0, rectangles_size_ * 4);

  // Draw outlines.
  glColor3f(1.0, 1.0, 1.0);
  glPolygonMode(GL_FRONT, GL_LINE);
  glDrawArrays(GL_QUADS, 0, rectangles_size_ * 4);

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}
