#ifndef _PYTHONQTSTDDECORATORS_H
#define _PYTHONQTSTDDECORATORS_H

/*
 *
 *  Copyright (C) 2010 MeVis Medical Solutions AG All Rights Reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  Further, this software is distributed without any warranty that it is
 *  free of the rightful claim of any third person regarding infringement
 *  or the like.  Any license provided herein, whether implied or
 *  otherwise, applies only to this software file.  Patent licenses, if
 *  any, provided herein do not apply to combinations of this program with
 *  other software, or any other product whatsoever.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Contact information: MeVis Medical Solutions AG, Universitaetsallee 29,
 *  28359 Bremen, Germany or:
 *
 *  http://www.mevis.de
 *
 */

//----------------------------------------------------------------------------------
/*!
// \file    PythonQtStdDecorators.h
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2007-04
*/
//----------------------------------------------------------------------------------

#include "PythonQtSystem.h"
#include <Python.h>
#include <QObject>
#include <QVariantList>
#include <QTextDocument>
#include <QColor>
#include <QDateTime>
#include <QDate>
#include <QTime>

class PYTHONQT_EXPORT PythonQtStdDecorators : public QObject
{
  Q_OBJECT

public slots:
  bool connect(QObject* sender, const QByteArray& signal, PyObject* callable);
  bool connect(QObject* sender, const QByteArray& signal, QObject* receiver, const QByteArray& slot);
  bool disconnect(QObject* sender, const QByteArray& signal, PyObject* callable);
  bool disconnect(QObject* sender, const QByteArray& signal, QObject* receiver, const QByteArray& slot);

#undef emit
  void emit(QObject* sender, const QByteArray& signal, PyObject* arg1 = NULL,PyObject* arg2 = NULL,
            PyObject* arg3 = NULL,PyObject* arg4 = NULL,PyObject* arg5 = NULL,PyObject* arg6 = NULL,PyObject* arg7 = NULL);
#define emit
  
  QObject* parent(QObject* o);
  void setParent(QObject* o, QObject* parent);

  const QObjectList* children(QObject* o);
  QObject* findChild(QObject* parent, PyObject* type, const QString& name = QString());
  QList<QObject*> findChildren(QObject* parent, PyObject* type, const QString& name= QString());
  QList<QObject*> findChildren(QObject* parent, PyObject* type, const QRegExp& regExp);
  
  bool setProperty(QObject* o, const char* name, const QVariant& value);
  QVariant property(QObject* o, const char* name);

  double static_Qt_qAbs(double a) { return qAbs(a); }
  double static_Qt_qBound(double a,double b,double c) { return qBound(a,b,c); }
  void static_Qt_qDebug(const QByteArray& msg) { qDebug(msg.constData()); }
  // TODO: multi arg qDebug...
  void static_Qt_qWarning(const QByteArray& msg) { qWarning(msg.constData()); }
  // TODO: multi arg qWarning...
  void static_Qt_qCritical(const QByteArray& msg) { qCritical(msg.constData()); }
  // TODO: multi arg qCritical...
  void static_Qt_qFatal(const QByteArray& msg) { qFatal(msg.constData()); }
  // TODO: multi arg qFatal...
  bool static_Qt_qFuzzyCompare(double a, double b) { return qFuzzyCompare(a, b); }
  double static_Qt_qMax(double a, double b) { return qMax(a, b); }
  double static_Qt_qMin(double a, double b) { return qMin(a, b); }
  int static_Qt_qRound(double a) { return qRound(a); }
  qint64 static_Qt_qRound64(double a) { return qRound64(a); }
  const char* static_Qt_qVersion() { return qVersion(); }
  int static_Qt_qrand() { return qrand(); }
  void static_Qt_qsrand(uint a) { qsrand(a); }

  QString tr(QObject* obj, const QByteArray& text, const QByteArray& ambig = QByteArray(), int n = -1);
  
  QByteArray static_Qt_SIGNAL(const QByteArray& s) { return QByteArray("2") + s; }
  QByteArray static_Qt_SLOT(const QByteArray& s) { return QByteArray("1") + s; }

private:
  QObject* findChild(QObject* parent, const char* typeName, const QMetaObject* meta, const QString& name);
  int findChildren(QObject* parent, const char* typeName, const QMetaObject* meta, const QString& name, QList<QObject*>& list);
  int findChildren(QObject* parent, const char* typeName, const QMetaObject* meta, const QRegExp& regExp, QList<QObject*>& list);
};


#endif
