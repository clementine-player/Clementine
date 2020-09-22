/* This file is part of Clementine.
   Copyright 2010, 2014, David Sansome <me@davidsansome.com>
   Copyright 2013-2014, John Maguire <john.maguire@gmail.com>
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

#ifndef CORE_ORGANISEFORMAT_H_
#define CORE_ORGANISEFORMAT_H_

#include <QSyntaxHighlighter>
#include <QTextEdit>
#include <QValidator>

#include "core/song.h"

class OrganiseFormat {
 public:
  explicit OrganiseFormat(const QString& format = QString());

  static const char* kTagPattern;
  static const char* kBlockPattern;
  static const QStringList kKnownTags;
  static const char kInvalidFatCharacters[];
  static const int kInvalidFatCharactersCount;
  static const char kInvalidPrefixCharacters[];
  static const int kInvalidPrefixCharactersCount;

  QString format() const { return format_; }
  bool replace_non_ascii() const { return replace_non_ascii_; }
  bool replace_spaces() const { return replace_spaces_; }
  bool replace_the() const { return replace_the_; }

  void set_format(const QString& v);
  void set_replace_non_ascii(bool v) { replace_non_ascii_ = v; }
  void set_replace_spaces(bool v) { replace_spaces_ = v; }
  void set_replace_the(bool v) { replace_the_ = v; }

  void add_tag_override(const QString& tag, const QString& v) {
    tag_overrides_[tag] = v;
  }
  void reset_tag_overrides() { tag_overrides_.clear(); }

  bool IsValid() const;
  QString GetFilenameForSong(const Song& song) const;

  class Validator : public QValidator {
   public:
    explicit Validator(QObject* parent = nullptr);
    QValidator::State validate(QString& format, int& pos) const;
  };

  class SyntaxHighlighter : public QSyntaxHighlighter {
   public:
    static const QRgb kValidTagColorLight;
    static const QRgb kInvalidTagColorLight;
    static const QRgb kBlockColorLight;
    static const QRgb kValidTagColorDark;
    static const QRgb kInvalidTagColorDark;
    static const QRgb kBlockColorDark;

    explicit SyntaxHighlighter(QObject* parent = nullptr);
    explicit SyntaxHighlighter(QTextEdit* parent);
    explicit SyntaxHighlighter(QTextDocument* parent);
    void highlightBlock(const QString& format);
  };

 private:
  QString ParseBlock(QString block, const Song& song,
                     bool* any_empty = nullptr) const;
  QString TagValue(const QString& tag, const Song& song) const;

  QMap<QString, QString> tag_overrides_;

  QString format_;
  bool replace_non_ascii_;
  bool replace_spaces_;
  bool replace_the_;
};

#endif  // CORE_ORGANISEFORMAT_H_
