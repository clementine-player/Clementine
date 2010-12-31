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

public:
  static PlaylistItem* NewFromType(const QString& type);
  static PlaylistItem* NewFromSongsTable(const QString& table, const Song& song);

  enum Option {
    Default,
    SpecialPlayBehaviour,
    ContainsMultipleTracks,
    PauseDisabled,
    LastFMControls,
  };
  typedef QFlags<PlaylistItem::Option> Options;

  QString type() const;
  Options options() const;

  void Reload();
  void BackgroundReload();

  Song Metadata() const;
  QUrl Url() const;

  void SetTemporaryMetadata(const Song& metadata);
  void ClearTemporaryMetadata();
  bool HasTemporaryMetadata() const;

  void SetDynamicHistory(bool history);
  bool IsDynamicHistory() const;

  bool IsLocalLibraryItem() const;

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
