struct Queue : QAbstractProxyModel {

%TypeHeaderCode
#include "playlist/queue.h"
%End

%Docstring
A model containing the list of songs that are enqueued on a playlist.

In Clementine each L{Playlist} has its own Queue object.  The Queue keeps an
ordered list of items that are enqueued on that playlist.  Whenever a song
finishes the top item is removed from the Queue and is played immediately.
When there are no items left on the Queue playback continues again in the normal
order.

The L{Playlist} is a standard Qt item model where each row represents one item
in the playlist.  The Queue is a Qt proxy model on top of the Playlist model,
which means that rows in the Queue can map directly onto rows in the Playlist.
Use C{mapToSource(index)} and C{mapFromSource(index)} to map rows in the Queue
to or from rows in the Playlist.

You can't create Queue objects directly.  Instead you have to get a Queue from
an existing playlist.

  >>> queue = clementine.playlists.current().queue()
  >>> queue.Clear()
%End

public:
  static const char* kRowsMimetype;

  bool is_empty() const;
%Docstring
is_empty() -> bool
Checks whether the queue is empty.
%End

  int PositionOf(const QModelIndex& source_index) const;
%Docstring
PositionOf(source_index) -> int
Finds the position in the Queue of a playlist item.

@param source_index: The index of an item in the underlying L{Playlist} model.
@type source_index: L{PyQt4.QtCore.QModelIndex}
@return: The position in the queue of the playlist item, or -1 if the playlist
  item is invalid or is not in the queue.
%End

  bool ContainsSourceRow(int source_row) const;
%Docstring
ContainsSourceRow(source_row) -> bool
Checks whether a playlist row is in the queue.

@param source_row: The row number of an item in the underlying L{Playlist} model.
@type source_row: int
%End

  int PeekNext() const;
%Docstring
PeekNext() -> int
Returns the row number in the L{Playlist} of the next item in the queue, leaving
the Queue unchanged.
%End

  int TakeNext();
%Docstring
TakeNext() -> int
Like L{PeekNext()}, but removes the item from the Queue as well as returning it.
%End

  void ToggleTracks(const QModelIndexList& source_indexes);
%Docstring
ToggleTracks(source_indexes)
Adds or removes the Playlist indexes to/from the queue.

@param source_indexes: A list of indexes from the underlying L{Playlist} model.
@type source_indexes: list of L{PyQt4.QtCore.QModelIndex}es
%End

  void Clear();
%Docstring
Clear()
Removes all the items from the Queue.
%End

  void Move(const QList<int>& proxy_rows, int pos);
%Docstring
Move(proxy_rows, pos)
Moves a list of queued items to a different position in the queue.

Example::

  # Moves the second item to the front.
  queue.Move([1], 0)
  
  # Moves the first three items to the end.
  queue.Move([0, 1, 2], queue.rowCount() - 1)

@param proxy_rows: A list of row numbers of items already in the queue.
@type proxy_rows: list of ints
@param pos: The new position in which to insert the items.
@type pos: int
%End

  void MoveUp(int row);
%Docstring
MoveUp(row)
Moves the queued item up one position in the Queue.

This is equivalent to::

  queue.Move([row], row - 1)
%End

  void MoveDown(int row);
%Docstring
MoveUp(row)
Moves the queued item down one position in the Queue.

This is equivalent to::

  queue.Move([row], row + 2)
%End

private:
  Queue();
};
