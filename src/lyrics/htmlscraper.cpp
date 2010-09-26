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

#include "htmlscraper.h"
#include "core/networkaccessmanager.h"

#include <QNetworkReply>
#include <QTextCodec>

#include <boost/scoped_ptr.hpp>

const int HtmlScraper::kRedirectLimit = 5;


HtmlScraper::HtmlScraper(NetworkAccessManager* network, QObject* parent)
  : LyricProvider(network, parent)
{
}

LyricProvider::Result HtmlScraper::Search(const Song& metadata) const {
  LyricProvider::Result ret;

  // Get the text codec
  const QTextCodec* codec = QTextCodec::codecForName(charset_.toAscii().constData());
  if (!codec) {
    qWarning() << "Invalid codec" << charset_;
    return ret;
  }

  // Fill in fields in the URL
  QString url_text(url_);
  DoUrlReplace(&url_text, "{artist}", metadata.artist().toLower());
  DoUrlReplace(&url_text, "{album}",  metadata.album().toLower());
  DoUrlReplace(&url_text, "{title}",  metadata.title().toLower());
  DoUrlReplace(&url_text, "{Artist}", metadata.artist());
  DoUrlReplace(&url_text, "{Album}",  metadata.album());
  DoUrlReplace(&url_text, "{Title}",  metadata.title());
  DoUrlReplace(&url_text, "{Title2}", TitleCase(metadata.title()));
  DoUrlReplace(&url_text, "{a}",      FirstChar(metadata.artist()));

  QUrl url(url_text);

  // Fetch the URL, follow redirects
  boost::scoped_ptr<QNetworkReply> reply;

  for (int i=0 ; ; ++i) {
    if (i >= kRedirectLimit)
      return ret;

    qDebug() << "Fetching" << url;
    reply.reset(network_->GetBlocking(QUrl(url)));
    if (reply->error() != QNetworkReply::NoError)
      return ret;

    QVariant redirect_target = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (redirect_target.isValid()) {
      QUrl target = redirect_target.toUrl();
      if (target.scheme().isEmpty() || target.host().isEmpty()) {
        QString path = target.path();
        target = url;
        target.setPath(path);
      }
      url = target;
    } else
      break;
  }

  const QString original_content = codec->toUnicode(reply->readAll());

  // Check for invalid indicators
  foreach (const QString& indicator, invalid_indicators_) {
    if (original_content.contains(indicator))
      return ret;
  }

  // Apply extract rules
  foreach (const Rule& rule, extract_rules_) {
    QString content = original_content;
    ApplyExtractRule(rule, &content);

    if (!content.isEmpty())
      ret.content = content;
  }

  // Apply exclude rules
  foreach (const Rule& rule, exclude_rules_) {
    ApplyExcludeRule(rule, &ret.content);
  }

  if (!ret.content.isEmpty())
    ret.valid = true;
  return ret;
}

void HtmlScraper::ApplyExtractRule(const Rule& rule, QString* content) const {
  foreach (const RuleItem& item, rule) {
    if (item.second.isNull()) {
      *content = ExtractXmlTag(*content, item.first);
    } else {
      *content = Extract(*content, item.first, item.second);
    }
  }
}

QString HtmlScraper::ExtractXmlTag(const QString& source, const QString& tag) {
  QRegExp re("<(\\w+).*>");
  if (re.indexIn(tag) == -1)
    return QString();

  return Extract(source, tag, "</" + re.cap(1) + ">");
}

QString HtmlScraper::Extract(const QString& source, const QString& begin, const QString& end) {
  int begin_idx = source.indexOf(begin);
  if (begin_idx == -1)
    return QString();
  begin_idx += begin.length();

  int end_idx = source.indexOf(end, begin_idx);
  if (end_idx == -1)
    return QString();

  return source.mid(begin_idx, end_idx - begin_idx - 1);
}

void HtmlScraper::ApplyExcludeRule(const Rule& rule, QString* content) const {
  foreach (const RuleItem& item, rule) {
    if (item.second.isNull()) {
      *content = ExcludeXmlTag(*content, item.first);
    } else {
      *content = Exclude(*content, item.first, item.second);
    }
  }
}

QString HtmlScraper::ExcludeXmlTag(const QString& source, const QString& tag) {
  QRegExp re("<(\\w+).*>");
  if (re.indexIn(tag) == -1)
    return source;

  return Exclude(source, tag, "</" + re.cap(1) + ">");
}

QString HtmlScraper::Exclude(const QString& source, const QString& begin, const QString& end) {
  int begin_idx = source.indexOf(begin);
  if (begin_idx == -1)
    return source;

  int end_idx = source.indexOf(end, begin_idx + begin.length());
  if (end_idx == -1)
    return source;

  return source.left(begin_idx) + source.right(source.length() - end_idx - end.length());
}

QString HtmlScraper::FirstChar(const QString& text) {
  if (text.isEmpty())
    return QString();
  return text[0].toLower();
}

QString HtmlScraper::TitleCase(const QString& text) {
  if (text.length() == 0)
    return QString();
  if (text.length() == 1)
    return text[0].toUpper();
  return text[0].toUpper() + text.right(text.length() - 1).toLower();
}

void HtmlScraper::DoUrlReplace(QString* url, const QString& tag,
                               const QString& value) const {
  if (!url->contains(tag))
    return;

  // Apply URL character replacement
  QString value_copy(value);
  foreach (const UrlFormat& format, url_formats_) {
    QRegExp re("[" + QRegExp::escape(format.first) + "]");
    value_copy.replace(re, format.second);
  }

  url->replace(tag, value_copy, Qt::CaseInsensitive);
}
