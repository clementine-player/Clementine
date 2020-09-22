/* This file is part of Clementine.
   Copyright 2014, Chocobozzz <djidane14ff@hotmail.fr>
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

#include "seafiletree.h"

#include <QDir>
#include <QRegExp>
#include <QStringList>

#include "core/logging.h"

SeafileTree::SeafileTree(QObject* parent) : QObject(parent) {}

SeafileTree::SeafileTree(const SeafileTree& copy, QObject* parent)
    : QObject(parent) {
  libraries_ = copy.libraries();
}

QList<SeafileTree::TreeItem*> SeafileTree::libraries() const {
  return libraries_;
}

void SeafileTree::Print() const {
  qLog(Debug) << "library count : " << libraries_.count();

  for (TreeItem* item : libraries_) {
    qLog(Debug) << "library : " << item->ToString(1);
  }
}

void SeafileTree::AddLibrary(const QString& name, const QString& id) {
  libraries_.append(new TreeItem(Entry(name, id, Entry::Type::LIBRARY)));
}

void SeafileTree::DeleteLibrary(const QString& id) {
  for (int i = 0; i < libraries_.size(); ++i) {
    if (libraries_.at(i)->entry().id() == id) {
      libraries_.removeAt(i);
      return;
    }
  }
}

bool SeafileTree::AddEntry(const QString& library, const QString& path,
                           const Entry& entry) {
  TreeItem* dir_node = FindFromAbsolutePath(library, path);

  if (!dir_node) {
    qLog(Warning) << "Can't find the path...";
    return false;
  }

  // If it is not a dir or a library we can't add an entry...
  if (!dir_node->entry().is_dir() && !dir_node->entry().is_library()) {
    qLog(Warning) << "This is not a dir or a file...";
    return false;
  }

  dir_node->AppendChild(entry);

  return true;
}

void SeafileTree::CheckEntries(const Entries& server_entries,
                               const Entry& library, const QString& path) {
  TreeItem* local_item = FindFromAbsolutePath(library.id(), path);

  // Don't know the path
  // Have to add all entries
  if (!local_item) {
    emit ToAdd(library.id(), path, library);
    return;
  }

  Entries local_entries = local_item->children_entries();

  for (const Entry& server_entry : server_entries) {
    bool is_in_tree = false;

    for (int i = 0; i < local_entries.size(); ++i) {
      Entry local_entry = local_entries.at(i);

      // We found the entry in the tree
      if (local_entry.name() == server_entry.name() &&
          local_entry.type() == server_entry.type()) {
        is_in_tree = true;

        // Need to update
        if (local_entry.id() != server_entry.id()) {
          emit ToUpdate(library.id(), path, server_entry);

          // Set the new id to the local entry
          local_entry.set_id(server_entry.id());
        }

        // local_entries could be named "local_entries_to_proceed"
        // So we delete from the list the entry we just proceeded
        local_entries.removeAt(i);

        break;
      }
    }

    // Need to add the entry
    if (!is_in_tree) {
      emit ToAdd(library.id(), path, server_entry);
    }
  }

  // Each entry in this list corresponds to an entry that we didn't proceed
  // So if the entry is in the tree but not on the server : we have to delete it
  for (const Entry& local_entry : local_entries) {
    emit ToDelete(library.id(), path, local_entry);
  }
}

SeafileTree::TreeItem* SeafileTree::FindLibrary(const QString& library) {
  for (TreeItem* item : libraries_) {
    if (item->entry().id() == library) return item;
  }

  return nullptr;
}

SeafileTree::TreeItem* SeafileTree::FindFromAbsolutePath(const QString& library,
                                                         const QString& path) {
  TreeItem* node_item = FindLibrary(library);

  if (!node_item) {
    return nullptr;
  }

  QStringList path_parts = path.split("/", QString::SkipEmptyParts);

  for (const QString& part : path_parts) {
    node_item = node_item->FindChild(part);

    if (!node_item) {
      break;
    }
  }

  return node_item;
}

bool SeafileTree::DeleteEntry(const QString& library, const QString& path,
                              const Entry& entry) {
  TreeItem* item_parent = FindFromAbsolutePath(library, path);

  if (!item_parent) {
    qLog(Debug) << "Unable to delete " << library + path + entry.name()
                << " : path " << path << " not found";
    return false;
  }

  TreeItem* item_entry = item_parent->FindChild(entry.name());

  if (!item_entry) {
    qLog(Debug) << "Unable to delete " << library + path + entry.name()
                << " : entry " << entry.name() << " from path not found";
    return false;
  }

  if (!item_parent->RemoveChild(item_entry)) {
    qLog(Debug) << "Can't remove " << item_entry->entry().name()
                << " from parent";
    return false;
  }

  delete item_entry;

  return true;
}

void SeafileTree::Clear() {
  qDeleteAll(libraries_);
  libraries_.clear();
}

QList<QPair<QString, SeafileTree::Entry>> SeafileTree::GetRecursiveFilesOfDir(
    const QString& path, const TreeItem* item) {
  // key = path, value = entry
  QList<QPair<QString, Entry>> files;

  if (!item) {
    return files;
  }

  if (item->entry().is_file()) {
    files.append(qMakePair(path, item->entry()));
    // Get files of the dir
  } else {
    for (TreeItem* child_item : item->children()) {
      if (child_item->entry().is_file()) {
        files.append(qMakePair(path, child_item->entry()));
      } else {
        QString name = child_item->entry().name() + "/";
        files.append(GetRecursiveFilesOfDir(path + name, child_item));
      }
    }
  }

  return files;
}

SeafileTree::~SeafileTree() {}

/* ################################# Entry ################################# */

QString SeafileTree::Entry::name() const { return name_; }
QString SeafileTree::Entry::id() const { return id_; }
SeafileTree::Entry::Type SeafileTree::Entry::type() const { return type_; }
bool SeafileTree::Entry::is_dir() const { return (type_ == Entry::DIR); }
bool SeafileTree::Entry::is_file() const { return (type_ == Entry::FILE); }
bool SeafileTree::Entry::is_library() const {
  return (type_ == Entry::LIBRARY);
}
void SeafileTree::Entry::set_name(const QString& name) { name_ = name; }
void SeafileTree::Entry::set_id(const QString& id) { id_ = id; }
void SeafileTree::Entry::set_type(const Type& type) { type_ = type; }

QString SeafileTree::Entry::ToString() const {
  return "name : " + name_ + " id : " + id_ + " type : " + TypeToString(type_);
}

SeafileTree::Entry& SeafileTree::Entry::operator=(const Entry& entry) {
  name_ = entry.name();
  id_ = entry.id();
  type_ = entry.type();

  return *this;
}

bool SeafileTree::Entry::operator==(const Entry& a) const {
  return a.name() == name() && a.id() == id() && a.type() == type();
}

bool SeafileTree::Entry::operator!=(const Entry& a) const {
  return !(operator==(a));
}

SeafileTree::Entry::~Entry() {}

QString SeafileTree::Entry::TypeToString(const Type& type) {
  switch (type) {
    case DIR:
      return "dir";
    case FILE:
      return "file";
    case LIBRARY:
      return "library";
    default:
      return QString();
  }
}

SeafileTree::Entry::Type SeafileTree::Entry::StringToType(const QString& type) {
  if (type == "dir") {
    return DIR;
  } else if (type == "file") {
    return FILE;
  } else if (type == "library") {
    return LIBRARY;
  }

  return NONE;
}

/* ############################### TreeItem ############################### */

SeafileTree::Entries SeafileTree::TreeItem::children_entries() const {
  Entries entries;

  for (TreeItem* item : children_) {
    entries.append(Entry(item->entry()));
  }

  return entries;
}

SeafileTree::TreeItem* SeafileTree::TreeItem::child(int i) const {
  return children_.at(i);
}
QList<SeafileTree::TreeItem*> SeafileTree::TreeItem::children() const {
  return children_;
}
SeafileTree::Entry SeafileTree::TreeItem::entry() const { return entry_; }
void SeafileTree::TreeItem::set_entry(const Entry& entry) { entry_ = entry; }
void SeafileTree::TreeItem::set_children(const QList<TreeItem*>& children) {
  children_ = children;
}

void SeafileTree::TreeItem::AppendChild(TreeItem* child) {
  children_.append(child);
}

void SeafileTree::TreeItem::AppendChild(const Entry& entry) {
  children_.append(new TreeItem(entry));
}

bool SeafileTree::TreeItem::RemoveChild(TreeItem* child) {
  return children_.removeOne(child);
}

SeafileTree::TreeItem* SeafileTree::TreeItem::FindChild(
    const QString& name) const {
  for (TreeItem* item : children_) {
    if (item->entry().name() == name) return item;
  }

  return nullptr;
}

QString SeafileTree::TreeItem::ToString(int i) const {
  QString res = "";

  for (int j = 0; j < i; ++j) {
    res += "  ";
  }

  res += entry_.ToString() + "\n";

  for (TreeItem* item : children_) {
    res += item->ToString(i + 1);
  }

  return res;
}

SeafileTree::TreeItem::~TreeItem() {
  // We need to delete children
  qDeleteAll(children_);
}

QDataStream& operator<<(QDataStream& out, const SeafileTree::Entry& entry) {
  out << entry.name_ << entry.id_ << static_cast<quint8>(entry.type_);

  return out;
}

QDataStream& operator>>(QDataStream& in, SeafileTree::Entry& entry) {
  quint8 temp;

  in >> entry.name_;
  in >> entry.id_;
  in >> temp;
  entry.type_ = SeafileTree::Entry::Type(temp);

  return in;
}

QDataStream& operator<<(QDataStream& out, SeafileTree::TreeItem* item) {
  out << item->entry_ << item->children_;

  return out;
}

QDataStream& operator>>(QDataStream& in, SeafileTree::TreeItem*& item) {
  SeafileTree::Entry entry;
  QList<SeafileTree::TreeItem*> children;

  in >> entry;
  in >> children;

  item = new SeafileTree::TreeItem(entry, children);

  return in;
}

QDataStream& operator<<(QDataStream& out, const SeafileTree& tree) {
  out << tree.libraries_;

  return out;
}

QDataStream& operator>>(QDataStream& in, SeafileTree& tree) {
  in >> tree.libraries_;

  return in;
}
