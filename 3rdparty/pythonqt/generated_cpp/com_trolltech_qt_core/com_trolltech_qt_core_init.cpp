#include <PythonQt.h>
#include "com_trolltech_qt_core_init.h"
#include "com_trolltech_qt_core0.h"
#include "com_trolltech_qt_core1.h"
#include "com_trolltech_qt_core2.h"

static void* polymorphichandler_QEvent(const void *ptr, char **class_name)
{
    Q_ASSERT(ptr != 0);
    QEvent *object = (QEvent *)ptr;
    if (object->type() == QEvent::None) {
        *class_name = "QEvent";
        return (QEvent*)object;
    }
    if (object->type() == QEvent::ChildAdded || object->type() == QEvent::ChildPolished || object->type() == QEvent::ChildRemoved) {
        *class_name = "QChildEvent";
        return (QChildEvent*)object;
    }
    if (object->type() == QEvent::StateMachineWrapped) {
        *class_name = "QStateMachine_WrappedEvent";
        return (QStateMachine::WrappedEvent*)object;
    }
    if (object->type() == QEvent::StateMachineSignal) {
        *class_name = "QStateMachine_SignalEvent";
        return (QStateMachine::SignalEvent*)object;
    }
    if (object->type() == QEvent::Timer) {
        *class_name = "QTimerEvent";
        return (QTimerEvent*)object;
    }
    if (object->type() == QEvent::DynamicPropertyChange) {
        *class_name = "QDynamicPropertyChangeEvent";
        return (QDynamicPropertyChangeEvent*)object;
    }
    return NULL;
}

void PythonQt_init_QtCore(PyObject* module) {
PythonQt::priv()->registerClass(&QAbstractAnimation::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QAbstractAnimation>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QAbstractAnimation>, module, 0);
PythonQt::priv()->registerClass(&QAbstractItemModel::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QAbstractItemModel>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QAbstractItemModel>, module, 0);
PythonQt::priv()->registerClass(&QAbstractListModel::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QAbstractListModel>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QAbstractListModel>, module, 0);
PythonQt::priv()->registerClass(&QAbstractState::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QAbstractState>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QAbstractState>, module, 0);
PythonQt::priv()->registerClass(&QAbstractTransition::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QAbstractTransition>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QAbstractTransition>, module, 0);
PythonQt::priv()->registerClass(&QAnimationGroup::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QAnimationGroup>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QAnimationGroup>, module, 0);
PythonQt::priv()->registerCPPClass("QBasicTimer", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QBasicTimer>, NULL, module, 0);
PythonQt::priv()->registerClass(&QBuffer::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QBuffer>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QBuffer>, module, 0);
PythonQt::priv()->registerCPPClass("QByteArrayMatcher", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QByteArrayMatcher>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QChildEvent", "QEvent", "QtCore", PythonQtCreateObject<PythonQtWrapper_QChildEvent>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QChildEvent>, module, 0);
PythonQt::priv()->registerClass(&QCoreApplication::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QCoreApplication>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QCoreApplication>, module, 0);
PythonQt::priv()->registerCPPClass("QCryptographicHash", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QCryptographicHash>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QDataStream", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QDataStream>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QDataStream>, module, 0);
PythonQt::priv()->registerCPPClass("QDir", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QDir>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QDirIterator", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QDirIterator>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QDirIterator>, module, 0);
PythonQt::priv()->registerCPPClass("QDynamicPropertyChangeEvent", "QEvent", "QtCore", PythonQtCreateObject<PythonQtWrapper_QDynamicPropertyChangeEvent>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QEasingCurve", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QEasingCurve>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QEvent", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QEvent>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QEvent>, module, 0);
PythonQt::priv()->registerClass(&QEventLoop::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QEventLoop>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QEventLoop>, module, 0);
PythonQt::priv()->registerClass(&QEventTransition::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QEventTransition>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QEventTransition>, module, 0);
PythonQt::priv()->registerCPPClass("QFactoryInterface", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QFactoryInterface>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QFactoryInterface>, module, 0);
PythonQt::priv()->registerClass(&QFile::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QFile>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QFile>, module, 0);
PythonQt::priv()->registerCPPClass("QFileInfo", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QFileInfo>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerClass(&QFileSystemWatcher::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QFileSystemWatcher>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QFileSystemWatcher>, module, 0);
PythonQt::priv()->registerClass(&QFinalState::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QFinalState>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QFinalState>, module, 0);
PythonQt::priv()->registerClass(&QHistoryState::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QHistoryState>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QHistoryState>, module, 0);
PythonQt::priv()->registerClass(&QIODevice::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QIODevice>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QIODevice>, module, 0);
PythonQt::priv()->registerCPPClass("QLibraryInfo", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QLibraryInfo>, NULL, module, 0);
PythonQt::priv()->registerClass(&QMimeData::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QMimeData>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QMimeData>, module, 0);
PythonQt::priv()->registerCPPClass("QModelIndex", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QModelIndex>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QMutex", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QMutex>, NULL, module, 0);
PythonQt::priv()->registerClass(&QObject::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QObject>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QObject>, module, 0);
PythonQt::priv()->registerClass(&QParallelAnimationGroup::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QParallelAnimationGroup>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QParallelAnimationGroup>, module, 0);
PythonQt::priv()->registerClass(&QPauseAnimation::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QPauseAnimation>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QPauseAnimation>, module, 0);
PythonQt::priv()->registerClass(&QProcess::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QProcess>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QProcess>, module, 0);
PythonQt::priv()->registerCPPClass("QProcessEnvironment", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QProcessEnvironment>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerClass(&QPropertyAnimation::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QPropertyAnimation>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QPropertyAnimation>, module, 0);
PythonQt::priv()->registerCPPClass("QReadWriteLock", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QReadWriteLock>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QRunnable", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QRunnable>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QRunnable>, module, 0);
PythonQt::priv()->registerCPPClass("QSemaphore", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QSemaphore>, NULL, module, 0);
PythonQt::priv()->registerClass(&QSequentialAnimationGroup::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QSequentialAnimationGroup>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QSequentialAnimationGroup>, module, 0);
PythonQt::priv()->registerClass(&QSettings::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QSettings>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QSettings>, module, 0);
PythonQt::priv()->registerClass(&QSignalMapper::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QSignalMapper>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QSignalMapper>, module, 0);
PythonQt::priv()->registerClass(&QSignalTransition::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QSignalTransition>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QSignalTransition>, module, 0);
PythonQt::priv()->registerClass(&QSocketNotifier::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QSocketNotifier>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QSocketNotifier>, module, 0);
PythonQt::priv()->registerClass(&QState::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QState>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QState>, module, 0);
PythonQt::priv()->registerClass(&QStateMachine::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QStateMachine>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QStateMachine>, module, 0);
PythonQt::priv()->registerCPPClass("QStateMachine::SignalEvent", "QEvent", "QtCore", PythonQtCreateObject<PythonQtWrapper_QStateMachine_SignalEvent>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QStateMachine::WrappedEvent", "QEvent", "QtCore", PythonQtCreateObject<PythonQtWrapper_QStateMachine_WrappedEvent>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QStringMatcher", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QStringMatcher>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QSystemSemaphore", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QSystemSemaphore>, NULL, module, 0);
PythonQt::priv()->registerClass(&QTemporaryFile::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QTemporaryFile>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QTemporaryFile>, module, 0);
PythonQt::priv()->registerCPPClass("QTextBoundaryFinder", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QTextBoundaryFinder>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QTextCodec", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QTextCodec>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QTextCodec>, module, 0);
PythonQt::priv()->registerCPPClass("QTextDecoder", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QTextDecoder>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QTextEncoder", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QTextEncoder>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QTextStream", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QTextStream>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QTextStream>, module, 0);
PythonQt::priv()->registerClass(&QThreadPool::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QThreadPool>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QThreadPool>, module, 0);
PythonQt::priv()->registerClass(&QTimeLine::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QTimeLine>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QTimeLine>, module, 0);
PythonQt::priv()->registerClass(&QTimer::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QTimer>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QTimer>, module, 0);
PythonQt::priv()->registerCPPClass("QTimerEvent", "QEvent", "QtCore", PythonQtCreateObject<PythonQtWrapper_QTimerEvent>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QTimerEvent>, module, 0);
PythonQt::priv()->registerClass(&QTranslator::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QTranslator>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QTranslator>, module, 0);
PythonQt::priv()->registerCPPClass("QUuid", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QUuid>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QUuid>, module, PythonQt::Type_NonZero|PythonQt::Type_RichCompare);
PythonQt::priv()->registerClass(&QVariantAnimation::staticMetaObject, "QtCore", PythonQtCreateObject<PythonQtWrapper_QVariantAnimation>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QVariantAnimation>, module, 0);
PythonQt::priv()->registerCPPClass("QWaitCondition", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QWaitCondition>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QXmlStreamEntityResolver", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QXmlStreamEntityResolver>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QXmlStreamEntityResolver>, module, 0);
PythonQt::priv()->registerCPPClass("QtConcurrent", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QtConcurrent>, NULL, module, 0);

PythonQt::self()->addPolymorphicHandler("QEvent", polymorphichandler_QEvent);
}
