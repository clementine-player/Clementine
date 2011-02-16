class IconLoader {

%TypeHeaderCode
#include "ui/iconloader.h"
%End

%Docstring
Singleton class that loads icons from the system theme if possible, falling
back on Clementine's builtin icons otherwise.

You can access the IconLoader methods directly, there is no need to create an
instance of the class first.

  >>> clementine.IconLoader.Load("media-pause")

Icons are identified by their name without any extension, and they follow the
U{Freedesktop icon naming specification<http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html#names>}.
You can find the list of icons bundled with Clementine in the
U{data/icons/<http://code.google.com/p/clementine-player/source/browse/#svn%2Ftrunk%2Fdata%2Ficons%2F22x22>}
directory in Clementine's source tree.
%End

public:
  static QIcon Load(const QString& name);
%Docstring
Load(name) -> QIcon

Tries to load an icon with the given name.

@param name: The name of an icon from the Freedesktop icon naming specification.
@type name: C{str}
@return: If the icon was loaded successfully then a valid QIcon is returned.
  Otherwise a null QIcon is returned (C{.isNull()} will return C{True}).
%End

private:
  IconLoader();
};
