%ModuleCode
  #include "playlist/playlistitem.h"
  #include "scripting/python/sharedpointermanager.h"

  template<> SharedPointerManager<PlaylistItem>::pointer_map_type*
    SharedPointerManager<PlaylistItem>::_pointer_map(NULL);
%End


class PlaylistItem {
%TypeHeaderCode
  #include "playlist/playlistitem.h"
  #include "scripting/python/sharedpointermanager.h"
%End

%Docstring
Represents a single row in a playlist.

Playlists in Clementine are lists of PlaylistItems.  At a minimum each
PlaylistItem contains some metadata and a URL, but items may also have special
loading behaviour associated with them if playing the item is more complicated
than just loading a URL (for example, Last.fm stations have to request a
special playlist using the Last.fm API).

PlaylistItem is an abstract class and instances of it cannot be created
directly by Python code.  If you want to add items to a playlist you should use
the C{Insert...} methods in L{Playlist}, such as
L{Playlist.InsertSongsOrLibraryItems()}.

You can get individual PlaylistItems from a L{Playlist} using
L{Playlist.item_at()}.  Get the PlaylistItem that is currently playing (in any
playlist) using L{Player.GetCurrentItem()}.  These functions are marked as
returning C{PlaylistItemPtr}s, because in C++ the playlist items are held
inside and managed by smart pointers.  This doesn't affect Python at all
however - you can use a PlaylistItemPtr in just the same way as you would a
PlaylistItem.

  >>> item = clementine.player.GetCurrentItem()
  ... print item.Metadata().title()

The L{options()} field of a PlaylistItem is an ORed combination of flags that
describe the item's behaviour when it is played.  Valid values are:

  - C{Default} - no special behaviour, the L{Url()} is used directly when
    playing the song.
  - C{SpecialPlayBehaviour} - The URL returned by Url() isn't the actual URL of
    the music - the item needs to do something special before it can get an
    actual URL.  Causes StartLoading() to get called when the user wants to
    play.
  - C{ContainsMultipleTracks} - this item might be able to provide another
    track after one finishes, for example in a radio stream.  Causes LoadNext()
    to get called when the next URL is required.
  - C{PauseDisabled} - disables the "pause" action.
  - C{LastFMControls} - enables the last.fm "ban" action.

%End

public:
  enum Option {
    Default,
    SpecialPlayBehaviour,
    ContainsMultipleTracks,
    PauseDisabled,
    LastFMControls,
  };
  typedef QFlags<PlaylistItem::Option> Options;

  struct SpecialLoadResult {
%Docstring
Returned by StartLoading() and LoadNext(), indicates what the player should do
when it wants to load a playlist item that is marked SpecialPlayBehaviour or
ContainsMultipleTracks.

Valid values for the type_ field are:

  - C{NoMoreTracks} - there wasn't a track available, and the player should
    move on to the next playlist item.
  - C{WillLoadAsynchronously} - there might be another track available,
    something will call the player's HandleSpecialLoad() slot later with the
    same original_url.
  - C{TrackAvailable} - There was a track available.  Its url is in media_url.

%End

    enum Type {
      NoMoreTracks,
      WillLoadAsynchronously,
      TrackAvailable,
    };

    SpecialLoadResult(); // Workaround SIP Mercurial 3e647ed0f2a2
    SpecialLoadResult(Type type,
                      const QUrl& original_url = QUrl(),
                      const QUrl& media_url = QUrl());

    Type type_;
    QUrl original_url_;
    QUrl media_url_;
  };

  QString type() const;
%Docstring
type() -> str
Returns a string describing the subclass of PlaylistItem that this item really
is.  This string is stored in the database so the correct subclass can be
recreated again when the playlist is loaded.  Values include:

  - C{Library}
  - C{File}
  - C{Stream}
  - C{Jamendo}
  - C{Magnatune}
  - C{Radio}
%End

  Options options() const;
%Docstring
options() -> L{Options}
Returns the options set on this item.

@return: An ORed combination of L{Option}s.
%End

  void Reload();
%Docstring
Reload()
If this item is backed by a file, re-reads the metadata from the file and
updates the row in the playlist.

@warning: This method is B{blocking} and runs on the GUI thread.  It is better
  to call L{BackgroundReload()} which runs in a separate thread.
%End

  void BackgroundReload();
%Docstring
BackgroundReload()
Like L{Reload()}, but runs in a background thread.
%End

  Song Metadata() const;
%Docstring
Metadata() -> L{Song}
Returns this item's metadata.
%End

  QUrl Url() const;
%Docstring
Url() -> L{PyQt4.QtCore.QUrl}
Returns the URL that contains this item's media.  This is usually derived from
C{Metadata().filename()}.
%End

  void SetTemporaryMetadata(const Song& metadata);
%Docstring
SetTemporaryMetadata(metadata)
Sets some metadata on the item that will override that returned from
L{Metadata()} while the song is playing, and until L{ClearTemporaryMetadata()}
is called.  This is useful for radio streams where the actual metadata is not
known until the stream is loaded.

@note: Some types of PlaylistItem may not allow you to override their metadata.
@type metadata: L{Song}
%End

  void ClearTemporaryMetadata();
%Docstring
ClearTemporaryMetadata()
Clears any metadata set by L{SetTemporaryMetadata()}.
%End

  bool HasTemporaryMetadata() const;
%Docstring
HasTemporaryMetadata() -> bool
Returns true if L{SetTemporaryMetadata()} has been called.
%End

  bool IsLocalLibraryItem() const;
%Docstring
IsLocalLibraryItem() -> bool
Convenience function to check whether this item is from the local
library, as opposed to a device, a file on disk or a stream.
Remember that even if this returns true, the library item might be
invalid so you might want to check that it's id is not equal to -1
before actually using it.
%End

  void SetBackgroundColor(short priority, const QColor& color);
%Docstring
SetBackgroundColor(priority, color)
Adds a background color to this PlaylistItem. PlaylistItem's background
colors affect directly the background color of associated row in the playlist
view. Note that every color comes with a priority.
PlaylistItem can have many background colors set at the same time but only
one will be used - the one with the highest priority. If at any point, the
most important color gets removed (RemoveBackgroundColor()), the next to the
highest one will start taking precedence.

Priority must be unique across Clementine and all of it's plugins so that
no functionality messes with another's background color settings.
Below we list all the currently taken background priorities:
- 1 by Rainbowizer plugin

If it makes sense, one functionality (one plugin) can use many background
priorities at the same time.
%End

  bool HasBackgroundColor(short priority) const;
%Docstring
HasBackgroundColor(priority) -> bool
Checks if this PlaylistItem has a background color with this priority set.
It doesn't need to be the current color, it just needs to exist in the colors
list.
%End

  void RemoveBackgroundColor(short priority);
%Docstring
RemoveBackgroundColor(priority)
Removes a background color with the given priority from this PlaylistItem's
background. If there's no such color, this call will be ignored.

@see: L{SetBackgroundColor()}
%End

  void SetForegroundColor(short priority, const QColor& color);
%Docstring
SetForegroundColor(priority, color)
Adds a foreground color to this PlaylistItem. PlaylistItem's foreground
colors affect directly the font color of associated row in the playlist
view. Note that every color comes with a priority.
PlaylistItem can have many foreground colors set at the same time but
only one will be used - the one with the highest priority. If at any
point, the most important color gets removed (RemoveForegroundColor()),
the next to the highest one will start taking precedence.

Priority must be unique across Clementine and all of it's plugins so that
no functionality messes with another's foreground color settings.
Below we list all the currently taken foreground priorities:
- 100 by dynamic history items
- 200 by unplayable items

If it makes sense, one functionality (one plugin) can use many foreground
priorities at the same time.
  %End

  bool HasForegroundColor(short priority) const;
%Docstring
HasForegroundColor(priority) -> bool
Checks if this PlaylistItem has a foreground color with this priority set.
It doesn't need to be the current color, it just needs to exist in the colors
list.
%End

  void RemoveForegroundColor(short priority);
%Docstring
RemoveForegroundColor(priority)
Removes a foreground color with the given priority from this PlaylistItem's
foreground. If there's no such color, this call will be ignored.

@see: L{SetForegroundColor()}
%End

  ~PlaylistItem();
%MethodCode
  // Don't actually destroy the PlaylistItem, just decrement the reference
  // count of Python's shared_ptr.
  SharedPointerManager<PlaylistItem>::SubRef(sipCpp);
  return;
%End

private:
  PlaylistItem();
};

typedef boost::shared_ptr<PlaylistItem> PlaylistItemPtr;
typedef QList<PlaylistItemPtr> PlaylistItemList;


%MappedType PlaylistItemPtr
{
%TypeHeaderCode
  #include "playlist/playlistitem.h"
  #include "scripting/python/sharedpointermanager.h"
%End

%ConvertFromTypeCode
  if (!sipCpp)
    return NULL;

  // Add an extra reference...
  SharedPointerManager<PlaylistItem>::AddRef(*sipCpp);

  PyObject* o = sipConvertFromType(sipCpp->get(), sipType_PlaylistItem, Py_None);
  return o;
%End

%ConvertToTypeCode
  if(sipIsErr == NULL)
    return PyInstance_Check(sipPy);

  int iserr = 0;
  PlaylistItem* ord = reinterpret_cast<PlaylistItem*>(
    sipForceConvertToType(sipPy, sipType_PlaylistItem, Py_None, SIP_NO_CONVERTORS, NULL, &iserr));

  if (iserr){
    *sipIsErr = 1;
    return 0;
  }

  *sipCppPtr = SharedPointerManager<PlaylistItem>::CreatePointer(ord);
  return 1;
%End
};
