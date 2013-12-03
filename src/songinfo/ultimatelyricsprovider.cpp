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

#include "songinfotextview.h"
#include "ultimatelyricslyric.h"
#include "ultimatelyricsprovider.h"
#include "core/logging.h"
#include "core/network.h"

#include <QCoreApplication>
#include <QNetworkReply>
#include <QTextCodec>
#include <QThread>

#include <boost/scoped_ptr.hpp>

const int UltimateLyricsProvider::kRedirectLimit = 5;


UltimateLyricsProvider::UltimateLyricsProvider()
  : network_(new NetworkAccessManager(this)),
    relevance_(0),
    redirect_count_(0),
    url_hop_(false)
{
}

void UltimateLyricsProvider::FetchInfo(int id, const Song& metadata) {
  // Get the text codec
  const QTextCodec* codec = QTextCodec::codecForName(charset_.toAscii().constData());
  if (!codec) {
    qLog(Warning) << "Invalid codec" << charset_;
    emit Finished(id);
    return;
  }

  // Fill in fields in the URL
  QString url_text(url_);
  ReplaceFields(metadata, &url_text);

  QUrl url(url_text);
  qLog(Debug) << "Fetching lyrics from" << url;

  // Fetch the URL, follow redirects
  metadata_ = metadata;
  redirect_count_ = 0;
  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  requests_[reply] = id;
  connect(reply, SIGNAL(finished()), SLOT(LyricsFetched()));
}

void UltimateLyricsProvider::LyricsFetched() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    url_hop_ = false;
    return;
  }

  int id = requests_.take(reply);
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    url_hop_ = false;
    emit Finished(id);
    return;
  }

  // Handle redirects
  QVariant redirect_target = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
  if (redirect_target.isValid()) {
    if (redirect_count_ >= kRedirectLimit) {
      url_hop_ = false;
      emit Finished(id);
      return;
    }

    QUrl target = redirect_target.toUrl();
    if (target.scheme().isEmpty() || target.host().isEmpty()) {
      QString path = target.path();
      target = reply->url();
      target.setPath(path);
    }

    redirect_count_ ++;
    QNetworkReply* reply = network_->get(QNetworkRequest(target));
    requests_[reply] = id;
    connect(reply, SIGNAL(finished()), SLOT(LyricsFetched()));
    return;
  }

  const QTextCodec* codec = QTextCodec::codecForName(charset_.toAscii().constData());
  const QString original_content = codec->toUnicode(reply->readAll());
  QString lyrics;

  // Check for invalid indicators
  foreach (const QString& indicator, invalid_indicators_) {
    if (original_content.contains(indicator)) {
      qLog(Debug) << "Found invalid indicator" << indicator;
      url_hop_ = false;
      emit Finished(id);
      return;
    }
  }

  if (!url_hop_) {
    // Apply extract rules
    foreach (const Rule& rule, extract_rules_) {
      // Modify the rule for this request's metadata
      Rule rule_copy(rule);
      for (Rule::iterator it = rule_copy.begin() ; it != rule_copy.end() ; ++it) {
        ReplaceFields(metadata_, &it->first);
      }

      QString content = original_content;
      if (ApplyExtractRule(rule_copy, &content)) {
        url_hop_ = true;
        QUrl url(content);
        qLog(Debug) << "Next url hop: " << url;
        QNetworkReply* reply = network_->get(QNetworkRequest(url));
        requests_[reply] = id;
        connect(reply, SIGNAL(finished()), SLOT(LyricsFetched()));
        return;
      }

      // Apply exclude rules
      foreach (const Rule& rule, exclude_rules_) {
        ApplyExcludeRule(rule, &lyrics);
      }

      if (!content.isEmpty()) {
        lyrics = content;
        break;
      }
    }
  } else {
    lyrics = original_content;
  }

  if (!lyrics.isEmpty()) {
    CollapsibleInfoPane::Data data;
    data.id_ = "ultimatelyrics/" + name_;
    data.title_ = tr("Lyrics from %1").arg(name_);
    data.type_ = CollapsibleInfoPane::Data::Type_Lyrics;
    data.relevance_ = relevance();

    if (QThread::currentThread() == QCoreApplication::instance()->thread()) {
      SongInfoTextView* editor = new SongInfoTextView;
      editor->SetHtml(lyrics);
      data.contents_ = editor;
    } else {
      UltimateLyricsLyric* editor = new UltimateLyricsLyric;
      editor->SetHtml(lyrics);
      data.content_object_ = editor;
    }

    emit InfoReady(id, data);
  }
  url_hop_ = false;
  emit Finished(id);
}

bool UltimateLyricsProvider::ApplyExtractRule(const Rule& rule, QString* content) const {
  foreach (const RuleItem& item, rule) {
    if (item.second.isNull()) {
      if (item.first.startsWith("http://") && item.second.isNull()) {
        *content = ExtractUrl(*content, rule);
        return true;
      } else {
        *content = ExtractXmlTag(*content, item.first);        
      }
    } else {
      *content = Extract(*content, item.first, item.second);
    }
  }
  return false;
}

QString UltimateLyricsProvider::ExtractUrl(const QString& source, const Rule& rule) {
  QString url;
  QString id;

  foreach(const RuleItem& item, rule) {
    if (item.first.startsWith("http://") && item.second.isNull())
      url = item.first;
    else
      id = Extract(source, item.first,item.second);
  }

  url.replace("{id}", id);

  return url;
}

QString UltimateLyricsProvider::ExtractXmlTag(const QString& source, const QString& tag) {
  QRegExp re("<(\\w+).*>"); // ಠ_ಠ
  if (re.indexIn(tag) == -1)
    return QString();

  return Extract(source, tag, "</" + re.cap(1) + ">");
}

QString UltimateLyricsProvider::Extract(const QString& source, const QString& begin, const QString& end) {
  int begin_idx = source.indexOf(begin);
  if (begin_idx == -1)
    return QString();
  begin_idx += begin.length();

  int end_idx = source.indexOf(end, begin_idx);
  if (end_idx == -1)
    return QString();

  return source.mid(begin_idx, end_idx - begin_idx - 1);
}

void UltimateLyricsProvider::ApplyExcludeRule(const Rule& rule, QString* content) const {
  foreach (const RuleItem& item, rule) {
    if (item.second.isNull()) {
      *content = ExcludeXmlTag(*content, item.first);
    } else {
      *content = Exclude(*content, item.first, item.second);
    }
  }
}

QString UltimateLyricsProvider::ExcludeXmlTag(const QString& source, const QString& tag) {
  QRegExp re("<(\\w+).*>"); // ಠ_ಠ
  if (re.indexIn(tag) == -1)
    return source;

  return Exclude(source, tag, "</" + re.cap(1) + ">");
}

QString UltimateLyricsProvider::Exclude(const QString& source, const QString& begin, const QString& end) {
  int begin_idx = source.indexOf(begin);
  if (begin_idx == -1)
    return source;

  int end_idx = source.indexOf(end, begin_idx + begin.length());
  if (end_idx == -1)
    return source;

  return source.left(begin_idx) + source.right(source.length() - end_idx - end.length());
}

QString UltimateLyricsProvider::FirstChar(const QString& text) {
  if (text.isEmpty())
    return QString();
  return text[0].toLower();
}

QString UltimateLyricsProvider::TitleCase(const QString& text) {
  if (text.length() == 0)
    return QString();

  QString ret = text;
  bool last_was_space = true;

  for (QString::iterator it = ret.begin() ; it != ret.end() ; ++it) {
    if (last_was_space) {
      *it = it->toUpper();
      last_was_space = false;
    } else if (it->isSpace()) {
      last_was_space = true;
    }
  }
  
  return ret;
}

void UltimateLyricsProvider::ReplaceField(const QString& tag, const QString& value,
                                          QString* text) const {
  if (!text->contains(tag))
    return;

  // Apply URL character replacement
  QString value_copy(value);
  foreach (const UrlFormat& format, url_formats_) {
    QRegExp re("[" + QRegExp::escape(format.first) + "]");
    value_copy.replace(re, format.second);
  }

  text->replace(tag, value_copy, Qt::CaseInsensitive);
}

void UltimateLyricsProvider::ReplaceFields(const Song& metadata, QString* text) const {
  ReplaceField("{artist}", metadata.artist().toLower(),          text);
  ReplaceField("{artist2}",NoSpace(metadata.artist().toLower()), text);
  ReplaceField("{album}",  metadata.album().toLower(),           text);
  ReplaceField("{album2}", NoSpace(metadata.album().toLower()),  text);
  ReplaceField("{title}",  metadata.title().toLower(),           text);
  ReplaceField("{Artist}", metadata.artist(),                    text);
  ReplaceField("{Album}",  metadata.album(),                     text);
  ReplaceField("{ARTIST}", metadata.artist().toUpper(),          text);
  ReplaceField("{year}",   metadata.PrettyYear(),                text);
  ReplaceField("{Title}",  metadata.title(),                     text);
  ReplaceField("{Title2}", TitleCase(metadata.title()),          text);
  ReplaceField("{a}",      FirstChar(metadata.artist()),         text);
  ReplaceField("{track}",  QString::number(metadata.track()),    text);
}

QString UltimateLyricsProvider::NoSpace(const QString& text) {
  QString ret(text);
  ret.remove(' ');
  return ret;
}
