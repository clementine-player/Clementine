#include <PythonQt.h>
#include "clementine0.h"


void PythonQt_init_Clementine(PyObject* module) {
PythonQt::priv()->registerClass(&AlbumCoverFetcherSearch::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_AlbumCoverFetcherSearch>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_AlbumCoverFetcherSearch>, module, 0);
PythonQt::priv()->registerClass(&CoverProvider::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_CoverProvider>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_CoverProvider>, module, 0);
PythonQt::priv()->registerClass(&CoverProviderFactory::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_CoverProviderFactory>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_CoverProviderFactory>, module, 0);
PythonQt::priv()->registerClass(&CoverProviders::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_CoverProviders>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("CoverSearchResults", "", "Clementine", PythonQtCreateObject<PythonQtWrapper_CoverSearchResults>, NULL, module, 0);
PythonQt::priv()->registerClass(&NetworkAccessManager::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_NetworkAccessManager>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_NetworkAccessManager>, module, 0);
PythonQt::priv()->registerClass(&NetworkTimeouts::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_NetworkTimeouts>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_NetworkTimeouts>, module, 0);
PythonQt::priv()->registerClass(&RadioModel::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_RadioModel>, NULL, module, 0);
PythonQt::priv()->registerClass(&RadioService::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_RadioService>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_RadioService>, module, 0);
PythonQt::priv()->registerClass(&ThreadSafeNetworkDiskCache::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_ThreadSafeNetworkDiskCache>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_ThreadSafeNetworkDiskCache>, module, 0);
PythonQt::priv()->registerClass(&UrlHandler::staticMetaObject, "Clementine", PythonQtCreateObject<PythonQtWrapper_UrlHandler>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_UrlHandler>, module, 0);
PythonQt::priv()->registerCPPClass("UrlHandler::LoadResult", "", "Clementine", PythonQtCreateObject<PythonQtWrapper_UrlHandler_LoadResult>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_UrlHandler_LoadResult>, module, 0);

}
