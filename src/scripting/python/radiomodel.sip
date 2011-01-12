class RadioModel : QStandardItemModel {

%TypeHeaderCode
#include "radio/radiomodel.h"
#include "scripting/python/pythonengine.h"
%End

public:
  enum Role {
    Role_Type,
    Role_PlayBehaviour,
    Role_Url,
    Role_Title,
    Role_Artist,
    Role_CanLazyLoad,
    Role_Service,
  };

  enum Type {
    Type_Service,
    TypeCount
  };

  enum PlayBehaviour {
    PlayBehaviour_None,
    PlayBehaviour_UseSongLoader,
    PlayBehaviour_SingleItem,
  };

  void AddService(RadioService* service /Transfer/);
%MethodCode
  sipCpp->AddService(a0);
  PythonEngine::instance()->RegisterNativeObject(a0);
%End
  void RemoveService(RadioService* service /TransferBack/);

  bool IsPlayable(const QModelIndex& index) const;

  MergedProxyModel* merged_model() const;
  TaskManager* task_manager() const;

private:
  RadioModel();
};
