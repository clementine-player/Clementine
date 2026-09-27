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

#include "networkremote/internetbrowser.h"

#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTest>
#include <QThread>
#include <QTimer>
#include <functional>
#include <memory>

#include "core/mimedata.h"
#include "core/song.h"
#include "gtest/gtest.h"
#include "internet/core/internetmodel.h"
#include "library/libraryitem.h"
#include "test_utils.h"

namespace {

// Marks an item as a library item of this LibraryItem::Type, for the fake
// item_info hook.
const int kLibraryTypeRole = Qt::UserRole + 500;

// Stands in for InternetModel: nodes marked Role_CanLazyLoad get their
// children later, when first counted, as a service's LazyPopulate does.
class FakeModel : public QStandardItemModel {
 public:
  // Children to add to a lazy node, and how long after it's first counted.
  std::function<void(QStandardItem*)> populate;
  int populate_delay_msec = 20;

  int rowCount(const QModelIndex& parent) const override {
    if (parent.data(InternetModel::Role_CanLazyLoad).toBool()) {
      QStandardItem* item = itemFromIndex(parent);
      item->setData(false, InternetModel::Role_CanLazyLoad);
      if (populate) {
        auto add = populate;
        QTimer::singleShot(populate_delay_msec, [add, item]() { add(item); });
      }
    }
    return QStandardItemModel::rowCount(parent);
  }

  bool hasChildren(const QModelIndex& parent) const override {
    if (parent.data(InternetModel::Role_CanLazyLoad).toBool()) return true;
    return QStandardItemModel::hasChildren(parent);
  }

  // What a drag from the sidebar would carry: here, just the titles.
  QMimeData* mimeData(const QModelIndexList& indexes) const override {
    MimeData* data = new MimeData;
    QStringList titles;
    for (const QModelIndex& index : indexes) titles << index.data().toString();
    data->setText(titles.join(","));
    return data;
  }
};

QStandardItem* Service(const QString& name) {
  QStandardItem* item = new QStandardItem(name);
  item->setData(InternetModel::Type_Service, InternetModel::Role_Type);
  item->setDragEnabled(false);
  return item;
}

QStandardItem* Folder(const QString& name) {
  QStandardItem* item = new QStandardItem(name);
  item->setDragEnabled(false);
  return item;
}

QStandardItem* Playable(const QString& name, qint64 length_sec = 0,
                        const QString& artist = QString()) {
  QStandardItem* item = new QStandardItem(name);
  item->setDragEnabled(true);
  Song song;
  song.Init(name, artist, "Album", length_sec * 1000000000ll);
  item->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
  return item;
}

class InternetBrowserTest : public ::testing::Test {
 protected:
  // QSignalSpy has to be able to store AddToPlaylist's argument.
  static void SetUpTestCase() { qRegisterMetaType<QMimeData*>("QMimeData*"); }

  void SetUp() {
    // Radio: a divider, a folder, a stream, a track and a smart playlist.
    radio_ = Service("Radio");
    QStandardItem* divider = new QStandardItem("A");
    divider->setData(true, InternetModel::Role_IsDivider);
    radio_->appendRow(divider);
    genres_ = Folder("Genres");
    genres_->appendRow(Playable("Jazz"));
    radio_->appendRow(genres_);
    radio_->appendRow(Playable("Groove Salad"));
    radio_->appendRow(Playable("A song", 180, "An artist"));
    QStandardItem* smart = Playable("Smart");
    smart->setData(InternetModel::Type_SmartPlaylist, InternetModel::Role_Type);
    radio_->appendRow(smart);

    // Lazy: gets its children only when first browsed.
    lazy_ = Service("Lazy");
    lazy_->setData(true, InternetModel::Role_CanLazyLoad);

    // Unconfigured: has to be set up first.
    unconfigured_ = Service("Unconfigured");

    // Many: more children than one page.
    many_ = Service("Many");
    for (int i = 0; i < 25; ++i) many_->appendRow(Playable(QString::number(i)));

    model_.appendRow(radio_);
    model_.appendRow(lazy_);
    model_.appendRow(unconfigured_);
    model_.appendRow(many_);

    InternetBrowser::Hooks hooks = InternetBrowser::DefaultHooks();
    hooks.needs_setup = [](const QModelIndex& index) {
      return index.data().toString() == "Unconfigured";
    };
    const auto default_info = hooks.item_info;
    hooks.item_info = [default_info](const QModelIndex& index) {
      InternetBrowser::ItemInfo info = default_info(index);
      const QVariant type = index.data(kLibraryTypeRole);
      if (type.isValid()) info.library_type = type.toInt();
      return info;
    };
    hooks_ = hooks;
    browser_.SetTimeouts(10, 300);
    browser_.SetModel(&model_, hooks_);
  }

  void TearDown() {
    for (const QList<QVariant>& args : added_) {
      delete args[0].value<QMimeData*>();
    }
  }

  void Send(const cpb::remote::Message& msg, int client = 1) {
    const std::string data = msg.SerializeAsString();
    browser_.HandleMessage(client, QByteArray(data.data(), data.size()));
  }

  void RequestBrowse(const QString& node_id = QString(), int offset = 0,
                     int limit = 0, int client = 1) {
    cpb::remote::Message msg;
    msg.set_type(cpb::remote::REQUEST_BROWSE);
    msg.mutable_request_browse()->set_node_id(node_id.toStdString());
    if (offset) msg.mutable_request_browse()->set_offset(offset);
    if (limit) msg.mutable_request_browse()->set_limit(limit);
    Send(msg, client);
  }

  // The last message the browser sent.
  cpb::remote::Message Last() {
    EXPECT_FALSE(sent_.isEmpty());
    cpb::remote::Message msg;
    if (sent_.isEmpty()) return msg;
    const QByteArray data = sent_.last()[1].toByteArray();
    msg.ParseFromArray(data.constData(), data.size());
    return msg;
  }

  cpb::remote::ResponseBrowse Browse(const QString& node_id = QString(),
                                     int offset = 0, int limit = 0) {
    RequestBrowse(node_id, offset, limit);
    cpb::remote::Message msg = Last();
    EXPECT_EQ(cpb::remote::BROWSE, msg.type());
    return msg.response_browse();
  }

  // Waits for the browser to push another message, and returns it.
  cpb::remote::Message Next() {
    const int before = sent_.size();
    EXPECT_TRUE(QTest::qWaitFor([&]() { return sent_.size() > before; }, 2000))
        << "no update came";
    return Last();
  }

  QString IdOf(const cpb::remote::ResponseBrowse& response,
               const QString& title) {
    for (const cpb::remote::BrowseNode& node : response.nodes()) {
      if (QString::fromStdString(node.title()) == title) {
        return QString::fromStdString(node.node_id());
      }
    }
    ADD_FAILURE() << "no node called " << title.toStdString();
    return QString();
  }

  const cpb::remote::BrowseNode* NodeOf(
      const cpb::remote::ResponseBrowse& response, const QString& title) {
    for (const cpb::remote::BrowseNode& node : response.nodes()) {
      if (QString::fromStdString(node.title()) == title) return &node;
    }
    return nullptr;
  }

  cpb::remote::ResponseBrowseAdd AddNodes(const QStringList& ids,
                                          cpb::remote::BrowseAddAction action) {
    cpb::remote::Message msg;
    msg.set_type(cpb::remote::REQUEST_BROWSE_ADD);
    for (const QString& id : ids) {
      msg.mutable_request_browse_add()->add_node_ids(id.toStdString());
    }
    msg.mutable_request_browse_add()->set_action(action);
    Send(msg);
    cpb::remote::Message reply = Last();
    EXPECT_EQ(cpb::remote::BROWSE_ADD_RESULT, reply.type());
    return reply.response_browse_add();
  }

  MimeData* Added(int i) {
    return qobject_cast<MimeData*>(added_[i][0].value<QMimeData*>());
  }

  // NetworkRemote makes the browser on its own thread and moves it to the
  // main one, so do the same: whatever it owns has to move with it.
  static InternetBrowser* MadeOnAnotherThread() {
    InternetBrowser* browser = nullptr;
    QThread* thread = QThread::create([&browser]() {
      browser = new InternetBrowser;
      browser->moveToThread(QCoreApplication::instance()->thread());
    });
    thread->start();
    thread->wait();
    delete thread;
    return browser;
  }

  FakeModel model_;
  InternetBrowser::Hooks hooks_;
  std::unique_ptr<InternetBrowser> owned_browser_{MadeOnAnotherThread()};
  InternetBrowser& browser_{*owned_browser_};
  QSignalSpy sent_{&browser_, SIGNAL(SendToClient(int, QByteArray))};
  QSignalSpy added_{&browser_, SIGNAL(AddToPlaylist(QMimeData*))};

  QStandardItem* radio_;
  QStandardItem* genres_;
  QStandardItem* lazy_;
  QStandardItem* unconfigured_;
  QStandardItem* many_;
};

}  // namespace

TEST_F(InternetBrowserTest, ListsTheServices) {
  const cpb::remote::ResponseBrowse root = Browse();
  EXPECT_EQ(cpb::remote::BROWSE_STATE_READY, root.state());
  ASSERT_EQ(4, root.nodes_size());
  EXPECT_EQ("Radio", root.nodes(0).title());
  for (const cpb::remote::BrowseNode& node : root.nodes()) {
    EXPECT_EQ(cpb::remote::BROWSE_NODE_KIND_SERVICE, node.kind());
    EXPECT_FALSE(node.node_id().empty());
  }
  EXPECT_EQ(cpb::remote::BROWSE_CHILDREN_SOME,
            NodeOf(root, "Radio")->children());
  EXPECT_EQ(cpb::remote::BROWSE_CHILDREN_SOME,
            NodeOf(root, "Lazy")->children());
  EXPECT_EQ(cpb::remote::BROWSE_CHILDREN_NONE,
            NodeOf(root, "Unconfigured")->children());
}

TEST_F(InternetBrowserTest, DescribesEachKindOfNode) {
  const cpb::remote::ResponseBrowse radio = Browse(IdOf(Browse(), "Radio"));
  EXPECT_EQ(cpb::remote::BROWSE_STATE_READY, radio.state());
  // The divider isn't a node.
  ASSERT_EQ(4, radio.nodes_size());
  EXPECT_EQ(4, radio.total_count());

  const cpb::remote::BrowseNode* genres = NodeOf(radio, "Genres");
  EXPECT_EQ(cpb::remote::BROWSE_NODE_KIND_FOLDER, genres->kind());
  EXPECT_EQ(cpb::remote::BROWSE_CHILDREN_SOME, genres->children());
  EXPECT_EQ(cpb::remote::BROWSE_PLAYABILITY_NONE, genres->playability());

  const cpb::remote::BrowseNode* stream = NodeOf(radio, "Groove Salad");
  EXPECT_EQ(cpb::remote::BROWSE_NODE_KIND_STREAM, stream->kind());
  EXPECT_EQ(cpb::remote::BROWSE_PLAYABILITY_ADDABLE, stream->playability());

  const cpb::remote::BrowseNode* track = NodeOf(radio, "A song");
  EXPECT_EQ(cpb::remote::BROWSE_NODE_KIND_TRACK, track->kind());
  EXPECT_EQ("An artist", track->subtitle());
  EXPECT_EQ(180, track->song().length());

  EXPECT_EQ(cpb::remote::BROWSE_NODE_KIND_SMART_PLAYLIST,
            NodeOf(radio, "Smart")->kind());
}

TEST_F(InternetBrowserTest, PagesThroughChildren) {
  const QString many = IdOf(Browse(), "Many");
  const cpb::remote::ResponseBrowse page = Browse(many, 10, 5);
  EXPECT_EQ(10, page.offset());
  EXPECT_EQ(25, page.total_count());
  ASSERT_EQ(5, page.nodes_size());
  EXPECT_EQ("10", page.nodes(0).title());

  // No limit: everything, up to kMaxPageSize.
  EXPECT_EQ(25, Browse(many).nodes_size());
}

TEST_F(InternetBrowserTest, IdsAreStable) {
  const QString radio = IdOf(Browse(), "Radio");
  const QString genres = IdOf(Browse(radio), "Genres");
  EXPECT_EQ(radio, IdOf(Browse(), "Radio"));
  EXPECT_EQ(genres, IdOf(Browse(radio), "Genres"));
  EXPECT_NE(radio, genres);
}

TEST_F(InternetBrowserTest, LazyNodeLoadsThenUpdates) {
  model_.populate = [](QStandardItem* item) {
    item->appendRow(Playable("Late one"));
    item->appendRow(Playable("Late two"));
  };
  const cpb::remote::ResponseBrowse loading = Browse(IdOf(Browse(), "Lazy"));
  EXPECT_EQ(cpb::remote::BROWSE_STATE_LOADING, loading.state());
  EXPECT_EQ(0, loading.nodes_size());

  const cpb::remote::ResponseBrowse loaded = Next().response_browse();
  EXPECT_EQ(cpb::remote::BROWSE_STATE_READY, loaded.state());
  EXPECT_EQ(2, loaded.nodes_size());
  EXPECT_EQ(loading.node_id(), loaded.node_id());
}

TEST_F(InternetBrowserTest, LoadingGivesUpEventually) {
  // A service that never answers.
  const cpb::remote::ResponseBrowse loading = Browse(IdOf(Browse(), "Lazy"));
  EXPECT_EQ(cpb::remote::BROWSE_STATE_LOADING, loading.state());

  const cpb::remote::ResponseBrowse ready = Next().response_browse();
  EXPECT_EQ(cpb::remote::BROWSE_STATE_READY, ready.state());
  EXPECT_EQ(0, ready.nodes_size());
}

TEST_F(InternetBrowserTest, LoadingRowIsSkippedAndMeansLoading) {
  QStandardItem* loading_row = new QStandardItem("Loading...");
  loading_row->setData(LibraryItem::Type_LoadingIndicator, kLibraryTypeRole);
  genres_->removeRows(0, genres_->rowCount());
  genres_->appendRow(loading_row);

  const QString radio = IdOf(Browse(), "Radio");
  const cpb::remote::ResponseBrowse genres =
      Browse(IdOf(Browse(radio), "Genres"));
  EXPECT_EQ(cpb::remote::BROWSE_STATE_LOADING, genres.state());
  EXPECT_EQ(0, genres.nodes_size());
}

TEST_F(InternetBrowserTest, UnconfiguredServiceNeedsSetup) {
  const cpb::remote::ResponseBrowse response =
      Browse(IdOf(Browse(), "Unconfigured"));
  EXPECT_EQ(cpb::remote::BROWSE_STATE_NEEDS_SETUP, response.state());
  EXPECT_NE(std::string::npos, response.message().find("Unconfigured"));
}

TEST_F(InternetBrowserTest, UnknownIdIsGone) {
  EXPECT_EQ(cpb::remote::BROWSE_STATE_GONE, Browse("n999").state());
}

TEST_F(InternetBrowserTest, RemovedNodeIsGone) {
  const QString radio = IdOf(Browse(), "Radio");
  const QString genres = IdOf(Browse(radio), "Genres");
  Browse(genres);

  radio_->removeRow(genres_->row());
  EXPECT_EQ(cpb::remote::BROWSE_STATE_GONE, Next().response_browse().state());
  EXPECT_EQ(cpb::remote::BROWSE_STATE_GONE, Browse(genres).state());
}

TEST_F(InternetBrowserTest, RemovedAncestorIsGone) {
  const QString radio = IdOf(Browse(), "Radio");
  Browse(IdOf(Browse(radio), "Genres"));

  model_.removeRow(radio_->row());
  EXPECT_EQ(cpb::remote::BROWSE_STATE_GONE, Next().response_browse().state());
}

TEST_F(InternetBrowserTest, ChangesToTheWatchedNodeArePushed) {
  const QString radio = IdOf(Browse(), "Radio");
  Browse(radio);
  radio_->appendRow(Playable("New station"));

  const cpb::remote::ResponseBrowse update = Next().response_browse();
  EXPECT_EQ(radio.toStdString(), update.node_id());
  EXPECT_NE(nullptr, NodeOf(update, "New station"));
}

TEST_F(InternetBrowserTest, OtherChangesAreNotPushed) {
  Browse(IdOf(Browse(), "Radio"));
  const int before = sent_.size();

  many_->appendRow(Playable("Elsewhere"));
  QTest::qWait(100);
  EXPECT_EQ(before, sent_.size());
}

TEST_F(InternetBrowserTest, AddingSetsThePlayFlags) {
  const cpb::remote::ResponseBrowse radio = Browse(IdOf(Browse(), "Radio"));
  const QString stream = IdOf(radio, "Groove Salad");

  struct Case {
    cpb::remote::BrowseAddAction action;
    bool play_now, enqueue_next_now, clear_first;
  };
  const Case cases[] = {
      {cpb::remote::BROWSE_ADD_ACTION_APPEND, false, false, false},
      {cpb::remote::BROWSE_ADD_ACTION_PLAY_NOW, true, false, false},
      {cpb::remote::BROWSE_ADD_ACTION_PLAY_NEXT, false, true, false},
      {cpb::remote::BROWSE_ADD_ACTION_REPLACE, true, false, true},
  };
  for (int i = 0; i < 4; ++i) {
    const cpb::remote::ResponseBrowseAdd result =
        AddNodes({stream}, cases[i].action);
    EXPECT_EQ(cpb::remote::BROWSE_ADD_RESULT_ADDED, result.result());
    ASSERT_EQ(i + 1, added_.size());
    MimeData* data = Added(i);
    ASSERT_NE(nullptr, data);
    EXPECT_TRUE(data->override_user_settings_);
    EXPECT_EQ(cases[i].play_now, data->play_now_) << i;
    EXPECT_EQ(cases[i].enqueue_next_now, data->enqueue_next_now_) << i;
    EXPECT_FALSE(data->enqueue_now_) << i;
    EXPECT_EQ(cases[i].clear_first, data->clear_first_) << i;
    EXPECT_EQ("Groove Salad", data->text());
  }
}

TEST_F(InternetBrowserTest, FoldersThatCantBeDraggedAreNotPlayable) {
  const QString genres = IdOf(Browse(IdOf(Browse(), "Radio")), "Genres");
  EXPECT_EQ(cpb::remote::BROWSE_ADD_RESULT_NOT_PLAYABLE,
            AddNodes({genres}, cpb::remote::BROWSE_ADD_ACTION_APPEND).result());
  EXPECT_EQ(0, added_.size());
}

TEST_F(InternetBrowserTest, AddingAGoneNodeAddsNothing) {
  const QString stream = IdOf(Browse(IdOf(Browse(), "Radio")), "Groove Salad");
  EXPECT_EQ(cpb::remote::BROWSE_ADD_RESULT_GONE,
            AddNodes({stream, "n999"}, cpb::remote::BROWSE_ADD_ACTION_APPEND)
                .result());
  EXPECT_EQ(0, added_.size());
}

TEST_F(InternetBrowserTest, SelectionsSpanningModelsAreAddedInOrder) {
  // As though each node came from a different source model.
  InternetBrowser::Hooks hooks = hooks_;
  hooks.group_by_model = [](const QModelIndexList& indexes) {
    QList<QModelIndexList> groups;
    for (const QModelIndex& index : indexes) groups << QModelIndexList{index};
    return groups;
  };
  browser_.SetModel(&model_, hooks);

  const cpb::remote::ResponseBrowse many = Browse(IdOf(Browse(), "Many"));
  const QStringList ids = {IdOf(many, "0"), IdOf(many, "1"), IdOf(many, "2")};

  AddNodes(ids, cpb::remote::BROWSE_ADD_ACTION_PLAY_NOW);
  ASSERT_EQ(3, added_.size());
  EXPECT_EQ("0", Added(0)->text());
  EXPECT_TRUE(Added(0)->play_now_);
  EXPECT_FALSE(Added(1)->play_now_);
  EXPECT_FALSE(Added(2)->play_now_);

  // Each group queued "next" goes in front of the last, so they're queued
  // last first to end up in order.
  AddNodes(ids, cpb::remote::BROWSE_ADD_ACTION_PLAY_NEXT);
  ASSERT_EQ(6, added_.size());
  EXPECT_EQ("2", Added(3)->text());
  EXPECT_EQ("1", Added(4)->text());
  EXPECT_EQ("0", Added(5)->text());
}

TEST_F(InternetBrowserTest, ClientsHaveTheirOwnIds) {
  const QString radio = IdOf(Browse(), "Radio");
  RequestBrowse(radio, 0, 0, 2);
  EXPECT_EQ(cpb::remote::BROWSE_STATE_GONE, Last().response_browse().state());

  browser_.ClientDisconnected(1);
  EXPECT_EQ(cpb::remote::BROWSE_STATE_GONE, Browse(radio).state());
}
