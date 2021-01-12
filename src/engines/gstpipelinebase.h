/* This file is part of Clementine.
   Copyright 2021, Jim Broadus <jbroadus@gmail.com>

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

#ifndef GSTPIPELINEBASE_H
#define GSTPIPELINEBASE_H

#include <gst/gst.h>

#include <QObject>

class GstPipelineBase : public QObject {
 public:
  GstPipelineBase();
  virtual ~GstPipelineBase();

  virtual bool Init(const QString& name);

  // Globally unique across all pipelines.
  int id() const { return id_; }

  void DumpGraph();

 protected:
  GstElement* pipeline_;

 private:
  // Using == to compare two pipelines is a bad idea, because new ones often
  // get created in the same address as old ones.  This ID will be unique for
  // each pipeline.
  static std::atomic<int> sId;
  const int id_;
};

#endif  // GSTPIPELINEBASE_H
