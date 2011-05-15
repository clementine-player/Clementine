/****************************************************************************
**
** Copyright (C) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Script Generator project on Qt Labs.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "prigenerator.h"
#include "shellgenerator.h"
#include "reporthandler.h"
#include "fileout.h"

void PriGenerator::addHeader(const QString &folder, const QString &header)
{
    priHash[folder].headers << header;
}

void PriGenerator::addSource(const QString &folder, const QString &source)
{
    priHash[folder].sources << source;
}

static void collectAndRemoveFile(QTextStream& stream, const QString& file) {
  QFile f(file);
  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString s = QString::fromLatin1(f.readAll());
    if (file.endsWith(".cpp")) {
      // remove first line include
      s = s.mid(s.indexOf('\n')+1);
    }
    stream << s;
    f.close();
    QFile::remove(file);
  }
}
                   
static QString combineIncludes(const QString& text) {
  QStringList lines = text.split('\n');
  QSet<QString> includes;
  QString result;
  foreach(QString line, lines) {
    if (line.startsWith("#include")) {
      includes.insert(line);
    } else if (line.startsWith("#")) {
      // skip preprocessor stuff
    } else {
      result += line + "\n";
    }
  }
  QStringList includeList = includes.toList();
  qSort(includeList);
  result = includeList.join("\n") + result;
  return result;
}

static QStringList compactFiles(const QStringList& list, const QString& ext, const QString& dir, const QString& prefix) {
  QStringList outList;
  int count = list.count();
  int fileNum = 0;
  QString srcDir = dir;
  if (dir.endsWith("_builtin")) {
    srcDir = dir.left(dir.length()-strlen("_builtin"));
  }
  while (count>0) {
    QString outFileName = prefix + QString::number(fileNum) + ext;
    FileOut file(dir + "/" + outFileName);
    if (ext == ".cpp") {
      file.stream << "#include \"" + prefix + QString::number(fileNum) + ".h\"\n";
    }
    outList << outFileName;
    QString allText;
    QTextStream ts(&allText);
    for (int i = 0; i<MAX_CLASSES_PER_FILE && count>0; i++) {
      collectAndRemoveFile(ts,  srcDir + "/" + list.at(list.count()-count));
      count--;
    }
    allText = combineIncludes(allText);
    file.stream << allText;
    fileNum++;
  }
  return outList;
}

void PriGenerator::generate()
{
    QHashIterator<QString, Pri> pri(priHash);
    while (pri.hasNext()) {
        pri.next();
        QStringList list = pri.value().headers;
        if (list.isEmpty())
            continue;

        QString folder = pri.key();
        folder.replace('\\','/');
        int idx = folder.indexOf('/');
        folder = folder.left(idx);

        qSort(list.begin(), list.end());
        FileOut file(m_out_dir + "/generated_cpp/" + pri.key());
      
        // strange idea to do the file compacting so late, but it is the most effective way without patching the generator a lot
        bool compact = true;
        if (compact) {
          list = compactFiles(list, ".h", m_out_dir + "/generated_cpp/" + folder, folder); 
        }
      
        file.stream << "HEADERS += \\\n";
        foreach (const QString &entry, list) {
          file.stream << "           $$PWD/" << entry << " \\\n";
        }

        file.stream << "\n";
        file.stream << "SOURCES += \\\n";
        list = pri.value().sources;
        qSort(list.begin(), list.end());
        if (compact) {
          list = compactFiles(list, ".cpp", m_out_dir + "/generated_cpp/" + folder, folder); 
        }
        foreach (const QString &entry, list) {
            file.stream << "           $$PWD/" << entry << " \\\n";
        }
        file.stream << "           $$PWD/" << folder << "_init.cpp\n";

        if (file.done())
            ++m_num_generated_written;
        ++m_num_generated;
    }
}
