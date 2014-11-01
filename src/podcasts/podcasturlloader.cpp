/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>

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

#include "podcasturlloader.h"

#include <QNetworkReply>

#include "podcastparser.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/utilities.h"

const int PodcastUrlLoader::kMaxRedirects = 5;

PodcastUrlLoader::PodcastUrlLoader(QObject* parent)
    : QObject(parent),
      network_(new NetworkAccessManager(this)),
      parser_(new PodcastParser),
      html_link_re_("<link (.*)>"),
      html_link_rel_re_("rel\\s*=\\s*['\"]?\\s*alternate"),
      html_link_type_re_("type\\s*=\\s*['\"]?([^'\" ]+)"),
      html_link_href_re_("href\\s*=\\s*['\"]?([^'\" ]+)") {
  html_link_re_.setMinimal(true);
  html_link_re_.setCaseSensitivity(Qt::CaseInsensitive);
}

PodcastUrlLoader::~PodcastUrlLoader() { delete parser_; }

QUrl PodcastUrlLoader::FixPodcastUrl(const QString& url_text) {
  QString url_text_copy(url_text.trimmed());

  // Thanks gpodder!
  QuickPrefixList quick_prefixes =
      QuickPrefixList()
      << QuickPrefix("fb:", "http://feeds.feedburner.com/%1")
      << QuickPrefix("yt:", "https://www.youtube.com/rss/user/%1/videos.rss")
      << QuickPrefix("sc:", "https://soundcloud.com/%1")
      << QuickPrefix("fm4od:", "http://onapp1.orf.at/webcam/fm4/fod/%1.xspf")
      << QuickPrefix("ytpl:",
                     "https://gdata.youtube.com/feeds/api/playlists/%1");

  // Check if it matches one of the quick prefixes.
  for (QuickPrefixList::const_iterator it = quick_prefixes.constBegin();
       it != quick_prefixes.constEnd(); ++it) {
    if (url_text_copy.startsWith(it->first)) {
      url_text_copy = it->second.arg(url_text_copy.mid(it->first.length()));
    }
  }

  if (!url_text_copy.contains("://")) {
    url_text_copy.prepend("http://");
  }

  return FixPodcastUrl(QUrl(url_text_copy));
}

QUrl PodcastUrlLoader::FixPodcastUrl(const QUrl& url_orig) {
  QUrl url(url_orig);

  // Replace schemes
  if (url.scheme().isEmpty() || url.scheme() == "feed" ||
      url.scheme() == "itpc" || url.scheme() == "itms") {
    url.setScheme("http");
  } else if (url.scheme() == "zune" && url.host() == "subscribe" &&
             !url.queryItems().isEmpty()) {
    url = QUrl(url.queryItems()[0].second);
  }

  return url;
}

PodcastUrlLoaderReply* PodcastUrlLoader::Load(const QString& url_text) {
  return Load(FixPodcastUrl(url_text));
}

PodcastUrlLoaderReply* PodcastUrlLoader::Load(const QUrl& url) {
  // Create a reply
  PodcastUrlLoaderReply* reply = new PodcastUrlLoaderReply(url, this);

  // Create a state object to track this request
  RequestState* state = new RequestState;
  state->redirects_remaining_ = kMaxRedirects + 1;
  state->reply_ = reply;

  // Start the first request
  NextRequest(url, state);

  return reply;
}

void PodcastUrlLoader::SendErrorAndDelete(const QString& error_text,
                                          RequestState* state) {
  state->reply_->SetFinished(error_text);
  delete state;
}

void PodcastUrlLoader::NextRequest(const QUrl& url, RequestState* state) {
  // Stop the request if there have been too many redirects already.
  if (state->redirects_remaining_-- == 0) {
    SendErrorAndDelete(tr("Too many redirects"), state);
    return;
  }

  qLog(Debug) << "Loading URL" << url;

  QNetworkRequest req(url);
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);
  QNetworkReply* network_reply = network_->get(req);

  NewClosure(network_reply, SIGNAL(finished()), this,
             SLOT(RequestFinished(RequestState*, QNetworkReply*)), state,
             network_reply);
}

void PodcastUrlLoader::RequestFinished(RequestState* state,
                                       QNetworkReply* reply) {
  reply->deleteLater();

  if (reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isValid()) {
    const QUrl next_url = reply->url().resolved(
        reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl());

    NextRequest(next_url, state);
    return;
  }

  // Check for errors.
  if (reply->error() != QNetworkReply::NoError) {
    SendErrorAndDelete(reply->errorString(), state);
    return;
  }

  const QVariant http_status =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  if (http_status.isValid() && http_status.toInt() != 200) {
    SendErrorAndDelete(
        QString("HTTP %1: %2")
            .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                     .toString(),
                 reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute)
                     .toString()),
        state);
    return;
  }

  // Check the mime type.
  const QString content_type =
      reply->header(QNetworkRequest::ContentTypeHeader).toString();
  if (parser_->SupportsContentType(content_type)) {
    const QVariant ret = parser_->Load(reply, reply->url());

    if (ret.canConvert<Podcast>()) {
      state->reply_->SetFinished(PodcastList() << ret.value<Podcast>());
    } else if (ret.canConvert<OpmlContainer>()) {
      state->reply_->SetFinished(ret.value<OpmlContainer>());
    } else {
      SendErrorAndDelete(tr("Failed to parse the XML for this RSS feed"),
                         state);
      return;
    }

    delete state;
    return;
  } else if (content_type.contains("text/html")) {
    // I don't want a full HTML parser here, so do this the dirty way.
    const QString page_text = QString::fromUtf8(reply->readAll());
    int pos = 0;
    while ((pos = html_link_re_.indexIn(page_text, pos)) != -1) {
      const QString link = html_link_re_.cap(1).toLower();
      pos += html_link_re_.matchedLength();

      if (html_link_rel_re_.indexIn(link) == -1 ||
          html_link_type_re_.indexIn(link) == -1 ||
          html_link_href_re_.indexIn(link) == -1) {
        continue;
      }

      const QString link_type = html_link_type_re_.cap(1);
      const QString href =
          Utilities::DecodeHtmlEntities(html_link_href_re_.cap(1));

      if (parser_->supported_mime_types().contains(link_type)) {
        NextRequest(QUrl(href), state);
        return;
      }
    }

    SendErrorAndDelete(tr("HTML page did not contain any RSS feeds"), state);
  } else {
    SendErrorAndDelete(tr("Unknown content-type") + ": " + content_type, state);
  }
}

PodcastUrlLoaderReply::PodcastUrlLoaderReply(const QUrl& url, QObject* parent)
    : QObject(parent), url_(url), finished_(false) {}

void PodcastUrlLoaderReply::SetFinished(const PodcastList& results) {
  result_type_ = Type_Podcast;
  podcast_results_ = results;
  finished_ = true;
  emit Finished(true);
}

void PodcastUrlLoaderReply::SetFinished(const OpmlContainer& results) {
  result_type_ = Type_Opml;
  opml_results_ = results;
  finished_ = true;
  emit Finished(true);
}

void PodcastUrlLoaderReply::SetFinished(const QString& error_text) {
  error_text_ = error_text;
  finished_ = true;
  emit Finished(false);
}
