/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
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

#ifndef PODCASTS_PODCASTURLLOADER_H_
#define PODCASTS_PODCASTURLLOADER_H_

#include <QObject>
#include <QRegExp>

#include "opmlcontainer.h"
#include "podcast.h"

class PodcastParser;

class QNetworkAccessManager;
class QNetworkReply;

class PodcastUrlLoaderReply : public QObject {
  Q_OBJECT

 public:
  PodcastUrlLoaderReply(const QUrl& url, QObject* parent);

  enum ResultType { Type_Podcast, Type_Opml };

  const QUrl& url() const { return url_; }
  bool is_finished() const { return finished_; }
  bool is_success() const { return error_text_.isEmpty(); }
  const QString& error_text() const { return error_text_; }

  ResultType result_type() const { return result_type_; }
  const PodcastList& podcast_results() const { return podcast_results_; }
  const OpmlContainer& opml_results() const { return opml_results_; }

  void SetFinished(const QString& error_text);
  void SetFinished(const PodcastList& results);
  void SetFinished(const OpmlContainer& results);

 signals:
  void Finished(bool success);

 private:
  QUrl url_;
  bool finished_;
  QString error_text_;

  ResultType result_type_;
  PodcastList podcast_results_;
  OpmlContainer opml_results_;
};

class PodcastUrlLoader : public QObject {
  Q_OBJECT

 public:
  explicit PodcastUrlLoader(QObject* parent = nullptr);
  ~PodcastUrlLoader();

  static const int kMaxRedirects;

  PodcastUrlLoaderReply* Load(const QString& url_text);
  PodcastUrlLoaderReply* Load(const QUrl& url);

  // Both the FixPodcastUrl functions replace common podcatcher URL schemes
  // like itpc:// or zune:// with their http:// equivalents.  The QString
  // overload also cleans up user-entered text a bit - stripping whitespace and
  // applying shortcuts like sc:tag.
  static QUrl FixPodcastUrl(const QString& url_text);
  static QUrl FixPodcastUrl(const QUrl& url);

 private:
  struct RequestState {
    int redirects_remaining_;
    PodcastUrlLoaderReply* reply_;
  };

  typedef QPair<QString, QString> QuickPrefix;
  typedef QList<QuickPrefix> QuickPrefixList;

 private slots:
  void RequestFinished(RequestState* state, QNetworkReply* reply);

 private:
  void SendErrorAndDelete(const QString& error_text, RequestState* state);
  void NextRequest(const QUrl& url, RequestState* state);

 private:
  QNetworkAccessManager* network_;
  PodcastParser* parser_;

  QRegExp html_link_re_;
  QRegExp whitespace_re_;
  QRegExp html_link_rel_re_;
  QRegExp html_link_type_re_;
  QRegExp html_link_href_re_;
};

#endif  // PODCASTS_PODCASTURLLOADER_H_
