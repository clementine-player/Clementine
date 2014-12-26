/* This file is part of Clementine.
   Copyright 2011-2012, David Sansome <me@davidsansome.com>
   Copyright 2011, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2012, Arnaud Bienner <arnaud.bienner@gmail.com>
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

#include "amazoncoverprovider.h"

#include <QDateTime>
#include <QNetworkReply>
#include <QStringList>
#include <QXmlStreamReader>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/utilities.h"

// Amazon has a web crawler that looks for access keys in public source code,
// so we apply some sophisticated encryption to these keys.
const char* AmazonCoverProvider::kAccessKeyB64 = "QUtJQUlRSDI2UlZNNlVaNFdBNlE=";
const char* AmazonCoverProvider::kSecretAccessKeyB64 =
    "ZTQ2UGczM0JRNytDajd4MWR6eFNvODVFd2tpdi9FbGVCcUZjMkVmMQ==";
const char* AmazonCoverProvider::kUrl = "http://ecs.amazonaws.com/onca/xml";
const char* AmazonCoverProvider::kAssociateTag = "clemmusiplay-20";

AmazonCoverProvider::AmazonCoverProvider(QObject* parent)
    : CoverProvider("Amazon", parent),
      network_(new NetworkAccessManager(this)) {}

bool AmazonCoverProvider::StartSearch(const QString& artist,
                                      const QString& album, int id) {
  typedef QPair<QString, QString> Arg;
  typedef QList<Arg> ArgList;

  typedef QPair<QByteArray, QByteArray> EncodedArg;
  typedef QList<EncodedArg> EncodedArgList;

  // Must be sorted by parameter name
  ArgList args =
      ArgList() << Arg("AWSAccessKeyId", QByteArray::fromBase64(kAccessKeyB64))
                << Arg("AssociateTag", kAssociateTag)
                << Arg("Keywords", artist + " " + album)
                << Arg("Operation", "ItemSearch")
                << Arg("ResponseGroup", "Images") << Arg("SearchIndex", "All")
                << Arg("Service", "AWSECommerceService")
                << Arg("Timestamp", QDateTime::currentDateTime().toString(
                                        "yyyy-MM-ddThh:mm:ss.zzzZ"))
                << Arg("Version", "2009-11-01");

  EncodedArgList encoded_args;
  QStringList query_items;

  // Encode the arguments
  for (const Arg& arg : args) {
    EncodedArg encoded_arg(QUrl::toPercentEncoding(arg.first),
                           QUrl::toPercentEncoding(arg.second));
    encoded_args << encoded_arg;
    query_items << QString(encoded_arg.first + "=" + encoded_arg.second);
  }

  // Sign the request
  QUrl url(kUrl);

  const QByteArray data_to_sign =
      QString("GET\n%1\n%2\n%3")
          .arg(url.host(), url.path(), query_items.join("&"))
          .toAscii();
  const QByteArray signature(Utilities::HmacSha256(
      QByteArray::fromBase64(kSecretAccessKeyB64), data_to_sign));

  // Add the signature to the request
  encoded_args << EncodedArg("Signature",
                             QUrl::toPercentEncoding(signature.toBase64()));
  url.setEncodedQueryItems(encoded_args);

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(QueryFinished(QNetworkReply*, int)), reply, id);

  return true;
}

void AmazonCoverProvider::QueryFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();

  CoverSearchResults results;

  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    reader.readNext();
    if (reader.tokenType() == QXmlStreamReader::StartElement &&
        reader.name() == "Item") {
      ReadItem(&reader, &results);
    }
  }

  emit SearchFinished(id, results);
}

void AmazonCoverProvider::ReadItem(QXmlStreamReader* reader,
                                   CoverSearchResults* results) {
  while (!reader->atEnd()) {
    switch (reader->readNext()) {
      case QXmlStreamReader::StartElement:
        if (reader->name() == "LargeImage") {
          ReadLargeImage(reader, results);
        } else {
          reader->skipCurrentElement();
        }
        break;

      case QXmlStreamReader::EndElement:
        return;

      default:
        break;
    }
  }
}

void AmazonCoverProvider::ReadLargeImage(QXmlStreamReader* reader,
                                         CoverSearchResults* results) {
  while (!reader->atEnd()) {
    switch (reader->readNext()) {
      case QXmlStreamReader::StartElement:
        if (reader->name() == "URL") {
          CoverSearchResult result;
          result.image_url = QUrl(reader->readElementText());
          results->append(result);
        } else {
          reader->skipCurrentElement();
        }
        break;

      case QXmlStreamReader::EndElement:
        return;

      default:
        break;
    }
  }
}
