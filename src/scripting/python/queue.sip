struct Queue : QAbstractProxyModel {

%TypeHeaderCode
#include "playlist/queue.h"
%End

public:
  Queue();

  static const char* kRowsMimetype;

  // Query the queue
  bool is_empty() const;
  int PositionOf(const QModelIndex& source_index) const;
  bool ContainsSourceRow(int source_row) const;
  int PeekNext() const;

  // Modify the queue
  int TakeNext();
  void ToggleTracks(const QModelIndexList& source_indexes);
  void Clear();
  void Move(const QList<int>& proxy_rows, int pos);
  void MoveUp(int row);
  void MoveDown(int row);
};
