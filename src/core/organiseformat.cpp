/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "core/organiseformat.h"

#include <QApplication>
#include <QFileInfo>
#include <QPalette>
#include <QUrl>

#include "core/timeconstants.h"
#include "core/utilities.h"

const char* OrganiseFormat::kTagPattern = "\\%([a-zA-Z]*)";
const char* OrganiseFormat::kBlockPattern = "\\{([^{}]+)\\}";
const QStringList OrganiseFormat::kKnownTags = QStringList()
    << "title" << "album" << "artist" << "artistinitial" << "albumartist"
    << "composer" << "track" << "disc" << "bpm" << "year" << "genre"
    << "comment" << "length" << "bitrate" << "samplerate" << "extension"
    << "performer" << "grouping";

// From http://en.wikipedia.org/wiki/8.3_filename#Directory_table
const char* OrganiseFormat::kInvalidFatCharacters = "\"*/\\:<>?|";
const int OrganiseFormat::kInvalidFatCharactersCount =
    strlen(OrganiseFormat::kInvalidFatCharacters);

const QRgb OrganiseFormat::SyntaxHighlighter::kValidTagColorLight =
    qRgb(64, 64, 255);
const QRgb OrganiseFormat::SyntaxHighlighter::kInvalidTagColorLight =
    qRgb(255, 64, 64);
const QRgb OrganiseFormat::SyntaxHighlighter::kBlockColorLight =
    qRgb(230, 230, 230);

const QRgb OrganiseFormat::SyntaxHighlighter::kValidTagColorDark =
    qRgb(128, 128, 255);
const QRgb OrganiseFormat::SyntaxHighlighter::kInvalidTagColorDark =
    qRgb(255, 128, 128);
const QRgb OrganiseFormat::SyntaxHighlighter::kBlockColorDark =
    qRgb(64, 64, 64);

OrganiseFormat::OrganiseFormat(const QString &format)
  : format_(format),
    replace_non_ascii_(false),
    replace_spaces_(false),
    replace_the_(false) {
}

void OrganiseFormat::set_format(const QString &v) {
  format_ = v;
  format_.replace('\\', '/');
}

bool OrganiseFormat::IsValid() const {
  int pos = 0;
  QString format_copy(format_);

  Validator v;
  return v.validate(format_copy, pos) == QValidator::Acceptable;
}

QString OrganiseFormat::GetFilenameForSong(const Song &song) const {
  QString filename = ParseBlock(format_, song);

  if (QFileInfo(filename).completeBaseName().isEmpty()) {
    // Avoid having empty filenames, or filenames with extension only: in this
    // case, keep the original filename.
    // We remove the extension from "filename" if it exists, as song.basefilename()
    // also contains the extension.
    filename = Utilities::PathWithoutFilenameExtension(filename) + song.basefilename();
  }

  if (replace_spaces_)
    filename.replace(QRegExp("\\s"), "_");

  if (replace_non_ascii_) {
    QString stripped;
    for (int i = 0 ; i < filename.length(); ++i) {
      const QCharRef c = filename[i];
      if (c < 128) {
        stripped.append(c);
      } else {
        const QString decomposition = c.decomposition();
        if (!decomposition.isEmpty() && decomposition[0] < 128)
          stripped.append(decomposition[0]);
        else
          stripped.append("_");
      }
    }
    filename = stripped;
  }

  return filename;
}

QString OrganiseFormat::ParseBlock(QString block, const Song& song,
                                   bool* any_empty) const {
  QRegExp tag_regexp(kTagPattern);
  QRegExp block_regexp(kBlockPattern);

  // Find any blocks first
  int pos = 0;
  while ((pos = block_regexp.indexIn(block, pos)) != -1) {
    // Recursively parse the block
    bool empty = false;
    QString value = ParseBlock(block_regexp.cap(1), song, &empty);
    if (empty)
      value = "";

    // Replace the block's value
    block.replace(pos, block_regexp.matchedLength(), value);
    pos += value.length();
  }

  // Now look for tags
  bool empty = false;
  pos = 0;
  while ((pos = tag_regexp.indexIn(block, pos)) != -1) {
    QString value = TagValue(tag_regexp.cap(1), song);
    if (value.isEmpty())
      empty = true;

    block.replace(pos, tag_regexp.matchedLength(), value);
    pos += value.length();
  }

  if (any_empty)
    *any_empty = empty;
  return block;
}

QString OrganiseFormat::TagValue(const QString &tag, const Song &song) const {
  QString value;

  if (tag == "title")            value = song.title();
  else if (tag == "album")       value = song.album();
  else if (tag == "artist")      value = song.artist();
  else if (tag == "composer")    value = song.composer();
  else if (tag == "performer")   value = song.performer();
  else if (tag == "grouping")    value = song.grouping();
  else if (tag == "genre")       value = song.genre();
  else if (tag == "comment")     value = song.comment();
  else if (tag == "year")        value = QString::number(song.year());
  else if (tag == "track")       value = QString::number(song.track());
  else if (tag == "disc")        value = QString::number(song.disc());
  else if (tag == "bpm")         value = QString::number(song.bpm());
  else if (tag == "length")      value =
      QString::number(song.length_nanosec() / kNsecPerSec);
  else if (tag == "bitrate")     value = QString::number(song.bitrate());
  else if (tag == "samplerate")  value = QString::number(song.samplerate());
  else if (tag == "extension")   value = QFileInfo(song.url().toLocalFile()).suffix();
  else if (tag == "artistinitial") {
    value = song.effective_albumartist().trimmed();
    if (replace_the_ && !value.isEmpty())
      value.replace(QRegExp("^the\\s+", Qt::CaseInsensitive), "");
    if (!value.isEmpty()) value = value[0].toUpper();
  } else if (tag == "albumartist") {
    value = song.is_compilation()
        ?  "Various Artists"
        : song.effective_albumartist();
  }

  if (replace_the_ && (tag == "artist" || tag == "albumartist"))
    value.replace(QRegExp("^the\\s+", Qt::CaseInsensitive), "");

  if (value == "0" || value == "-1")
    value = "";

  // Prepend a 0 to single-digit track numbers
  if (tag == "track" && value.length() == 1)
    value.prepend('0');

  // Replace characters that really shouldn't be in paths
  for (int i = 0 ; i < kInvalidFatCharactersCount; ++i) {
    value.replace(kInvalidFatCharacters[i], '_');
  }

  return value;
}


OrganiseFormat::Validator::Validator(QObject *parent)
  : QValidator(parent) {}

QValidator::State OrganiseFormat::Validator::validate(
    QString& input, int&) const {
  QRegExp tag_regexp(kTagPattern);

  // Make sure all the blocks match up
  int block_level = 0;
  for (int i = 0 ; i < input.length(); ++i) {
    if (input[i] == '{')
      block_level++;
    else if (input[i] == '}')
      block_level--;

    if (block_level < 0 || block_level > 1)
      return QValidator::Invalid;
  }

  if (block_level != 0)
    return QValidator::Invalid;

  // Make sure the tags are valid
  int pos = 0;
  while ((pos = tag_regexp.indexIn(input, pos)) != -1) {
    if (!OrganiseFormat::kKnownTags.contains(tag_regexp.cap(1)))
      return QValidator::Invalid;

    pos += tag_regexp.matchedLength();
  }

  return QValidator::Acceptable;
}


OrganiseFormat::SyntaxHighlighter::SyntaxHighlighter(QObject *parent)
  : QSyntaxHighlighter(parent) {}

OrganiseFormat::SyntaxHighlighter::SyntaxHighlighter(QTextEdit* parent)
  : QSyntaxHighlighter(parent) {}

OrganiseFormat::SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent)
  : QSyntaxHighlighter(parent) {}

void OrganiseFormat::SyntaxHighlighter::highlightBlock(const QString& text) {
  const bool light =
      QApplication::palette().color(QPalette::Base).value() > 128;
  const QRgb block_color = light ? kBlockColorLight : kBlockColorDark;
  const QRgb valid_tag_color = light ? kValidTagColorLight : kValidTagColorDark;
  const QRgb invalid_tag_color =
      light ? kInvalidTagColorLight : kInvalidTagColorDark;

  QRegExp tag_regexp(kTagPattern);
  QRegExp block_regexp(kBlockPattern);

  QTextCharFormat block_format;
  block_format.setBackground(QColor(block_color));

  // Reset formatting
  setFormat(0, text.length(), QTextCharFormat());

  // Blocks
  int pos = 0;
  while ((pos = block_regexp.indexIn(text, pos)) != -1) {
    setFormat(pos, block_regexp.matchedLength(), block_format);

    pos += block_regexp.matchedLength();
  }

  // Tags
  pos = 0;
  while ((pos = tag_regexp.indexIn(text, pos)) != -1) {
    QTextCharFormat f = format(pos);
    f.setForeground(
        QColor(OrganiseFormat::kKnownTags.contains(tag_regexp.cap(1))
               ? valid_tag_color
               : invalid_tag_color));

    setFormat(pos, tag_regexp.matchedLength(), f);
    pos += tag_regexp.matchedLength();
  }
}

