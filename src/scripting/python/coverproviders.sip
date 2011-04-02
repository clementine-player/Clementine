class CoverProviders /NoDefaultCtors/ {

%TypeHeaderCode
#include "covers/coverprovider.h"
#include "covers/coverproviders.h"
#include "scripting/python/pythonengine.h"
%End

%Docstring
This is a global repository for cover providers.

Each one of those has to register with CoverProviders instance by invoking
"CoverProviders::instance().AddCoverProvider(this)". Providers are automatically
unregistered from the repository when they are deleted.
%End

public:
  void AddCoverProvider(CoverProvider* provider /Transfer/);
%MethodCode
  sipCpp->AddCoverProvider(a0);
  PythonEngine::instance()->RegisterNativeObject(a0);
%End
%Docstring
Let's a cover provider to register itself in the repository.
%End

};
