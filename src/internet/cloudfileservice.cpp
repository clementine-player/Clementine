#include "cloudfileservice.h"

#include <QMenu>
#include <QSortFilterProxyModel>

#include "core/application.h"
#include "core/database.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/player.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/librarysearchprovider.h"
#include "internet/internetmodel.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"
#include "playlist/playlist.h"
#include "ui/iconloader.h"

CloudFileService::CloudFileService(
    Application* app,
    InternetModel* parent,
    const QString& service_name,
    const QString& service_id,
    const QIcon& icon,
    SettingsDialog::Page settings_page)
  : InternetService(service_name, app, parent, parent),
    root_(nullptr),
    network_(new NetworkAccessManager(this)),
    library_sort_model_(new QSortFilterProxyModel(this)),
    playlist_manager_(app->playlist_manager()),
    icon_(icon),
    settings_page_(settings_page) {
  library_backend_ = new LibraryBackend;
  library_backend_->moveToThread(app_->database()->thread());

  QString songs_table = service_id + "_songs";
  QString songs_fts_table = service_id + "_songs_fts";

  library_backend_->Init(
      app->database(), songs_table, QString::null, QString::null, songs_fts_table);
  library_model_ = new LibraryModel(library_backend_, app_, this);

  library_sort_model_->setSourceModel(library_model_);
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);

  app->global_search()->AddProvider(new LibrarySearchProvider(
      library_backend_,
      service_name,
      service_id,
      icon_,
      true, app_, this));
}

QStandardItem* CloudFileService::CreateRootItem() {
  root_ = new QStandardItem(icon_, name());
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void CloudFileService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      if (!has_credentials()) {
        ShowSettingsDialog();
      } else {
        Connect();
      }
      library_model_->Init();
      model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
      break;

    default:
      break;
  }
}

void CloudFileService::ShowContextMenu(const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_.reset(new QMenu);
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addAction(
        IconLoader::Load("download"),
        tr("Cover Manager"),
        this,
        SLOT(ShowCoverManager()));
  }
  context_menu_->popup(global_pos);
}

void CloudFileService::ShowCoverManager() {
  if (!cover_manager_) {
    cover_manager_.reset(new AlbumCoverManager(app_, library_backend_));
    cover_manager_->Init();
    connect(cover_manager_.get(), SIGNAL(AddToPlaylist(QMimeData*)),
            SLOT(AddToPlaylist(QMimeData*)));
  }
  cover_manager_->show();
}

void CloudFileService::AddToPlaylist(QMimeData* mime) {
  playlist_manager_->current()->dropMimeData(
      mime, Qt::CopyAction, -1, 0, QModelIndex());
}

void CloudFileService::ShowSettingsDialog() {
  app_->OpenSettingsDialogAtPage(settings_page_);
}
