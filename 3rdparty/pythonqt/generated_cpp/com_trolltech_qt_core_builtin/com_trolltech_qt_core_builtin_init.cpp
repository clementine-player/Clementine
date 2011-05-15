#include <PythonQt.h>
#include "com_trolltech_qt_core_builtin0.h"

void PythonQt_init_QtCoreBuiltin(PyObject* module) {
PythonQt::priv()->registerCPPClass("QBitArray", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QBitArray>, NULL, module, PythonQt::Type_And|PythonQt::Type_NonZero|PythonQt::Type_InplaceXor|PythonQt::Type_InplaceOr|PythonQt::Type_Xor|PythonQt::Type_RichCompare|PythonQt::Type_Or|PythonQt::Type_InplaceAnd|PythonQt::Type_Invert);
PythonQt::priv()->registerCPPClass("QByteArray", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QByteArray>, NULL, module, PythonQt::Type_NonZero|PythonQt::Type_InplaceAdd|PythonQt::Type_RichCompare|PythonQt::Type_Add);
PythonQt::priv()->registerCPPClass("QDate", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QDate>, NULL, module, PythonQt::Type_NonZero|PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QDateTime", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QDateTime>, NULL, module, PythonQt::Type_NonZero|PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QLine", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QLine>, NULL, module, PythonQt::Type_NonZero|PythonQt::Type_RichCompare|PythonQt::Type_Multiply);
PythonQt::priv()->registerCPPClass("QLineF", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QLineF>, NULL, module, PythonQt::Type_NonZero|PythonQt::Type_RichCompare|PythonQt::Type_Multiply);
PythonQt::priv()->registerCPPClass("QLocale", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QLocale>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QPoint", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QPoint>, NULL, module, PythonQt::Type_Divide|PythonQt::Type_InplaceDivide|PythonQt::Type_NonZero|PythonQt::Type_InplaceSubtract|PythonQt::Type_InplaceAdd|PythonQt::Type_RichCompare|PythonQt::Type_Subtract|PythonQt::Type_InplaceMultiply|PythonQt::Type_Multiply|PythonQt::Type_Add);
PythonQt::priv()->registerCPPClass("QPointF", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QPointF>, NULL, module, PythonQt::Type_Divide|PythonQt::Type_InplaceDivide|PythonQt::Type_NonZero|PythonQt::Type_InplaceSubtract|PythonQt::Type_InplaceAdd|PythonQt::Type_RichCompare|PythonQt::Type_Subtract|PythonQt::Type_InplaceMultiply|PythonQt::Type_Multiply|PythonQt::Type_Add);
PythonQt::priv()->registerCPPClass("QRect", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QRect>, NULL, module, PythonQt::Type_And|PythonQt::Type_NonZero|PythonQt::Type_InplaceOr|PythonQt::Type_RichCompare|PythonQt::Type_Or|PythonQt::Type_InplaceAnd);
PythonQt::priv()->registerCPPClass("QRectF", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QRectF>, NULL, module, PythonQt::Type_And|PythonQt::Type_NonZero|PythonQt::Type_InplaceOr|PythonQt::Type_RichCompare|PythonQt::Type_Or|PythonQt::Type_InplaceAnd);
PythonQt::priv()->registerCPPClass("QRegExp", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QRegExp>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QSize", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QSize>, NULL, module, PythonQt::Type_Divide|PythonQt::Type_InplaceDivide|PythonQt::Type_NonZero|PythonQt::Type_InplaceSubtract|PythonQt::Type_InplaceAdd|PythonQt::Type_RichCompare|PythonQt::Type_Subtract|PythonQt::Type_InplaceMultiply|PythonQt::Type_Multiply|PythonQt::Type_Add);
PythonQt::priv()->registerCPPClass("QSizeF", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QSizeF>, NULL, module, PythonQt::Type_Divide|PythonQt::Type_InplaceDivide|PythonQt::Type_NonZero|PythonQt::Type_InplaceSubtract|PythonQt::Type_InplaceAdd|PythonQt::Type_RichCompare|PythonQt::Type_Subtract|PythonQt::Type_InplaceMultiply|PythonQt::Type_Multiply|PythonQt::Type_Add);
PythonQt::priv()->registerCPPClass("QTime", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QTime>, NULL, module, PythonQt::Type_NonZero|PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QUrl", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_QUrl>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("Qt", "", "QtCore", PythonQtCreateObject<PythonQtWrapper_Qt>, NULL, module, 0);

}
