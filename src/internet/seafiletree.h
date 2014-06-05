/* Contacts (for explanations, congratulations, insults) :
 *  - <florian.bigard@gmail.com>
*/

#ifndef SEAFILETREE_H
#define SEAFILETREE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QPair>

#include "cloudfileservice.h"

// Reproduce the file system of Seafile server libraries
// Analog to a tree
class SeafileTree : public QObject {
  Q_OBJECT

 public:
  SeafileTree();
  SeafileTree(const SeafileTree& copy);
  ~SeafileTree();

  class Entry {
   public:
    enum Type { DIR = 0, FILE = 1, LIBRARY = 2, NONE = 3 };

    Entry(const QString& name = QString(), const QString& id = QString(),
          const Type& type = NONE)
        : name_(name), id_(id), type_(type) {}
    Entry(const Entry& entry)
      : name_(entry.name()), id_(entry.id()), type_(entry.type()) {}
    ~Entry();

    QString name() const;
    void set_name(const QString& name);
    QString id() const;
    void set_id(const QString& id);
    Type type() const;
    void set_type(const Type& type);

    bool is_dir() const;
    bool is_file() const;
    bool is_library() const;

    Entry& operator=(const Entry& entry);
    bool operator==(const Entry& a) const;
    bool operator!=(const Entry& a) const;

    QString ToString() const;

    static QString TypeToString(const Type& type);
    static Type StringToType(const QString& type);

   private:
    QString name_, id_;
    Type type_;

    friend QDataStream& operator<<(QDataStream& out,
                                   const SeafileTree::Entry& entry);
    friend QDataStream& operator>>(QDataStream& in, SeafileTree::Entry& entry);
  };

  typedef QList<Entry> Entries;

  // Node of the tree
  // Contains an entry
  class TreeItem {
   public:
    TreeItem(const Entry& entry = Entry(),
             const QList<TreeItem*>& children = QList<TreeItem*>())
        : entry_(entry), children_(children) {}
    TreeItem(const TreeItem& copy)
      : entry_(copy.entry()), children_(copy.children()) {}
    ~TreeItem();

    TreeItem* child(int i) const;
    QList<TreeItem*> children() const;
    // List of each child's entry
    Entries children_entries() const;

    void set_children(const QList<TreeItem*>& children);

    Entry entry() const;
    void set_entry(const Entry& entry);

    void AppendChild(TreeItem* child);
    void AppendChild(const Entry& entry);

    // True if child is removed
    bool RemoveChild(TreeItem* child);

    // nullptr if we didn't find a child entry with the given name
    TreeItem* FindChild(const QString& name) const;

    // Convert the node in QString (for debug)
    QString ToString(int i) const;

   private:
    Entry entry_;
    QList<TreeItem*> children_;

    friend QDataStream& operator<<(QDataStream& out,
                                   SeafileTree::TreeItem* item);
    friend QDataStream& operator>>(QDataStream& in,
                                   SeafileTree::TreeItem*& item);
  };

  QList<TreeItem*> libraries() const;

  void AddLibrary(const QString& name, const QString& id);
  void DeleteLibrary(const QString& id);
  bool AddEntry(const QString& library, const QString& path,
                const Entry& entry);
  bool DeleteEntry(const QString& library, const QString& path,
                   const Entry& entry);

  // Get a list of pair (path, entry) corresponding to the subfiles (and
  // recursively to the subsubfiles...) of the given item
  QList<QPair<QString, SeafileTree::Entry>> GetRecursiveFilesOfDir(
      const QString& path, const TreeItem* item);

  // nullptr if we didn't find the library with the given id
  TreeItem* FindLibrary(const QString& library);
  // nullptr if we didn't find the item
  TreeItem* FindFromAbsolutePath(const QString& library, const QString& path);

  // Compare the server entries with the tree
  // Emit signals (ToDelete, ToAdd, ToUpdate)
  void CheckEntries(const Entries& server_entries, const Entry& library,
                    const QString& path);

  // Destroy the tree
  void Clear();

  // Print the tree in the debug log
  void Print() const;

signals:
  // Entry to delete in the tree
  void ToDelete(const QString& library, const QString& path,
                const SeafileTree::Entry& entry);
  // Entry to add in the tree
  void ToAdd(const QString& library, const QString& path,
             const SeafileTree::Entry& entry);
  // Entry to update in the tree
  void ToUpdate(const QString& library, const QString& path,
                const SeafileTree::Entry& entry);

 private:
  QList<TreeItem*> libraries_;

  friend QDataStream& operator<<(QDataStream& out, const SeafileTree& tree);
  friend QDataStream& operator>>(QDataStream& in, SeafileTree& tree);
};

QDataStream& operator<<(QDataStream& out, const SeafileTree& tree);
QDataStream& operator>>(QDataStream& in, SeafileTree& tree);

QDataStream& operator<<(QDataStream& out, const SeafileTree::Entry& entry);
QDataStream& operator>>(QDataStream& in, SeafileTree::Entry& entry);

QDataStream& operator<<(QDataStream& out, SeafileTree::TreeItem* item);
QDataStream& operator>>(QDataStream& in, SeafileTree::TreeItem*& item);

#endif  // SEAFILETREE_H
