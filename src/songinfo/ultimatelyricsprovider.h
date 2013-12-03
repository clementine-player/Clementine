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

#ifndef ULTIMATELYRICSPROVIDER_H
#define ULTIMATELYRICSPROVIDER_H

#include <QObject>
#include <QPair>
#include <QStringList>

#include "songinfoprovider.h"

class NetworkAccessManager;

class QNetworkReply;

class UltimateLyricsProvider : public SongInfoProvider {
  Q_OBJECT

public:
  UltimateLyricsProvider();

  static const int kRedirectLimit;

  typedef QPair<QString, QString> RuleItem;
  typedef QList<RuleItem> Rule;
  typedef QPair<QString, QString> UrlFormat;

  void set_name(const QString& name) { name_ = name; }
  void set_title(const QString& title) { title_ = title; }
  void set_url(const QString& url) { url_ = url; }
  void set_charset(const QString& charset) { charset_ = charset; }
  void set_relevance(int relevance) { relevance_ = relevance; }

  void add_url_format(const QString& replace, const QString& with) {
    url_formats_ << UrlFormat(replace, with); }

  void add_extract_rule(const Rule& rule) { extract_rules_ << rule; }
  void add_exclude_rule(const Rule& rule) { exclude_rules_ << rule; }
  void add_invalid_indicator(const QString& indicator) { invalid_indicators_ << indicator; }

  QString name() const { return name_; }
  int relevance() const { return relevance_; }

  void FetchInfo(int id, const Song& metadata);

private slots:
  void LyricsFetched();

private:
  bool ApplyExtractRule(const Rule& rule, QString* content) const;
  void ApplyExcludeRule(const Rule& rule, QString* content) const;

  static QString ExtractUrl(const QString& source, const Rule& rule);
  static QString ExtractXmlTag(const QString& source, const QString& tag);
  static QString Extract(const QString& source, const QString& begin, const QString& end);
  static QString ExcludeXmlTag(const QString& source, const QString& tag);
  static QString Exclude(const QString& source, const QString& begin, const QString& end);
  static QString FirstChar(const QString& text);
  static QString TitleCase(const QString& text);
  static QString NoSpace(const QString& text);

  void ReplaceField(const QString& tag, const QString& value, QString* text) const;
  void ReplaceFields(const Song& metadata, QString* text) const;

private:
  NetworkAccessManager* network_;
  QMap<QNetworkReply*, int> requests_;

  QString name_;
  QString title_;
  QString url_;
  QString charset_;
  int relevance_;

  QList<UrlFormat> url_formats_;
  QList<Rule> extract_rules_;
  QList<Rule> exclude_rules_;
  QStringList invalid_indicators_;

  Song metadata_;
  int redirect_count_;
  bool url_hop_;
};

#endif // ULTIMATELYRICSPROVIDER_H
