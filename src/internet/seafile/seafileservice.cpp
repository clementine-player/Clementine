/* This file is part of Clementine.
   Copyright 2014, Chocobozzz <djidane14ff@hotmail.fr>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, David Sansome <me@davidsansome.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "seafileservice.h"

#include <cmath>

#include <qjson/parser.h>
#include <QTimer>

#include "core/application.h"
#include "core/player.h"
#include "core/waitforsignal.h"
#include "internet/seafile/seafileurlhandler.h"
#include "library/librarybackend.h"
#include "internet/core/oauthenticator.h"

const char* SeafileService::kServiceName = "Seafile";
const char* SeafileService::kSettingsGroup = "Seafile";

namespace {

static const char* kAuthTokenUrl = "/api2/auth-token/";

static const char* kFolderItemsUrl = "/api2/repos/%1/dir/";
static const char* kListReposUrl = "/api2/repos/";

static const char* kFileUrl = "/api2/repos/%1/file/";
static const char* kFileContentUrl = "/api2/repos/%1/file/detail/";

static const int kMaxTries = 10;
}  // namespace

SeafileService::SeafileService(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kSettingsGroup,
                       QIcon(":/providers/seafile.png"),
                       SettingsDialog::Page_Seafile) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  access_token_ = s.value("access_token").toString();
  server_ = s.value("server").toString();

  QByteArray tree_bytes = s.value("tree").toByteArray();

  if (!tree_bytes.isEmpty()) {
    QDataStream stream(&tree_bytes, QIODevice::ReadOnly);
    stream >> tree_;
  }

  app->player()->RegisterUrlHandler(new SeafileUrlHandler(this, this));

  connect(&tree_, SIGNAL(ToAdd(QString, QString, SeafileTree::Entry)), this,
          SLOT(AddEntry(QString, QString, SeafileTree::Entry)));
  connect(&tree_, SIGNAL(ToDelete(QString, QString, SeafileTree::Entry)), this,
          SLOT(DeleteEntry(QString, QString, SeafileTree::Entry)));
  connect(&tree_, SIGNAL(ToUpdate(QString, QString, SeafileTree::Entry)), this,
          SLOT(UpdateEntry(QString, QString, SeafileTree::Entry)));
}

bool SeafileService::has_credentials() const {
  return !access_token_.isEmpty();
}

void SeafileService::AddAuthorizationHeader(QNetworkRequest* request) const {
  request->setRawHeader("Authorization",
                        QString("Token %1").arg(access_token_).toAscii());
}

void SeafileService::ForgetCredentials() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.remove("access_token");
  s.remove("tree");
  access_token_.clear();
  tree_.Clear();

  server_.clear();
}

bool SeafileService::GetToken(const QString& mail, const QString& password,
                              const QString& server) {
  QUrl url(server + kAuthTokenUrl);
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);

  url.addQueryItem("username", mail);
  url.addQueryItem("password", password);

  QNetworkReply* reply = network_->post(request, url.encodedQuery());
  WaitForSignal(reply, SIGNAL(finished()));

  if (!CheckReply(&reply)) {
    qLog(Warning) << "Something wrong with the reply... (GetToken)";
    return false;
  }

  reply->deleteLater();

  QJson::Parser parser;
  QVariantMap response = parser.parse(reply->readAll()).toMap();

  // Because the server responds "token"
  access_token_ = response["token"].toString().replace("\"", "");

  if (access_token_.isEmpty()) {
    return false;
  }

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("access_token", access_token_);

  server_ = server;

  emit Connected();

  return true;
}

void SeafileService::GetLibraries() {
  QUrl url(server_ + kListReposUrl);
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->get(request);

  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(GetLibrariesFinished(QNetworkReply*)), reply);
}

void SeafileService::GetLibrariesFinished(QNetworkReply* reply) {
  if (!CheckReply(&reply)) {
    qLog(Warning) << "Something wrong with the reply... (GetLibraries)";
    return;
  }

  reply->deleteLater();

  // key : id, value : name
  QMap<QString, QString> libraries;
  QByteArray data = reply->readAll();
  QJson::Parser parser;
  QList<QVariant> repos = parser.parse(data).toList();

  for (int i = 0; i < repos.size(); ++i) {
    QVariantMap repo = repos.at(i).toMap();
    QString repo_name = repo["name"].toString(),
            repo_id = repo["id"].toString();

    // One library can appear several times and we don't add encrypted libraries
    // (not supported yet)
    if (!libraries.contains(repo_id) && !repo["encrypted"].toBool()) {
      libraries.insert(repo_id, repo_name);
    }
  }

  emit GetLibrariesFinishedSignal(libraries);
}

void SeafileService::ChangeLibrary(const QString& new_library) {
  // Every other libraries have to be destroyed from the tree
  if (new_library != "all") {
    for (SeafileTree::TreeItem* library : tree_.libraries()) {
      if (new_library != library->entry().id()) {
        DeleteEntry(library->entry().id(), "/", library->entry());
      }
    }
  }

  UpdateLibraries();
}

void SeafileService::Connect() {
  if (has_credentials()) {
    UpdateLibraries();
  } else {
    ShowSettingsDialog();
  }
}

void SeafileService::UpdateLibraries() {
  connect(this, SIGNAL(GetLibrariesFinishedSignal(QMap<QString, QString>)),
          this, SLOT(UpdateLibrariesInProgress(QMap<QString, QString>)));

  GetLibraries();
}

void SeafileService::UpdateLibrariesInProgress(
    const QMap<QString, QString>& libraries) {
  disconnect(this, SIGNAL(GetLibrariesFinishedSignal(QMap<QString, QString>)),
             this, SLOT(UpdateLibrariesInProgress(QMap<QString, QString>)));

  QSettings s;
  s.beginGroup(kSettingsGroup);
  QString library_to_update = s.value("library").toString();

  // If the library doesn't change, we don't need to update
  if (!library_updated_.isNull() && library_updated_ == library_to_update) {
    return;
  }

  library_updated_ = library_to_update;

  if (library_to_update == "none") {
    return;
  }

  QMapIterator<QString, QString> library(libraries);
  while (library.hasNext()) {
    library.next();

    // Need to check this library ?
    if (library_to_update == "all" || library.key() == library_to_update) {
      FetchAndCheckFolderItems(
          SeafileTree::Entry(library.value(), library.key(),
                             SeafileTree::Entry::LIBRARY),
          "/");
    // If not, we can destroy the library from the tree
    } else {
      // If the library was not in the tree, it's not a problem because
      // DeleteEntry won't do anything
      DeleteEntry(library.key(), "/",
                  SeafileTree::Entry(library.value(), library.key(),
                                     SeafileTree::Entry::LIBRARY));
    }
  }
}

QNetworkReply* SeafileService::PrepareFetchFolderItems(const QString& library,
                                                       const QString& path) {
  QUrl url(server_ + QString(kFolderItemsUrl).arg(library));
  url.addQueryItem("p", path);

  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->get(request);

  return reply;
}

void SeafileService::FetchAndCheckFolderItems(const SeafileTree::Entry& library,
                                              const QString& path) {
  QNetworkReply* reply = PrepareFetchFolderItems(library.id(), path);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(FetchAndCheckFolderItemsFinished(
                 QNetworkReply*, SeafileTree::Entry, QString)),
             reply, library, path);
}

void SeafileService::FetchAndCheckFolderItemsFinished(
    QNetworkReply* reply, const SeafileTree::Entry& library,
    const QString& path) {
  if (!CheckReply(&reply)) {
    qLog(Warning)
        << "Something wrong with the reply... (FetchFolderItemsToList)";
    return;
  }

  reply->deleteLater();

  QByteArray data = reply->readAll();

  QJson::Parser parser;
  QList<QVariant> variant_entries = parser.parse(data).toList();

  SeafileTree::Entries entries;
  for (const QVariant& e : variant_entries) {
    QVariantMap entry = e.toMap();
    SeafileTree::Entry::Type entry_type =
        SeafileTree::Entry::StringToType(entry["type"].toString());
    QString entry_name = entry["name"].toString();

    // We just want libraries/directories and files which could be songs.
    if (entry_type == SeafileTree::Entry::NONE) {
      qLog(Warning) << "Type entry unknown for this entry";
    } else if (entry_type == SeafileTree::Entry::FILE &&
               GuessMimeTypeForFile(entry_name).isNull()) {
      continue;
    }

    entries.append(
        SeafileTree::Entry(entry_name, entry["id"].toString(), entry_type));
  }

  tree_.CheckEntries(entries, library, path);
}

void SeafileService::AddRecursivelyFolderItems(const QString& library,
                                               const QString& path) {
  QNetworkReply* reply = PrepareFetchFolderItems(library, path);
  NewClosure(
      reply, SIGNAL(finished()), this,
      SLOT(AddRecursivelyFolderItemsFinished(QNetworkReply*, QString, QString)),
      reply, library, path);
}

void SeafileService::AddRecursivelyFolderItemsFinished(QNetworkReply* reply,
                                                       const QString& library,
                                                       const QString& path) {
  if (!CheckReply(&reply)) {
    qLog(Warning) << "Something wrong with the reply... (FetchFolderItems)";
    return;
  }

  reply->deleteLater();

  QByteArray data = reply->readAll();
  QJson::Parser parser;
  QList<QVariant> entries = parser.parse(data).toList();

  for (const QVariant& e : entries) {
    QVariantMap entry_map = e.toMap();
    SeafileTree::Entry::Type entry_type =
        SeafileTree::Entry::StringToType(entry_map["type"].toString());
    QString entry_name = entry_map["name"].toString();

    // We just want libraries/directories and files which could be songs.
    if (entry_type == SeafileTree::Entry::NONE) {
      qLog(Warning) << "Type entry unknown for this entry";
    } else if (entry_type == SeafileTree::Entry::FILE &&
               GuessMimeTypeForFile(entry_name).isNull()) {
      continue;
    }

    SeafileTree::Entry entry(entry_name, entry_map["id"].toString(),
                             entry_type);

    // If AddEntry was not successful we stop
    // It could happen when the user changes the library to update while an
    // update was in progress
    if (!tree_.AddEntry(library, path, entry)) {
      return;
    }

    if (entry.is_dir()) {
      AddRecursivelyFolderItems(library, path + entry.name() + "/");
    } else {
      MaybeAddFileEntry(entry.name(), library, path);
    }
  }
}

QNetworkReply* SeafileService::PrepareFetchContentForFile(
    const QString& library, const QString& filepath) {
  QUrl content_url(server_ + QString(kFileContentUrl).arg(library));
  content_url.addQueryItem("p", filepath);

  QNetworkRequest request(content_url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->get(request);

  return reply;
}

void SeafileService::MaybeAddFileEntry(const QString& entry_name,
                                       const QString& library,
                                       const QString& path) {
  QString mime_type = GuessMimeTypeForFile(entry_name);

  if (mime_type.isNull()) return;

  // Get the details of the entry
  QNetworkReply* reply = PrepareFetchContentForFile(library, path + entry_name);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(MaybeAddFileEntryInProgress(QNetworkReply*, QString, QString,
                                              QString)),
             reply, library, path, mime_type);
}

void SeafileService::MaybeAddFileEntryInProgress(QNetworkReply* reply,
                                                 const QString& library,
                                                 const QString& path,
                                                 const QString& mime_type) {
  if (!CheckReply(&reply)) {
    qLog(Warning) << "Something wrong with the reply... (MaybeAddFileEntry)";
    return;
  }

  reply->deleteLater();

  QByteArray data = reply->readAll();

  QJson::Parser parser;
  QVariantMap entry_detail_map = parser.parse(data).toMap();

  QUrl url;
  url.setScheme("seafile");
  url.setPath("/" + library + path + entry_detail_map["name"].toString());

  Song song;
  song.set_url(url);
  song.set_ctime(0);
  song.set_mtime(entry_detail_map["mtime"].toInt());
  song.set_filesize(entry_detail_map["size"].toInt());
  song.set_title(entry_detail_map["name"].toString());

  // Get the download url of the entry
  reply = PrepareFetchContentUrlForFile(
      library, path + entry_detail_map["name"].toString());
  NewClosure(
      reply, SIGNAL(finished()), this,
      SLOT(FetchContentUrlForFileFinished(QNetworkReply*, Song, QString)),
      reply, song, mime_type);
}

QNetworkReply* SeafileService::PrepareFetchContentUrlForFile(
    const QString& library, const QString& filepath) {
  QUrl content_url(server_ + QString(kFileUrl).arg(library));
  content_url.addQueryItem("p", filepath);

  QNetworkRequest request(content_url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->get(request);

  return reply;
}

void SeafileService::FetchContentUrlForFileFinished(QNetworkReply* reply,
                                                    const Song& song,
                                                    const QString& mime_type) {
  if (!CheckReply(&reply)) {
    qLog(Warning)
        << "Something wrong with the reply... (FetchContentUrlForFile)";
    return;
  }

  reply->deleteLater();

  // Because server response is "http://..."
  QString real_url = QString(reply->readAll()).replace("\"", "");

  MaybeAddFileToDatabase(song, mime_type, QUrl(real_url),
                         QString("Token %1").arg(access_token_));
}

QUrl SeafileService::GetStreamingUrlFromSongId(const QString& library,
                                               const QString& filepath) {
  QNetworkReply* reply = PrepareFetchContentUrlForFile(library, filepath);
  WaitForSignal(reply, SIGNAL(finished()));

  if (!CheckReply(&reply)) {
    qLog(Warning)
        << "Something wrong with the reply... (GetStreamingUrlFromSongId)";
    return QUrl("");
  }
  reply->deleteLater();

  QString response = QString(reply->readAll()).replace("\"", "");

  return QUrl(response);
}

void SeafileService::AddEntry(const QString& library, const QString& path,
                              const SeafileTree::Entry& entry) {
  if (entry.is_library()) {
    tree_.AddLibrary(entry.name(), entry.id());
    AddRecursivelyFolderItems(library, "/");
  } else {
    // If AddEntry was not successful we stop
    // It could happen when the user changes the library to update while an
    // update was in progress
    if (!tree_.AddEntry(library, path, entry)) {
      return;
    }

    if (entry.is_file()) {
      MaybeAddFileEntry(entry.name(), library, path);
    } else {
      AddRecursivelyFolderItems(library, path + entry.name() + "/");
    }
  }
}

void SeafileService::UpdateEntry(const QString& library, const QString& path,
                                 const SeafileTree::Entry& entry) {
  if (entry.is_file()) {
    DeleteEntry(library, path, entry);
    AddEntry(library, path, entry);
  } else {
    QString entry_path = path;

    if (entry.is_dir()) {
      entry_path += entry.name() + "/";
    }

    FetchAndCheckFolderItems(
        SeafileTree::Entry("", library, SeafileTree::Entry::LIBRARY),
        entry_path);
  }
}

void SeafileService::DeleteEntry(const QString& library, const QString& path,
                                 const SeafileTree::Entry& entry) {
  // For the QPair -> 1 : path, 2 : entry
  QList<QPair<QString, SeafileTree::Entry>> files_to_delete;
  if (entry.is_library()) {
    SeafileTree::TreeItem* item = tree_.FindLibrary(library);
    files_to_delete = tree_.GetRecursiveFilesOfDir("/", item);
    tree_.DeleteLibrary(library);
  } else {
    if (entry.is_dir()) {
      SeafileTree::TreeItem* item =
          tree_.FindFromAbsolutePath(library, path + entry.name() + "/");
      files_to_delete =
          tree_.GetRecursiveFilesOfDir(path + entry.name() + "/", item);
    } else {
      files_to_delete.append(qMakePair(path, entry));
    }

    if (!tree_.DeleteEntry(library, path, entry)) {
      return;
    }
  }

  // Delete songs from the library of Clementine
  for (const QPair<QString, SeafileTree::Entry>& file_to_delete :
       files_to_delete) {
    if (!GuessMimeTypeForFile(file_to_delete.second.name()).isEmpty()) {
      QUrl song_url("seafile:/" + library + file_to_delete.first +
                    file_to_delete.second.name());
      Song song = library_backend_->GetSongByUrl(song_url);

      if (song.is_valid()) {
        library_backend_->DeleteSongs(SongList() << song);
      } else {
        qLog(Warning) << "Can't delete song from the Clementine's library : "
                      << song_url;
      }
    }
  }
}

bool SeafileService::CheckReply(QNetworkReply** reply, int tries) {
  if (!(*reply)) {
    return false;
  } else if (tries > kMaxTries) {
    (*reply)->deleteLater();
    return false;
  }

  QVariant status_code_variant =
      (*reply)->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  if (status_code_variant.isValid()) {
    int status_code = status_code_variant.toInt();

    if (status_code == NO_ERROR) {
      return true;
    } else if (status_code == TOO_MANY_REQUESTS) {
      qLog(Debug) << "Too many requests, wait...";

      int seconds_to_wait;
      if ((*reply)->hasRawHeader("X-Throttle-Wait-Seconds")) {
        seconds_to_wait =
            ((*reply)->rawHeader("X-Throttle-Wait-Seconds").toInt() + 1) * 1000;
      } else {
        seconds_to_wait = std::pow(tries, 2) * 1000;
      }

      QTimer timer;
      timer.start(seconds_to_wait);
      WaitForSignal(&timer, SIGNAL(timeout()));

      (*reply)->deleteLater();

      // We execute the reply again
      *reply = network_->get((*reply)->request());
      WaitForSignal(*reply, SIGNAL(finished()));

      return CheckReply(reply, ++tries);
    }
  }

  // Unknown, 404 ...
  (*reply)->deleteLater();
  qLog(Warning) << "Error for reply : " << status_code_variant.toInt();
  return false;
}

SeafileService::~SeafileService() {
  // Save the tree !
  QSettings s;
  s.beginGroup(kSettingsGroup);

  QByteArray tree_byte;
  QDataStream stream(&tree_byte, QIODevice::WriteOnly);
  stream << tree_;

  s.setValue("tree", tree_byte);
}
