/* This file is part of Clementine.

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

#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <gst/gst.h>

#include <QObject>
#include <QStringList>
#include <QFutureWatcher>
#include <QMetaType>

class Transcoder;

class TranscoderFormat {
  friend class Transcoder;

 public:
  virtual ~TranscoderFormat() {}

  virtual QString name() const = 0;
  virtual QString file_extension() const = 0;

 protected:
  virtual GstElement* CreateOutputBin() const = 0;

  GstElement* CreateElement(const QString& factory_name, GstElement* bin = NULL,
                            const QString& name = QString()) const;
  GstElement* CreateBin(const QStringList& elements) const;
};
Q_DECLARE_METATYPE(const TranscoderFormat*);


class Transcoder : public QObject {
  Q_OBJECT

 public:
  Transcoder(QObject* parent = 0);
  ~Transcoder();

  QList<const TranscoderFormat*> formats() const;

  void AddJob(const QString& input, const TranscoderFormat* output_format,
              const QString& output = QString());

 public slots:
  void Start();

 signals:
  void JobComplete(const QString& filename, bool success);
  void AllJobsComplete();

 private:
  struct Job {
    QString input;
    QString output;
    const TranscoderFormat* output_format;
  };

  void RunJob(const Job& job);

 private slots:
  void JobsFinished();

 private:
  QList<TranscoderFormat*> formats_;
  QList<Job> jobs_;

  QFutureWatcher<void>* future_watcher_;
};

#endif // TRANSCODER_H
