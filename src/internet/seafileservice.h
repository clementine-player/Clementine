/* Contacts (for explanations, congratulations, insults) :
 *  - <florian.bigard@gmail.com>
 *
 * Help :
 *  - The "path" variable has to end with "/".
 *     If we want to specify a filepath, the name of the variable has to be...
 *filepath :)
 *  - Seafile stores files in libraries (or repositories) so variable with the
 *name "library" corresponds to the
 *     Seafile library, not to the Clementine library
 *  - The authentification of Seafile's API is simply a token (REST API)
 *  - Seafile stores a hash for each entry. This hash changes when the entry is
 *modified.
 *     This is the reason why we just have to compare the local hash with the
 *server
 *     hash of a directory (for example) to know if the directory was modified.
 *     Libraries are an exception : Seafile stores a hash that never changes.
 *     This hash is called "id".
 *
 * Todo :
 *  - Add ssl certificate exception (for people who generate their own
 *certificate on their Seafile server
 *  - Stop Tagreader when user changes the library
*/

#ifndef SEAFILESERVICE_H
#define SEAFILESERVICE_H

#include "cloudfileservice.h"

#include <QDateTime>
#include <QMutex>

#include "seafiletree.h"

class QNetworkReply;
class QNetworkRequest;

// Interface between the seafile server and Clementine
class SeafileService : public CloudFileService {
  Q_OBJECT
 public:
  enum ApiError { NO_ERROR = 200, NOT_FOUND = 404, TOO_MANY_REQUESTS = 429 };

  SeafileService(Application* app, InternetModel* parent);
  ~SeafileService();

  static const char* kServiceName;
  static const char* kSettingsGroup;

  virtual bool has_credentials() const;
  QUrl GetStreamingUrlFromSongId(const QString& library,
                                 const QString& filepath);
  // Get the token for an user (simple rest api)
  bool GetToken(const QString& mail, const QString& password,
                const QString& server);
  // Get all the libraries available for the user. Will emit a signal
  void GetLibraries();
  void ChangeLibrary(const QString& new_library);

 public slots:
  void Connect();
  void ForgetCredentials();

signals:
  void Connected();
  // QMap, key : library's id, value : library's name
  void GetLibrariesFinishedSignal(QMap<QString, QString>);

 private slots:
  // Will emit the signal
  void GetLibrariesFinished(QNetworkReply* reply);

  void FetchAndCheckFolderItemsFinished(QNetworkReply* reply,
                                        const SeafileTree::Entry& library,
                                        const QString& path);

  // Add recursively the content of a folder from a library
  void AddRecursivelyFolderItemsFinished(QNetworkReply* reply,
                                         const QString& library,
                                         const QString& path);
  // Get the url and try to add the file to the database
  void FetchContentUrlForFileFinished(QNetworkReply* reply, const Song& song,
                                      const QString& mime_type);
  // Add the entry to the tree and maybe add this entry to the database
  void AddEntry(const QString& library, const QString& path,
                const SeafileTree::Entry& entry);
  // Update the entry or check recursively the directories
  void UpdateEntry(const QString& library, const QString& path,
                   const SeafileTree::Entry& entry);
  // Delete the entry (eventually the files of its subdir) of the tree and the
  // database
  void DeleteEntry(const QString& library, const QString& path,
                   const SeafileTree::Entry& entry);

  void UpdateLibrariesInProgress(const QMap<QString, QString>& libraries);

  void MaybeAddFileEntryInProgress(QNetworkReply* reply, const QString& library,
                                   const QString& path,
                                   const QString& mime_type);

 private:
  QString access_token() const;
  bool is_authenticated() const;

  void AddAuthorizationHeader(QNetworkRequest* request) const;

  void UpdateLibraries();

  void FetchAndCheckFolderItems(const SeafileTree::Entry& library,
                                const QString& path);
  void AddRecursivelyFolderItems(const QString& library, const QString& path);

  QNetworkReply* PrepareFetchFolderItems(const QString& library,
                                         const QString& path);
  QNetworkReply* PrepareFetchContentForFile(const QString& library,
                                            const QString& filepath);
  QNetworkReply* PrepareFetchContentUrlForFile(const QString& library,
                                               const QString& filepath);

  void MaybeAddFileEntry(const QString& entry_name, const QString& library,
                         const QString& path);

  // False if not 200 or 429
  // If 429 (too many requests), re execute the request and put the reply in the
  // argument
  bool CheckReply(QNetworkReply** reply, int tries = 1);

  SeafileTree tree_;
  QString access_token_;
  QString server_;
  QString library_updated_;
};

#endif  // SEAFILESERVICE_H
