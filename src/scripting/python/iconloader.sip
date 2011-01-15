class IconLoader {

%TypeHeaderCode
#include "ui/iconloader.h"
%End

public:
  static QIcon Load(const QString& name);

private:
  IconLoader();
};
