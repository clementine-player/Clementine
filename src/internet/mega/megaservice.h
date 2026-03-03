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

#ifndef INTERNET_MEGA_MEGASERVICE_H_
#define INTERNET_MEGA_MEGASERVICE_H_

#include <memory>

#include <QMutex>
#include <QSet>

#include "internet/core/cloudfileservice.h"

namespace mega {
class MegaApi;
class MegaNode;
}  // namespace mega

struct MegaFolderInfo {
  QString name;
  QString handle_b64;
  QList<MegaFolderInfo> children;
};

class MegaService : public CloudFileService {
  Q_OBJECT

 public:
  explicit MegaService(Application* app, InternetModel* parent);
  ~MegaService() override;

  static const char* kServiceName;
  static const char* kSettingsGroup;

  bool has_credentials() const override;

  // Returns a streamable HTTP URL for the given mega:// song URL.
  // Uses the SDK's built-in HTTP streaming server which transparently
  // decrypts and streams file content.
  QUrl GetStreamingUrlFromSongId(const QUrl& url);

  // Get the folder tree from the live SDK connection.
  // Requires being connected with nodes fetched.
  QList<MegaFolderInfo> GetFolderTree();

  // Load the cached folder tree from QSettings (persists across restarts).
  QList<MegaFolderInfo> GetCachedFolderTree() const;

  // Save a folder tree snapshot to QSettings.
  void SaveCachedFolderTree(const QList<MegaFolderInfo>& folders);

  // Clear the cached folder tree from QSettings.
  void ClearCachedFolderTree();

  // Get/set the list of folder handles (base64) that should be scanned.
  QStringList GetSelectedFolderHandles() const;
  void SetSelectedFolderHandles(const QStringList& handles);

  // Whether the service is logged in and nodes are fetched.
  bool IsReady() const { return logged_in_; }

  // Returns the set of folder handles (base64) that contain audio files
  // (directly or in subfolders).
  QSet<QString> GetFoldersWithAudio();

  // Clear the library database for this service and reset selected folders.
  void ResetDatabase();

 public slots:
  void Connect() override;
  void ForgetCredentials();

  // Scan only the user-selected folders (reads selection from settings).
  void ScanSelectedFolders();

 signals:
  void Connected();

 protected:
  void DoFullRescan() override;

 private:
  // Authenticate with the MEGA API using email/password or cached session.
  // Returns true on success.
  bool DoLogin();

  // Fetch node tree from MEGA (without indexing). Returns true on success.
  bool FetchNodes();

  // Build the folder tree structure for UI display.
  void BuildFolderTree(mega::MegaNode* node, QList<MegaFolderInfo>& list);

  // Recursively check if a folder (or its subfolders) contains audio files.
  // Populates the output set with handles of matching folders.
  bool CollectFoldersWithAudio(mega::MegaNode* node, QSet<QString>& out);

  // Recursively traverse the node tree and add music files to the library.
  void IndexNodeRecursive(mega::MegaNode* node, const QString& path);

  // Process a single file node: add to library if it's a music file.
  void ProcessFileNode(mega::MegaNode* node, const QString& path);

  // Ensure the SDK's HTTP streaming server is running.
  // Returns true if the server is started (or was already running).
  bool EnsureStreamingServer();

  // MEGA SDK application key (register at https://mega.nz/sdk).
  static const char* kAppKey;

 private:
  // The MEGA SDK API instance. Handles all communication with MEGA servers,
  // encryption/decryption, and the local HTTP streaming server.
  std::unique_ptr<mega::MegaApi> mega_api_;

  // Port on which the SDK's local HTTP streaming server is listening.
  // 0 means the server hasn't been started yet.
  int streaming_port_;

  // Whether we are currently logged in and nodes are fetched.
  bool logged_in_;

  // Mutex for thread-safe access to streaming state.
  mutable QMutex mutex_;
};

#endif  // INTERNET_MEGA_MEGASERVICE_H_
