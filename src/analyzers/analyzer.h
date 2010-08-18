#ifndef ANALYZER_H
#define ANALYZER_H

#include <QVector>
#include <QGLWidget>

#include "engines/engine_fwd.h"

class AnalyzerBase : public QGLWidget {
  Q_OBJECT
 public:
  AnalyzerBase(QWidget* parent = 0);

 public slots:
  void set_engine(Engine::Base* engine);

 protected slots:
  virtual void SpectrumAvailable(const QVector<float>& spectrum) = 0;

 private:

 protected:
  Engine::Base* engine_;
};

#endif
