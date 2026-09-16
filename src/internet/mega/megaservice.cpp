/* This file is part of Clementine.
   Copyright 2026

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

#include "megaservice.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>

#include <megaapi.h>

#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "internet/core/internetmodel.h"
#include "internet/mega/megaurlhandler.h"
#include "library/librarybackend.h"
#include "ui/iconloader.h"

namespace {

static const char* kServiceId = "mega";

// Path for the MEGA SDK local state cache.
QString MegaCachePath() {
  return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
         "/mega-sdk";
}

}  // namespace

const char* MegaService::kServiceName = "Mega";
const char* MegaService::kSettingsGroup = "Mega";

// MEGA SDK application key. Provide your own via -DMEGA_API_KEY="yourkey"
// at CMake configure time. Register at https://mega.nz/sdk to obtain one.
#ifndef MEGA_API_KEY
#define MEGA_API_KEY ""
#endif
const char* MegaService::kAppKey = MEGA_API_KEY;

MegaService::MegaService(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kServiceId,
                       IconLoader::Load("folder-cloud", IconLoader::Base),
                       SettingsDialog::Page_Mega),
      streaming_port_(0),
      logged_in_(false) {
  // Create the SDK cache directory.
  QDir().mkpath(MegaCachePath());

  // Initialize the MEGA SDK API.
  mega_api_.reset(new mega::MegaApi(
      kAppKey, MegaCachePath().toUtf8().constData(), "Clementine"));

  app->player()->RegisterUrlHandler(new MegaUrlHandler(this, this));
}

MegaService::~MegaService() {
  // Stop the streaming server if running.
  if (streaming_port_ > 0) {
    mega_api_->httpServerStop();
  }
}

bool MegaService::has_credentials() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  // Either we have a cached session or email+password.
  if (!s.value("session").toString().isEmpty()) return true;
  return !s.value("email").toString().trimmed().isEmpty() &&
         !s.value("password").toString().isEmpty();
}

void MegaService::Connect() {
  if (!has_credentials()) {
    ShowConfig();
    return;
  }

  if (DoLogin()) {
    if (!FetchNodes()) return;
    emit Connected();

    // Only auto-scan if folders have been previously selected.
    QStringList selected = GetSelectedFolderHandles();
    if (!selected.isEmpty()) {
      ScanSelectedFolders();
    }
  }
}

void MegaService::ForgetCredentials() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.remove("email");
  s.remove("password");
  s.remove("session");
  s.remove("cached_folder_tree");

  // Stop streaming server.
  {
    QMutexLocker lock(&mutex_);
    if (streaming_port_ > 0) {
      mega_api_->httpServerStop();
      streaming_port_ = 0;
    }
  }

  // Logout from SDK (local only, don't invalidate server session).
  mega::SynchronousRequestListener listener;
  mega_api_->localLogout(&listener);
  listener.wait();

  logged_in_ = false;
}

void MegaService::DoFullRescan() {
  library_backend_->DeleteAll();
  if (!logged_in_) {
    Connect();
  } else {
    ScanSelectedFolders();
  }
}

bool MegaService::DoLogin() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  // Try resuming a cached session first.
  QString cached_session = s.value("session").toString();
  if (!cached_session.isEmpty()) {
    qLog(Debug) << "Mega: resuming cached session";

    mega::SynchronousRequestListener listener;
    mega_api_->fastLogin(cached_session.toUtf8().constData(), &listener);
    listener.wait();

    if (listener.getError()->getErrorCode() == mega::MegaError::API_OK) {
      logged_in_ = true;
      qLog(Debug) << "Mega: fast login succeeded";
      return true;
    }

    qLog(Warning) << "Mega: fast login failed (error"
                  << listener.getError()->getErrorCode()
                  << listener.getError()->getErrorString()
                  << "), falling back to email/password";
    s.remove("session");
  }

  // Fall back to email + password login.
  const QString email = s.value("email").toString().trimmed().toLower();
  const QString password = s.value("password").toString();

  if (email.isEmpty() || password.isEmpty()) {
    qLog(Error) << "Mega: no credentials available";
    return false;
  }

  qLog(Debug) << "Mega: logging in with email/password";

  mega::SynchronousRequestListener listener;
  mega_api_->login(email.toUtf8().constData(),
                   password.toUtf8().constData(), &listener);
  listener.wait();

  if (listener.getError()->getErrorCode() != mega::MegaError::API_OK) {
    qLog(Error) << "Mega: login failed -"
                << listener.getError()->getErrorString();
    return false;
  }

  // Save the session for future fast logins, then remove the password
  // from settings so it is not persisted in plaintext on disk.
  std::unique_ptr<char[]> session(mega_api_->dumpSession());
  if (session) {
    s.setValue("session", QString::fromUtf8(session.get()));
    s.remove("password");
  }

  logged_in_ = true;
  qLog(Debug) << "Mega: login succeeded";
  return true;
}

bool MegaService::FetchNodes() {
  qLog(Debug) << "Mega: fetching nodes...";

  mega::SynchronousRequestListener listener;
  mega_api_->fetchNodes(&listener);
  listener.wait();

  if (listener.getError()->getErrorCode() != mega::MegaError::API_OK) {
    qLog(Error) << "Mega: fetchNodes failed -"
                << listener.getError()->getErrorString();
    return false;
  }

  qLog(Debug) << "Mega: nodes fetched successfully";
  return true;
}

void MegaService::ScanSelectedFolders() {
  QStringList selected = GetSelectedFolderHandles();
  if (selected.isEmpty()) {
    qLog(Debug) << "Mega: no folders selected for scanning";
    return;
  }

  if (!EnsureStreamingServer()) {
    qLog(Error) << "Mega: failed to start streaming server";
    return;
  }

  qLog(Debug) << "Mega: scanning selected folders...";

  for (const QString& handle_b64 : selected) {
    mega::MegaHandle handle = mega::MegaApi::base64ToHandle(
        handle_b64.toUtf8().constData());
    std::unique_ptr<mega::MegaNode> node(
        mega_api_->getNodeByHandle(handle));
    if (!node || !node->isFolder()) {
      qLog(Warning) << "Mega: selected folder not found:" << handle_b64;
      continue;
    }

    std::unique_ptr<char[]> path(mega_api_->getNodePath(node.get()));
    QString folder_path = path ? QString::fromUtf8(path.get()) : "";

    qLog(Debug) << "Mega: scanning folder" << folder_path;
    IndexNodeRecursive(node.get(), folder_path);
  }

  qLog(Debug) << "Mega: scanning complete";
}

QList<MegaFolderInfo> MegaService::GetFolderTree() {
  QList<MegaFolderInfo> result;
  std::unique_ptr<mega::MegaNode> root(mega_api_->getRootNode());
  if (!root) return result;
  BuildFolderTree(root.get(), result);
  return result;
}

void MegaService::BuildFolderTree(mega::MegaNode* node,
                                   QList<MegaFolderInfo>& list) {
  std::unique_ptr<mega::MegaNodeList> children(
      mega_api_->getChildren(node));
  if (!children) return;

  for (int i = 0; i < children->size(); ++i) {
    mega::MegaNode* child = children->get(i);
    if (!child->isFolder()) continue;

    const char* name = child->getName();
    if (!name) continue;

    MegaFolderInfo info;
    info.name = QString::fromUtf8(name);
    mega::MegaHandle handle = child->getHandle();
    std::unique_ptr<char[]> handle_b64(
        mega::MegaApi::handleToBase64(handle));
    info.handle_b64 = QString::fromUtf8(handle_b64.get());
    BuildFolderTree(child, info.children);
    list.append(info);
  }
}

QSet<QString> MegaService::GetFoldersWithAudio() {
  QSet<QString> result;
  std::unique_ptr<mega::MegaNode> root(mega_api_->getRootNode());
  if (!root) return result;
  CollectFoldersWithAudio(root.get(), result);
  return result;
}

bool MegaService::CollectFoldersWithAudio(mega::MegaNode* node,
                                           QSet<QString>& out) {
  std::unique_ptr<mega::MegaNodeList> children(
      mega_api_->getChildren(node));
  if (!children) return false;

  bool has_audio = false;
  for (int i = 0; i < children->size(); ++i) {
    mega::MegaNode* child = children->get(i);
    const char* name = child->getName();
    if (!name) continue;

    if (child->isFile()) {
      QString filename = QString::fromUtf8(name);
      // Use the same MIME type detection as the indexing path.
      if (!GuessMimeTypeForFile(filename).isEmpty()) {
        has_audio = true;
      }
    } else if (child->isFolder()) {
      if (CollectFoldersWithAudio(child, out)) {
        has_audio = true;
      }
    }
  }

  if (has_audio && node->isFolder()) {
    mega::MegaHandle handle = node->getHandle();
    std::unique_ptr<char[]> handle_b64(
        mega::MegaApi::handleToBase64(handle));
    out.insert(QString::fromUtf8(handle_b64.get()));
  }

  return has_audio;
}

void MegaService::ResetDatabase() {
  library_backend_->DeleteAll();

  // Clear selected folders and cached tree.
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.remove("selected_folders");
  s.remove("cached_folder_tree");

  qLog(Debug) << "Mega: database reset and folder selection cleared";
}

QStringList MegaService::GetSelectedFolderHandles() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  return s.value("selected_folders").toStringList();
}

void MegaService::SetSelectedFolderHandles(const QStringList& handles) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("selected_folders", handles);
}

namespace {

QJsonArray FolderTreeToJson(const QList<MegaFolderInfo>& folders) {
  QJsonArray arr;
  for (const MegaFolderInfo& f : folders) {
    QJsonObject obj;
    obj["name"] = f.name;
    obj["handle"] = f.handle_b64;
    if (!f.children.isEmpty()) {
      obj["children"] = FolderTreeToJson(f.children);
    }
    arr.append(obj);
  }
  return arr;
}

QList<MegaFolderInfo> JsonToFolderTree(const QJsonArray& arr) {
  QList<MegaFolderInfo> result;
  for (const QJsonValue& v : arr) {
    QJsonObject obj = v.toObject();
    MegaFolderInfo info;
    info.name = obj["name"].toString();
    info.handle_b64 = obj["handle"].toString();
    if (obj.contains("children")) {
      info.children = JsonToFolderTree(obj["children"].toArray());
    }
    result.append(info);
  }
  return result;
}

}  // namespace

QList<MegaFolderInfo> MegaService::GetCachedFolderTree() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  QByteArray data = s.value("cached_folder_tree").toByteArray();
  if (data.isEmpty()) return QList<MegaFolderInfo>();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (!doc.isArray()) return QList<MegaFolderInfo>();

  return JsonToFolderTree(doc.array());
}

void MegaService::SaveCachedFolderTree(const QList<MegaFolderInfo>& folders) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  QJsonDocument doc(FolderTreeToJson(folders));
  s.setValue("cached_folder_tree", doc.toJson(QJsonDocument::Compact));
}

void MegaService::ClearCachedFolderTree() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.remove("cached_folder_tree");
}

void MegaService::IndexNodeRecursive(mega::MegaNode* node,
                                     const QString& path) {
  std::unique_ptr<mega::MegaNodeList> children(
      mega_api_->getChildren(node));
  if (!children) return;

  for (int i = 0; i < children->size(); ++i) {
    mega::MegaNode* child = children->get(i);
    const char* name = child->getName();
    if (!name) continue;

    QString child_name = QString::fromUtf8(name);
    QString child_path = path + "/" + child_name;

    if (child->isFolder()) {
      IndexNodeRecursive(child, child_path);
    } else if (child->isFile()) {
      ProcessFileNode(child, child_path);
    }
  }
}

void MegaService::ProcessFileNode(mega::MegaNode* node, const QString& path) {
  const char* name = node->getName();
  if (!name) return;

  QString filename = QString::fromUtf8(name);
  int64_t size = node->getSize();

  // Build the mega:// URL using the node handle.
  QUrl url;
  url.setScheme("mega");
  // Encode handle as base64 string.
  mega::MegaHandle handle = node->getHandle();
  std::unique_ptr<char[]> handle_b64(mega::MegaApi::handleToBase64(handle));
  url.setPath("/" + QString::fromUtf8(handle_b64.get()));

  QString mime_type = GuessMimeTypeForFile(filename);
  if (!ShouldIndexFile(url, mime_type)) return;

  Song song;
  song.set_url(url);
  song.set_title(filename);
  song.set_filesize(size);
  song.set_ctime(0);
  song.set_mtime(node->getModificationTime());

  // Get a streaming URL for tag reading.
  // The SDK's HTTP server decrypts and streams the file on the fly.
  std::unique_ptr<char[]> local_link(
      mega_api_->httpServerGetLocalLink(node));
  if (!local_link) {
    qLog(Warning) << "Mega: could not get streaming link for" << filename;
    return;
  }

  QUrl streaming_url = QUrl(QString::fromUtf8(local_link.get()));
  MaybeAddFileToDatabase(song, mime_type, streaming_url, QString());
}

bool MegaService::EnsureStreamingServer() {
  QMutexLocker lock(&mutex_);

  if (streaming_port_ > 0) return true;

  // Start the HTTP streaming server on localhost with an auto-selected port.
  // localOnly=true for security, port=0 means pick any available port.
  bool started = mega_api_->httpServerStart(true, 0);
  if (!started) {
    // The server might already be running from a previous session.
    // Try to check if it's responsive by just asking for the port.
    // If it returns 0, the server is truly not running.
    qLog(Warning) << "Mega: httpServerStart returned false, "
                     "server may already be running";
  }

  // Allow serving all file links (not just the last one).
  mega_api_->httpServerSetRestrictedMode(
      mega::MegaApi::HTTP_SERVER_ALLOW_ALL);

  // Set a reasonable buffer size (10MB).
  mega_api_->httpServerSetMaxBufferSize(10 * 1024 * 1024);

  streaming_port_ = mega_api_->httpServerIsRunning();
  if (streaming_port_ <= 0) {
    qLog(Error) << "Mega: streaming server is not running";
    return false;
  }

  qLog(Debug) << "Mega: streaming server running on port" << streaming_port_;
  return true;
}

QUrl MegaService::GetStreamingUrlFromSongId(const QUrl& url) {
  // URL format: mega:///HANDLE_BASE64
  QString handle_b64 = url.path();
  if (handle_b64.startsWith('/')) handle_b64 = handle_b64.mid(1);
  if (handle_b64.isEmpty()) return QUrl();

  // Ensure we're logged in.
  if (!logged_in_) {
    if (!DoLogin()) return QUrl();
    // Also need to fetch nodes if not done yet.
    mega::SynchronousRequestListener listener;
    mega_api_->fetchNodes(&listener);
    listener.wait();
    if (listener.getError()->getErrorCode() != mega::MegaError::API_OK) {
      return QUrl();
    }
  }

  if (!EnsureStreamingServer()) return QUrl();

  // Convert base64 handle back to MegaHandle.
  mega::MegaHandle handle = mega::MegaApi::base64ToHandle(
      handle_b64.toUtf8().constData());
  std::unique_ptr<mega::MegaNode> node(mega_api_->getNodeByHandle(handle));
  if (!node) {
    qLog(Error) << "Mega: node not found for handle" << handle_b64;
    return QUrl();
  }

  // Get a local streaming URL from the SDK's HTTP server.
  // This URL transparently decrypts and streams the file content.
  std::unique_ptr<char[]> local_link(
      mega_api_->httpServerGetLocalLink(node.get()));
  if (!local_link) {
    qLog(Error) << "Mega: failed to get streaming link for" << handle_b64;
    return QUrl();
  }

  return QUrl(QString::fromUtf8(local_link.get()));
}
