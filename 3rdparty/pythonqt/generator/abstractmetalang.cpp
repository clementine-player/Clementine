/****************************************************************************
**
** Copyright (C) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Script Generator project on Qt Labs.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "abstractmetalang.h"
#include "reporthandler.h"

/*******************************************************************************
 * AbstractMetaType
 */
AbstractMetaType *AbstractMetaType::copy() const
{
    AbstractMetaType *cpy = new AbstractMetaType;

    cpy->setTypeUsagePattern(typeUsagePattern());
    cpy->setConstant(isConstant());
    cpy->setReference(isReference());
    cpy->setIndirections(indirections());
	cpy->setInstantiations(instantiations());
    cpy->setArrayElementCount(arrayElementCount());
    cpy->setOriginalTypeDescription(originalTypeDescription());
    cpy->setOriginalTemplateType(originalTemplateType() ? originalTemplateType()->copy() : 0);

    cpy->setArrayElementType(arrayElementType() ? arrayElementType()->copy() : 0);

    cpy->setTypeEntry(typeEntry());

    return cpy;
}

QString AbstractMetaType::cppSignature() const
{
    QString s;

    if (isConstant())
        s += "const ";

    s += typeEntry()->qualifiedCppName();

    if (hasInstantiationInCpp()) {
        QList<AbstractMetaType *> types = instantiations();
        s += "<";
        for (int i=0; i<types.count(); ++i) {
            if (i > 0)
                s += ", ";
            s += types.at(i)->cppSignature();
        }
        s += " >";
    }

    if (actualIndirections()) {
        s += ' ';
        if (indirections())
            s += QString(indirections(), '*');
        if (isReference())
            s += '&';
    }
    return s;
}

/*******************************************************************************
 * AbstractMetaArgument
 */
AbstractMetaArgument *AbstractMetaArgument::copy() const
{
    AbstractMetaArgument *cpy = new AbstractMetaArgument;
    cpy->setName(AbstractMetaVariable::name());
    cpy->setDefaultValueExpression(defaultValueExpression());
    cpy->setType(type()->copy());
    cpy->setArgumentIndex(argumentIndex());

    return cpy;
}


QString AbstractMetaArgument::argumentName() const
{
    QString n = AbstractMetaVariable::name();
    if (n.isEmpty()) {
        return QString("arg__%2").arg(m_argument_index + 1);
    }
    return n;
}


QString AbstractMetaArgument::indexedName() const
{
    QString n = AbstractMetaVariable::name();
    if (n.isEmpty())
        return argumentName();
    return QString("%1%2").arg(n).arg(m_argument_index);
}

QString AbstractMetaArgument::name() const
{
    Q_ASSERT_X(0, "AbstractMetaArgument::name()", "use argumentName() or indexedName() instead");
    return QString();
}


/*******************************************************************************
 * AbstractMetaFunction
 */
AbstractMetaFunction::~AbstractMetaFunction()
{
    qDeleteAll(m_arguments);
    delete m_type;
}

/*******************************************************************************
 * Indicates that this function has a modification that removes it
 */
bool AbstractMetaFunction::isModifiedRemoved(int types) const
{
    FunctionModificationList mods = modifications(implementingClass());
    foreach (FunctionModification mod, mods) {
        if (!mod.isRemoveModifier())
            continue;

        if ((mod.removal & types) == types)
            return true;
    }

    return false;
}

bool AbstractMetaFunction::needsCallThrough() const
{
    if (ownerClass()->isInterface())
        return false;
    if (referenceCounts(implementingClass()).size() > 0)
        return true;
    if (argumentsHaveNativeId() || !isStatic())
        return true;

    foreach (const AbstractMetaArgument *arg, arguments()) {
        if (arg->type()->isArray() || arg->type()->isTargetLangEnum() || arg->type()->isTargetLangFlags())
            return true;
    }

    if (type() && (type()->isArray() || type()->isTargetLangEnum() || type()->isTargetLangFlags()))
        return true;

    for (int i=-1; i<=arguments().size(); ++i) {
        TypeSystem::Ownership owner = this->ownership(implementingClass(), TypeSystem::TargetLangCode, i);
        if (owner != TypeSystem::InvalidOwnership)
            return true;
    }

    return false;
}

bool AbstractMetaFunction::needsSuppressUncheckedWarning() const
{
    for (int i=-1; i<=arguments().size(); ++i) {
        QList<ReferenceCount> referenceCounts = this->referenceCounts(implementingClass(), i);
        foreach (ReferenceCount referenceCount, referenceCounts) {
            if (referenceCount.action != ReferenceCount::Set)
                return true;
        }
    }
    return false;
}

QString AbstractMetaFunction::marshalledName() const
{
    QString returned = "__qt_" + name();
    AbstractMetaArgumentList arguments = this->arguments();
    foreach (const AbstractMetaArgument *arg, arguments) {
        returned += "_";
        if (arg->type()->isNativePointer()) {
            returned += "nativepointer";
        } else if (arg->type()->isIntegerEnum() || arg->type()->isIntegerFlags()) {
            returned += "int";
        } else {
            returned += arg->type()->name().replace("[]", "_3").replace(".", "_");
        }
    }
    return returned;
}

bool AbstractMetaFunction::operator<(const AbstractMetaFunction &other) const
{
    uint result = compareTo(&other);
    return result & NameLessThan;
}


/*!
    Returns a mask of CompareResult describing how this function is
    compares to another function
*/
uint AbstractMetaFunction::compareTo(const AbstractMetaFunction *other) const
{
    uint result = 0;

    // Enclosing class...
    if (ownerClass() == other->ownerClass()) {
        result |= EqualImplementor;
    }

    // Attributes
    if (attributes() == other->attributes()) {
        result |= EqualAttributes;
    }

    // Compare types
    AbstractMetaType *t = type();
    AbstractMetaType *ot = other->type();
    if ((!t && !ot) || ((t && ot && t->name() == ot->name()))) {
        result |= EqualReturnType;
    }

    // Compare names
    int cmp = originalName().compare(other->originalName());

    if (cmp < 0) {
        result |= NameLessThan;
    } else if (cmp == 0) {
        result |= EqualName;
    }

    // compare name after modification...
    cmp = modifiedName().compare(other->modifiedName());
    if (cmp == 0)
        result |= EqualModifiedName;

    // Compare arguments...
    AbstractMetaArgumentList min_arguments;
    AbstractMetaArgumentList max_arguments;
    if (arguments().size() < other->arguments().size()) {
        min_arguments = arguments();
        max_arguments = other->arguments();
    } else {
        min_arguments = other->arguments();
        max_arguments = arguments();
    }

    int min_count = min_arguments.size();
    int max_count = max_arguments.size();
    bool same = true;
    for (int i=0; i<max_count; ++i) {
        if (i < min_count) {
            const AbstractMetaArgument *min_arg = min_arguments.at(i);
            const AbstractMetaArgument *max_arg = max_arguments.at(i);
            if (min_arg->type()->name() != max_arg->type()->name()
                && (min_arg->defaultValueExpression().isEmpty() || max_arg->defaultValueExpression().isEmpty())) {
                same = false;
                break;
            }
        } else {
            if (max_arguments.at(i)->defaultValueExpression().isEmpty()) {
                same = false;
                break;
            }
        }
    }

    if (same)
        result |= min_count == max_count ? EqualArguments : EqualDefaultValueOverload;

    return result;
}

AbstractMetaFunction *AbstractMetaFunction::copy() const
{
    AbstractMetaFunction *cpy = new AbstractMetaFunction;
    cpy->setName(name());
    cpy->setOriginalName(originalName());
    cpy->setOwnerClass(ownerClass());
    cpy->setImplementingClass(implementingClass());
    cpy->setInterfaceClass(interfaceClass());
    cpy->setFunctionType(functionType());
    cpy->setAttributes(attributes());
    cpy->setDeclaringClass(declaringClass());
    if (type())
        cpy->setType(type()->copy());
    cpy->setConstant(isConstant());
    cpy->setOriginalAttributes(originalAttributes());

    foreach (AbstractMetaArgument *arg, arguments())
        cpy->addArgument(arg->copy());

    Q_ASSERT((!type() && !cpy->type())
             || (type()->instantiations() == cpy->type()->instantiations()));

    return cpy;
}

QStringList AbstractMetaFunction::introspectionCompatibleSignatures(const QStringList &resolvedArguments) const
{
    AbstractMetaArgumentList arguments = this->arguments();
    if (arguments.size() == resolvedArguments.size()) {
        return (QStringList() << QMetaObject::normalizedSignature((name() + "(" + resolvedArguments.join(",") + ")").toUtf8().constData()));
    } else {
        QStringList returned;

        AbstractMetaArgument *argument = arguments.at(resolvedArguments.size());
        QStringList minimalTypeSignature = argument->type()->minimalSignature().split("::");
        for (int i=0; i<minimalTypeSignature.size(); ++i) {
            returned += introspectionCompatibleSignatures(QStringList(resolvedArguments)
                << QStringList(minimalTypeSignature.mid(minimalTypeSignature.size() - i - 1)).join("::"));
        }

        return returned;
    }
}

QString AbstractMetaFunction::signature() const
{
    QString s(m_original_name);

    s += "(";

    for (int i=0; i<m_arguments.count(); ++i) {
        if (i > 0)
            s += ", ";
        AbstractMetaArgument *a = m_arguments.at(i);
        s += a->type()->cppSignature();

        // We need to have the argument names in the qdoc files
        s += " ";
        s += a->argumentName();
    }
    s += ")";

    if (isConstant())
        s += " const";

    return s;
}

int AbstractMetaFunction::actualMinimumArgumentCount() const
{
    AbstractMetaArgumentList arguments = this->arguments();

    int count = 0;
    for (int i=0; i<arguments.size(); ++i && ++count) {
        if (argumentRemoved(i + 1)) --count;
        else if (!arguments.at(i)->defaultValueExpression().isEmpty()) break;
    }

    return count;
}

// Returns reference counts for argument at idx, or all arguments if idx == -2
QList<ReferenceCount> AbstractMetaFunction::referenceCounts(const AbstractMetaClass *cls, int idx) const
{
    QList<ReferenceCount> returned;

    FunctionModificationList mods = this->modifications(cls);
    foreach (FunctionModification mod, mods) {
        QList<ArgumentModification> argument_mods = mod.argument_mods;
        foreach (ArgumentModification argument_mod, argument_mods) {
            if (argument_mod.index != idx && idx != -2)
                continue;
            returned += argument_mod.referenceCounts;
        }
    }

    return returned;
}

QString AbstractMetaFunction::replacedDefaultExpression(const AbstractMetaClass *cls, int key) const
{
    FunctionModificationList modifications = this->modifications(cls);
    foreach (FunctionModification modification, modifications) {
        QList<ArgumentModification> argument_modifications = modification.argument_mods;
        foreach (ArgumentModification argument_modification, argument_modifications) {
            if (argument_modification.index == key
                && !argument_modification.replaced_default_expression.isEmpty()) {
                return argument_modification.replaced_default_expression;
            }
        }
    }

    return QString();
}

bool AbstractMetaFunction::removedDefaultExpression(const AbstractMetaClass *cls, int key) const
{
    FunctionModificationList modifications = this->modifications(cls);
    foreach (FunctionModification modification, modifications) {
        QList<ArgumentModification> argument_modifications = modification.argument_mods;
        foreach (ArgumentModification argument_modification, argument_modifications) {
            if (argument_modification.index == key
                && argument_modification.removed_default_expression) {
                return true;
            }
        }
    }

    return false;
}

bool AbstractMetaFunction::resetObjectAfterUse(int argument_idx) const
{
    const AbstractMetaClass *cls = declaringClass();
    FunctionModificationList modifications = this->modifications(cls);
    foreach (FunctionModification modification, modifications) {
        QList<ArgumentModification> argumentModifications = modification.argument_mods;
        foreach (ArgumentModification argumentModification, argumentModifications) {
            if (argumentModification.index == argument_idx && argumentModification.reset_after_use)
                return true;            
        }
    }

    return false;
}

QString AbstractMetaFunction::nullPointerDefaultValue(const AbstractMetaClass *mainClass, int argument_idx) const
{
    Q_ASSERT(nullPointersDisabled(mainClass, argument_idx));

    const AbstractMetaClass *cls = mainClass;
    if (cls == 0)
        cls = implementingClass();

    do {
        FunctionModificationList modifications = this->modifications(cls);
        foreach (FunctionModification modification, modifications) {
            QList<ArgumentModification> argument_modifications = modification.argument_mods;
            foreach (ArgumentModification argument_modification, argument_modifications) {
                if (argument_modification.index == argument_idx
                    && argument_modification.no_null_pointers) {
                    return argument_modification.null_pointer_default_value;
                }
            }
        }

        cls = cls->baseClass();
    } while (cls != 0 && mainClass == 0); // Once when mainClass != 0, or once for all base classes of implementing class

    return QString();

}

bool AbstractMetaFunction::nullPointersDisabled(const AbstractMetaClass *mainClass, int argument_idx) const
{
    const AbstractMetaClass *cls = mainClass;
    if (cls == 0)
        cls = implementingClass();

    do {
        FunctionModificationList modifications = this->modifications(cls);
        foreach (FunctionModification modification, modifications) {
            QList<ArgumentModification> argument_modifications = modification.argument_mods;
            foreach (ArgumentModification argument_modification, argument_modifications) {
                if (argument_modification.index == argument_idx
                    && argument_modification.no_null_pointers) {
                    return true;
                }
            }
        }

        cls = cls->baseClass();
    } while (cls != 0 && mainClass == 0); // Once when mainClass != 0, or once for all base classes of implementing class

    return false;
}

QString AbstractMetaFunction::conversionRule(TypeSystem::Language language, int key) const
{
    FunctionModificationList modifications = this->modifications(declaringClass());
    foreach (FunctionModification modification, modifications) {
        QList<ArgumentModification> argument_modifications = modification.argument_mods;
        foreach (ArgumentModification argument_modification, argument_modifications) {
            if (argument_modification.index != key)
                continue;

            foreach (CodeSnip snip, argument_modification.conversion_rules) {
                if (snip.language == language && !snip.code().isEmpty())
                    return snip.code();
            }
        }
    }

    return QString();
}

QString AbstractMetaFunction::argumentReplaced(int key) const
{
    FunctionModificationList modifications = this->modifications(declaringClass());
    foreach (FunctionModification modification, modifications) {
        QList<ArgumentModification> argument_modifications = modification.argument_mods;
        foreach (ArgumentModification argument_modification, argument_modifications) {
            if (argument_modification.index == key && !argument_modification.replace_value.isEmpty()) {
                return argument_modification.replace_value;
            }
        }
    }

    return "";
}

bool AbstractMetaFunction::argumentRemoved(int key) const
{
    FunctionModificationList modifications = this->modifications(declaringClass());
    foreach (FunctionModification modification, modifications) {
        QList<ArgumentModification> argument_modifications = modification.argument_mods;
        foreach (ArgumentModification argument_modification, argument_modifications) {
            if (argument_modification.index == key) {
                if (argument_modification.removed) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool AbstractMetaFunction::isVirtualSlot() const
{
    FunctionModificationList modifications = this->modifications(declaringClass());
    foreach (FunctionModification modification, modifications) {
        if (modification.isVirtualSlot())
            return true;
    }

    return false;
}

bool AbstractMetaFunction::disabledGarbageCollection(const AbstractMetaClass *cls, int key) const
{
    FunctionModificationList modifications = this->modifications(cls);
    foreach (FunctionModification modification, modifications) {
        QList<ArgumentModification> argument_modifications = modification.argument_mods;
        foreach (ArgumentModification argument_modification, argument_modifications) {
            if (argument_modification.index != key)
                continue;

            foreach (TypeSystem::Ownership ownership, argument_modification.ownerships.values()) {
                if (ownership == TypeSystem::CppOwnership)
                    return true;
            }

        }
    }

    return false;
}

bool AbstractMetaFunction::isDeprecated() const
{
    FunctionModificationList modifications = this->modifications(declaringClass());
    foreach (FunctionModification modification, modifications) {
        if (modification.isDeprecated())
            return true;
    }
    return false;
}

TypeSystem::Ownership AbstractMetaFunction::ownership(const AbstractMetaClass *cls, TypeSystem::Language language, int key) const
{
    FunctionModificationList modifications = this->modifications(cls);
    foreach (FunctionModification modification, modifications) {
        QList<ArgumentModification> argument_modifications = modification.argument_mods;
        foreach (ArgumentModification argument_modification, argument_modifications) {
            if (argument_modification.index == key)
                return argument_modification.ownerships.value(language, TypeSystem::InvalidOwnership);
        }
    }

    return TypeSystem::InvalidOwnership;
}

bool AbstractMetaFunction::isRemovedFromAllLanguages(const AbstractMetaClass *cls) const
{
    return isRemovedFrom(cls, TypeSystem::All);
}

bool AbstractMetaFunction::isRemovedFrom(const AbstractMetaClass *cls, TypeSystem::Language language) const
{
    FunctionModificationList modifications = this->modifications(cls);
    foreach (FunctionModification modification, modifications) {
        if ((modification.removal & language) == language)
            return true;
    }

    return false;

}

QString AbstractMetaFunction::typeReplaced(int key) const
{
    FunctionModificationList modifications = this->modifications(declaringClass());
    foreach (FunctionModification modification, modifications) {
        QList<ArgumentModification> argument_modifications = modification.argument_mods;
        foreach (ArgumentModification argument_modification, argument_modifications) {
            if (argument_modification.index == key
                && !argument_modification.modified_type.isEmpty()) {
                return argument_modification.modified_type;
            }
        }
    }

    return QString();
}

QString AbstractMetaFunction::minimalSignature() const
{
    if (!m_cached_minimal_signature.isEmpty())
        return m_cached_minimal_signature;

    QString minimalSignature = originalName() + "(";
    AbstractMetaArgumentList arguments = this->arguments();

    for (int i=0; i<arguments.count(); ++i) {
        AbstractMetaType *t = arguments.at(i)->type();

        if (i > 0)
            minimalSignature += ",";

        minimalSignature += t->minimalSignature();
    }
    minimalSignature += ")";
    if (isConstant())
        minimalSignature += "const";

    minimalSignature = QMetaObject::normalizedSignature(minimalSignature.toLocal8Bit().constData());
    m_cached_minimal_signature = minimalSignature;

    return minimalSignature;
}

FunctionModificationList AbstractMetaFunction::modifications(const AbstractMetaClass *implementor) const
{
    Q_ASSERT(implementor);
    return implementor->typeEntry()->functionModifications(minimalSignature());
}

bool AbstractMetaFunction::hasModifications(const AbstractMetaClass *implementor) const
{
    FunctionModificationList mods = modifications(implementor);
    return mods.count() > 0;
}

QString AbstractMetaFunction::modifiedName() const
{
    if (m_cached_modified_name.isEmpty()) {
        FunctionModificationList mods = modifications(implementingClass());
        foreach (FunctionModification mod, mods) {
            if (mod.isRenameModifier()) {
                m_cached_modified_name = mod.renamedToName;
                break;
            }
        }
        if (m_cached_modified_name.isEmpty())
            m_cached_modified_name = name();
    }
    return m_cached_modified_name;
}

QString AbstractMetaFunction::targetLangSignature(bool minimal) const
{
    QString s;

    // Attributes...
    if (!minimal) {
#if 0 // jambi
        if (isPublic()) s += "public ";
        else if (isProtected()) s += "protected ";
        else if (isPrivate()) s += "private ";

//     if (isNative()) s += "native ";
//     else
        if (isFinalInTargetLang()) s += "final ";
        else if (isAbstract()) s += "abstract ";

        if (isStatic()) s += "static ";
#endif
        // Return type
        if (type())
            s += type()->name() + " ";
        else
            s += "void ";
    }

    s += modifiedName();
    s += "(";

    int j = 0;
    for (int i=0; i<m_arguments.size(); ++i) {
        if (argumentRemoved(i+1))
            continue;
        if (j != 0) {
            s += ",";
            if (!minimal)
                s += QLatin1Char(' ');
        }
        s += m_arguments.at(i)->type()->name();

        if (!minimal) {
            s += " ";
            s += m_arguments.at(i)->argumentName();
        }
        ++j;
    }

    s += ")";

    return s;
}


bool function_sorter(AbstractMetaFunction *a, AbstractMetaFunction *b)
{
    return a->signature() < b->signature();
}

/*******************************************************************************
 * AbstractMetaClass
 */
AbstractMetaClass::~AbstractMetaClass()
{
    qDeleteAll(m_functions);
    qDeleteAll(m_fields);
}

/*AbstractMetaClass *AbstractMetaClass::copy() const
{
    AbstractMetaClass *cls = new AbstractMetaClass;
    cls->setAttributes(attributes());
    cls->setBaseClass(baseClass());
    cls->setTypeEntry(typeEntry());
    foreach (AbstractMetaFunction *function, functions()) {
        AbstractMetaFunction *copy = function->copy();
        function->setImplementingClass(cls);
        cls->addFunction(copy);
    }
    cls->setEnums(enums());
    foreach (const AbstractMetaField *field, fields()) {
        AbstractMetaField *copy = field->copy();
        copy->setEnclosingClass(cls);
        cls->addField(copy);
    }
    cls->setInterfaces(interfaces());

    return cls;
}*/

/*******************************************************************************
 * Returns true if this class is a subclass of the given class
 */
bool AbstractMetaClass::inheritsFrom(const AbstractMetaClass *cls) const
{
    Q_ASSERT(cls != 0);

    const AbstractMetaClass *clazz = this;
    while (clazz != 0) {
        if (clazz == cls)
            return true;

        clazz = clazz->baseClass();
    }

    return false;
}

/*******************************************************************************
 * Constructs an interface based on the functions and enums in this
 * class and returns it...
 */
AbstractMetaClass *AbstractMetaClass::extractInterface()
{
    Q_ASSERT(typeEntry()->designatedInterface());

    if (m_extracted_interface == 0) {
        AbstractMetaClass *iface = new AbstractMetaClass;
        iface->setAttributes(attributes());
        iface->setBaseClass(0);
        iface->setPrimaryInterfaceImplementor(this);

        iface->setTypeEntry(typeEntry()->designatedInterface());

        foreach (AbstractMetaFunction *function, functions()) {
            if (!function->isConstructor())
                iface->addFunction(function->copy());
        }

//         iface->setEnums(enums());
//         setEnums(AbstractMetaEnumList());

        foreach (const AbstractMetaField *field, fields()) {
            if (field->isPublic()) {
                AbstractMetaField *new_field = field->copy();
                new_field->setEnclosingClass(iface);
                iface->addField(new_field);
            }
        }

        m_extracted_interface = iface;
        addInterface(iface);
    }

    return m_extracted_interface;
}

/*******************************************************************************
 * Returns a list of all the functions with a given name
 */
AbstractMetaFunctionList AbstractMetaClass::queryFunctionsByName(const QString &name) const
{
    AbstractMetaFunctionList returned;
    AbstractMetaFunctionList functions = this->functions();
    foreach (AbstractMetaFunction *function, functions) {
        if (function->name() == name)
            returned.append(function);
    }

    return returned;
}

bool AbstractMetaClass::hasDefaultIsNull() const 
{
  foreach(const AbstractMetaFunction* fun, queryFunctionsByName("isNull")) {
    if (fun->actualMinimumArgumentCount()==0) {
      return true;
    }
  }
  return false;
}

/*******************************************************************************
 * Returns all reference count modifications for any function in the class
 */
QList<ReferenceCount> AbstractMetaClass::referenceCounts() const
{
    QList<ReferenceCount> returned;

    AbstractMetaFunctionList functions = this->functions();
    foreach (AbstractMetaFunction *function, functions) {
        returned += function->referenceCounts(this);
    }

    return returned;
}

/*******************************************************************************
 * Returns a list of all the functions retrieved during parsing which should
 * be added to the Java API.
 */
AbstractMetaFunctionList AbstractMetaClass::functionsInTargetLang() const
{
    int default_flags = NormalFunctions | Visible | NotRemovedFromTargetLang;

    // Interfaces don't implement functions
    default_flags |= isInterface() ? 0 : ClassImplements;

    // Only public functions in final classes
    // default_flags |= isFinal() ? WasPublic : 0;
    int public_flags = isFinal() ? WasPublic : 0;

    // Constructors
    AbstractMetaFunctionList returned = queryFunctions(Constructors | default_flags | public_flags);

    // Final functions
    returned += queryFunctions(FinalInTargetLangFunctions | NonStaticFunctions | default_flags | public_flags);

    // Virtual functions
    returned += queryFunctions(VirtualInTargetLangFunctions | NonStaticFunctions | default_flags | public_flags);

    // Static functions
    returned += queryFunctions(StaticFunctions | default_flags | public_flags);

    // Empty, private functions, since they aren't caught by the other ones
    returned += queryFunctions(Empty | Invisible);

    return returned;
}

AbstractMetaFunctionList AbstractMetaClass::virtualFunctions() const
{
    AbstractMetaFunctionList list = functionsInShellClass();

    AbstractMetaFunctionList returned;
    foreach (AbstractMetaFunction *f, list) {
        if (!f->isFinalInCpp() || f->isVirtualSlot())
            returned += f;
    }

    return returned;
}

AbstractMetaFunctionList AbstractMetaClass::nonVirtualShellFunctions() const
{
    AbstractMetaFunctionList list = functionsInShellClass();
    AbstractMetaFunctionList returned;
    foreach (AbstractMetaFunction *f, list) {
        if (f->isFinalInCpp() && !f->isVirtualSlot())
            returned += f;
    }

    return returned;
}

/*******************************************************************************
 * Returns a list of all functions that should be declared and implemented in
 * the shell class which is generated as a wrapper on top of the actual C++ class
 */
AbstractMetaFunctionList AbstractMetaClass::functionsInShellClass() const
{
    // Only functions and only protected and public functions
    int default_flags = NormalFunctions | Visible | WasVisible | NotRemovedFromShell;

    // All virtual functions
    AbstractMetaFunctionList returned = queryFunctions(VirtualFunctions | default_flags);

    // All functions explicitly set to be implemented by the shell class
    // (mainly superclass functions that are hidden by other declarations)
    returned += queryFunctions(ForcedShellFunctions | default_flags);

    // All functions explicitly set to be virtual slots
    returned += queryFunctions(VirtualSlots | default_flags);

    return returned;
}

/*******************************************************************************
 * Returns a list of all functions that require a public override function to
 * be generated in the shell class. This includes all functions that were originally
 * protected in the superclass.
 */
AbstractMetaFunctionList AbstractMetaClass::publicOverrideFunctions() const
{
    return queryFunctions(NormalFunctions | WasProtected | FinalInCppFunctions | NotRemovedFromTargetLang)
         + queryFunctions(Signals | WasProtected | FinalInCppFunctions | NotRemovedFromTargetLang);
}

AbstractMetaFunctionList AbstractMetaClass::virtualOverrideFunctions() const
{
    return queryFunctions(NormalFunctions | NonEmptyFunctions | Visible | VirtualInCppFunctions | NotRemovedFromShell) +
           queryFunctions(Signals | NonEmptyFunctions | Visible | VirtualInCppFunctions | NotRemovedFromShell);
}

void AbstractMetaClass::sortFunctions()
{
    qSort(m_functions.begin(), m_functions.end(), function_sorter);
}

void AbstractMetaClass::setFunctions(const AbstractMetaFunctionList &functions)
{
    m_functions = functions;

    // Functions must be sorted by name before next loop
    sortFunctions();

    QString currentName;
    bool hasVirtuals = false;
    AbstractMetaFunctionList final_functions;
    foreach (AbstractMetaFunction *f, m_functions) {
        f->setOwnerClass(this);

        m_has_virtual_slots |= f->isVirtualSlot();
        m_has_virtuals |= !f->isFinal() || f->isVirtualSlot();
        m_has_nonpublic |= !f->isPublic();

        // If we have non-virtual overloads of a virtual function, we have to implement
        // all the overloads in the shell class to override the hiding rule
        if (currentName == f->name()) {
            hasVirtuals = hasVirtuals || !f->isFinal();
            if (f->isFinal())
                final_functions += f;
        } else {
            if (hasVirtuals && final_functions.size() > 0) {
                foreach (AbstractMetaFunction *final_function, final_functions) {
                    *final_function += AbstractMetaAttributes::ForceShellImplementation;

                    QString warn = QString("hiding of function '%1' in class '%2'")
                        .arg(final_function->name()).arg(name());
                    ReportHandler::warning(warn);
                }
            }

            hasVirtuals = !f->isFinal();
            final_functions.clear();
            if (f->isFinal())
                final_functions += f;
            currentName = f->name();
        }
    }

#ifndef QT_NO_DEBUG
    bool duplicate_function = false;
    for (int j=0; j<m_functions.size(); ++j) {
        FunctionModificationList mods = m_functions.at(j)->modifications(m_functions.at(j)->implementingClass());

        bool removed = false;
        foreach (const FunctionModification &mod, mods) {
            if (mod.isRemoveModifier()) {
                removed = true;
                break ;
            }
        }
        if (removed)
            continue ;

        for (int i=0; i<m_functions.size() - 1; ++i) {
            if (j == i)
                continue;

            mods = m_functions.at(i)->modifications(m_functions.at(i)->implementingClass());
            bool removed = false;
            foreach (const FunctionModification &mod, mods) {
                if (mod.isRemoveModifier()) {
                    removed = true;
                    break ;
                }
            }
            if (removed)
                continue ;

            uint cmp = m_functions.at(i)->compareTo(m_functions.at(j));
            if ((cmp & AbstractMetaFunction::EqualName) && (cmp & AbstractMetaFunction::EqualArguments)) {
                printf("%s.%s mostly equal to %s.%s\n",
                       qPrintable(m_functions.at(i)->implementingClass()->typeEntry()->qualifiedCppName()),
                       qPrintable(m_functions.at(i)->signature()),
                       qPrintable(m_functions.at(j)->implementingClass()->typeEntry()->qualifiedCppName()),
                       qPrintable(m_functions.at(j)->signature()));
                duplicate_function = true;
            }
        }
    }
    //Q_ASSERT(!duplicate_function);
#endif
}

bool AbstractMetaClass::hasFieldAccessors() const
{
    foreach (const AbstractMetaField *field, fields()) {
        if (field->getter() || field->setter())
            return true;
    }

    return false;
}

bool AbstractMetaClass::hasDefaultToStringFunction() const
{
    foreach (AbstractMetaFunction *f, queryFunctionsByName("toString")) {
        if (f->actualMinimumArgumentCount() == 0) {
            return true;
        }

    }
    return false;
}

void AbstractMetaClass::addFunction(AbstractMetaFunction *function)
{
    function->setOwnerClass(this);

    if (!function->isDestructor()) {
        m_functions << function;
        qSort(m_functions.begin(), m_functions.end(), function_sorter);
    }


    m_has_virtual_slots |= function->isVirtualSlot();
    m_has_virtuals |= !function->isFinal() || function->isVirtualSlot();
    m_has_nonpublic |= !function->isPublic();
}

bool AbstractMetaClass::hasSignal(const AbstractMetaFunction *other) const
{
    if (!other->isSignal())
        return false;

    foreach (const AbstractMetaFunction *f, functions()) {
        if (f->isSignal() && f->compareTo(other) & AbstractMetaFunction::EqualName)
            return other->modifiedName() == f->modifiedName();
    }

    return false;
}


QString AbstractMetaClass::name() const
{
    return QString(m_type_entry->targetLangName()).replace("::", "_");
}

bool AbstractMetaClass::hasFunction(const QString &str) const
{
    foreach (const AbstractMetaFunction *f, functions())
        if (f->name() == str)
            return true;
    return false;
}

/* Returns true if this class has one or more functions that are
   protected. If a class has protected members we need to generate a
   shell class with public accessors to the protected functions, so
   they can be called from the native functions.
*/
bool AbstractMetaClass::hasProtectedFunctions() const {
    foreach (AbstractMetaFunction *func, m_functions) {
        if (func->isProtected())
            return true;
    }
    return false;
}

bool AbstractMetaClass::generateShellClass() const
{
    return m_force_shell_class ||
        (!isFinal()
         && (hasVirtualFunctions()
             || hasProtectedFunctions()
             || hasFieldAccessors()));
}

QPropertySpec *AbstractMetaClass::propertySpecForRead(const QString &name) const
{
    for (int i=0; i<m_property_specs.size(); ++i)
        if (name == m_property_specs.at(i)->read())
            return m_property_specs.at(i);
    return 0;
}

QPropertySpec *AbstractMetaClass::propertySpecForWrite(const QString &name) const
{
    for (int i=0; i<m_property_specs.size(); ++i)
        if (name == m_property_specs.at(i)->write())
            return m_property_specs.at(i);
    return 0;
}

QPropertySpec *AbstractMetaClass::propertySpecForReset(const QString &name) const
{
    for (int i=0; i<m_property_specs.size(); ++i) {
        if (name == m_property_specs.at(i)->reset())
            return m_property_specs.at(i);
    }
    return 0;
}



static bool functions_contains(const AbstractMetaFunctionList &l, const AbstractMetaFunction *func)
{
    foreach (const AbstractMetaFunction *f, l) {
		if ((f->compareTo(func) & AbstractMetaFunction::PrettySimilar) == AbstractMetaFunction::PrettySimilar)
            return true;
    }
    return false;
}

AbstractMetaField::AbstractMetaField() : m_getter(0), m_setter(0), m_class(0)
{
}

AbstractMetaField::~AbstractMetaField()
{
    delete m_setter;
    delete m_getter;
}
ushort        painters;                        // refcount
AbstractMetaField *AbstractMetaField::copy() const
{
    AbstractMetaField *returned = new AbstractMetaField;
    returned->setEnclosingClass(0);
    returned->setAttributes(attributes());
    returned->setName(name());
    returned->setType(type()->copy());
    returned->setOriginalAttributes(originalAttributes());

    return returned;
}

static QString upCaseFirst(const QString &str) {
    Q_ASSERT(!str.isEmpty());
    QString s = str;
    s[0] = s.at(0).toUpper();
    return s;
}

static AbstractMetaFunction *createXetter(const AbstractMetaField *g, const QString &name, uint type) {
    AbstractMetaFunction *f = new AbstractMetaFunction;



    f->setName(name);
    f->setOriginalName(name);
    f->setOwnerClass(g->enclosingClass());
    f->setImplementingClass(g->enclosingClass());
    f->setDeclaringClass(g->enclosingClass());

    uint attr = AbstractMetaAttributes::Native
                | AbstractMetaAttributes::Final
                | type;
    if (g->isStatic())
        attr |= AbstractMetaAttributes::Static;
    if (g->isPublic())
        attr |= AbstractMetaAttributes::Public;
    else if (g->isProtected())
        attr |= AbstractMetaAttributes::Protected;
    else
        attr |= AbstractMetaAttributes::Private;
    f->setAttributes(attr);
    f->setOriginalAttributes(attr);

    FieldModificationList mods = g->modifications();
    foreach (FieldModification mod, mods) {
        if (mod.isRenameModifier())
            f->setName(mod.renamedTo());
        if (mod.isAccessModifier()) {
            if (mod.isPrivate())
                f->setVisibility(AbstractMetaAttributes::Private);
            else if (mod.isProtected())
                f->setVisibility(AbstractMetaAttributes::Protected);
            else if (mod.isPublic())
                f->setVisibility(AbstractMetaAttributes::Public);
            else if (mod.isFriendly())
                f->setVisibility(AbstractMetaAttributes::Friendly);
        }

    }
    return f;
}

FieldModificationList AbstractMetaField::modifications() const
{
    FieldModificationList mods = enclosingClass()->typeEntry()->fieldModifications();
    FieldModificationList returned;

    foreach (FieldModification mod, mods) {
        if (mod.name == name())
            returned += mod;
    }

    return returned;
}

const AbstractMetaFunction *AbstractMetaField::setter() const
{
    if (m_setter == 0) {
        m_setter = createXetter(this,
                                name(),
                                AbstractMetaAttributes::SetterFunction);
        AbstractMetaArgumentList arguments;
        AbstractMetaArgument *argument = new AbstractMetaArgument;
        argument->setType(type()->copy());
        argument->setName(name());
        arguments.append(argument);
        m_setter->setArguments(arguments);
    }
    return m_setter;
}

const AbstractMetaFunction *AbstractMetaField::getter() const
{
    if (m_getter == 0) {
        m_getter = createXetter(this,
                                name(),
                                AbstractMetaAttributes::GetterFunction);
        m_getter->setType(type());
    }

    return m_getter;
}


bool AbstractMetaClass::hasConstructors() const
{
    return queryFunctions(Constructors).size() != 0;
}

void AbstractMetaClass::addDefaultConstructor()
{
    AbstractMetaFunction *f = new AbstractMetaFunction;
    f->setName(name());
    f->setOwnerClass(this);
    f->setFunctionType(AbstractMetaFunction::ConstructorFunction);
    f->setArguments(AbstractMetaArgumentList());
    f->setDeclaringClass(this);

    uint attr = AbstractMetaAttributes::Native;
    attr |= AbstractMetaAttributes::Public;
    f->setAttributes(attr);
    f->setImplementingClass(this);
    f->setOriginalAttributes(f->attributes());

    addFunction(f);
}

bool AbstractMetaClass::hasFunction(const AbstractMetaFunction *f) const
{
    return functions_contains(m_functions, f);
}

/* Goes through the list of functions and returns a list of all
   functions matching all of the criteria in \a query.
 */

AbstractMetaFunctionList AbstractMetaClass::queryFunctions(uint query) const
{
    AbstractMetaFunctionList functions;

    foreach (AbstractMetaFunction *f, m_functions) {

        if ((query & VirtualSlots) && !f->isVirtualSlot())
            continue;

        if ((query & NotRemovedFromTargetLang) && f->isRemovedFrom(f->implementingClass(), TypeSystem::TargetLangCode)) {
            continue;
        }

        if ((query & NotRemovedFromTargetLang) && !f->isFinal() && f->isRemovedFrom(f->declaringClass(), TypeSystem::TargetLangCode)) {
            continue;
        }

        if ((query & NotRemovedFromShell) && f->isRemovedFrom(f->implementingClass(), TypeSystem::ShellCode)) {
            continue;
        }

        if ((query & NotRemovedFromShell) && !f->isFinal() && f->isRemovedFrom(f->declaringClass(), TypeSystem::ShellCode)) {
            continue;
        }

        if ((query & Visible) && f->isPrivate()) {
            continue;
        }

        if ((query & VirtualInTargetLangFunctions) && f->isFinalInTargetLang()) {
            continue;
        }

        if ((query & Invisible) && !f->isPrivate()) {
            continue;
        }

        if ((query & Empty) && !f->isEmptyFunction()) {
            continue;
        }

        if ((query & WasPublic) && !f->wasPublic()) {
            continue;
        }

        if ((query & WasVisible) && f->wasPrivate()) {
            continue;
        }

        if ((query & WasProtected) && !f->wasProtected()) {
            continue;
        }

        if ((query & ClassImplements) && f->ownerClass() != f->implementingClass()) {
            continue;
        }

        if ((query & Inconsistent) && (f->isFinalInTargetLang() || !f->isFinalInCpp() || f->isStatic())) {
            continue;
        }

        if ((query & FinalInTargetLangFunctions) && !f->isFinalInTargetLang()) {
            continue;
        }

        if ((query & FinalInCppFunctions) && !f->isFinalInCpp()) {
            continue;
        }

        if ((query & VirtualInCppFunctions) && f->isFinalInCpp()) {
            continue;
        }

        if ((query & Signals) && (!f->isSignal())) {
            continue;
        }

        if ((query & ForcedShellFunctions)
            && (!f->isForcedShellImplementation()
                || !f->isFinal())) {
            continue;
        }

        if (((query & Constructors) && (!f->isConstructor()
                                       || f->ownerClass() != f->implementingClass()))
            || (f->isConstructor() && (query & Constructors) == 0)) {
            continue;
        }

        // Destructors are never included in the functions of a class currently
        /*
           if ((query & Destructors) && (!f->isDestructor()
                                       || f->ownerClass() != f->implementingClass())
            || f->isDestructor() && (query & Destructors) == 0) {
            continue;
        }*/

        if ((query & VirtualFunctions) && (f->isFinal() || f->isSignal() || f->isStatic())) {
            continue;
        }

        if ((query & StaticFunctions) && (!f->isStatic() || f->isSignal())) {
            continue;
        }

        if ((query & NonStaticFunctions) && (f->isStatic())) {
            continue;
        }

        if ((query & NonEmptyFunctions) && (f->isEmptyFunction())) {
            continue;
        }

        if ((query & NormalFunctions) && (f->isSignal())) {
            continue;
        }

        if ((query & AbstractFunctions) && !f->isAbstract()) {
            continue;
        }

        functions << f;
    }

//    qDebug() << "queried" << m_type_entry->qualifiedCppName() << "got" << functions.size() << "out of" << m_functions.size();

    return functions;
}


bool AbstractMetaClass::hasInconsistentFunctions() const
{
    return cppInconsistentFunctions().size() > 0;
}

bool AbstractMetaClass::hasSignals() const
{
    return cppSignalFunctions().size() > 0;
}


/**
 * Adds the specified interface to this class by adding all the
 * functions in the interface to this class.
 */
void AbstractMetaClass::addInterface(AbstractMetaClass *interface)
{
    Q_ASSERT(!m_interfaces.contains(interface));
    m_interfaces << interface;

    if (m_extracted_interface != 0 && m_extracted_interface != interface)
        m_extracted_interface->addInterface(interface);

    foreach (AbstractMetaFunction *function, interface->functions())
        if (!hasFunction(function) && !function->isConstructor()) {
            AbstractMetaFunction *cpy = function->copy();
            // We do not want this in PythonQt:
            //cpy->setImplementingClass(this);

            // Setup that this function is an interface class.
            cpy->setInterfaceClass(interface);
            *cpy += AbstractMetaAttributes::InterfaceFunction;

            // Copy the modifications in interface into the implementing classes.
            FunctionModificationList mods = function->modifications(interface);
            foreach  (const FunctionModification &mod, mods) {
                m_type_entry->addFunctionModification(mod);
            }

            // It should be mostly safe to assume that when we implement an interface
            // we don't "pass on" pure virtual functions to our sublcasses...
//             *cpy -= AbstractMetaAttributes::Abstract;

            addFunction(cpy);
        }
}


void AbstractMetaClass::setInterfaces(const AbstractMetaClassList &interfaces)
{
    m_interfaces = interfaces;
}


AbstractMetaEnum *AbstractMetaClass::findEnum(const QString &enumName)
{
    foreach (AbstractMetaEnum *e, m_enums) {
        if (e->name() == enumName)
            return e;
    }

    if (typeEntry()->designatedInterface())
        return extractInterface()->findEnum(enumName);

    return 0;
}




/*!  Recursivly searches for the enum value named \a enumValueName in
  this class and its superclasses and interfaces. Values belonging to
  \a meta_enum are excluded from the search.
*/
AbstractMetaEnumValue *AbstractMetaClass::findEnumValue(const QString &enumValueName, AbstractMetaEnum *meta_enum)
{
    foreach (AbstractMetaEnum *e, m_enums) {
        if (e == meta_enum)
            continue;
        foreach (AbstractMetaEnumValue *v, e->values()) {
            if (v->name() == enumValueName)
                return v;
        }
    }

    if (typeEntry()->designatedInterface())
        return extractInterface()->findEnumValue(enumValueName, meta_enum);

    if (baseClass() != 0)
        return baseClass()->findEnumValue(enumValueName, meta_enum);

    return 0;
}


/*!
 * Searches through all of this class' enums for a value matching the
 * name \a enumValueName. The name is excluding the class/namespace
 * prefix. The function recursivly searches interfaces and baseclasses
 * of this class.
 */
AbstractMetaEnum *AbstractMetaClass::findEnumForValue(const QString &enumValueName)
{
    foreach (AbstractMetaEnum *e, m_enums) {
        foreach (AbstractMetaEnumValue *v, e->values()) {
            if (v->name() == enumValueName)
                return e;
        }
    }

    if (typeEntry()->designatedInterface())
        return extractInterface()->findEnumForValue(enumValueName);

    if (baseClass() != 0)
        return baseClass()->findEnumForValue(enumValueName);

    return 0;
}


static void add_extra_include_for_type(AbstractMetaClass *meta_class, const AbstractMetaType *type)
{

    if (type == 0)
        return;

    Q_ASSERT(meta_class != 0);
    const TypeEntry *entry = (type ? type->typeEntry() : 0);
    if (entry != 0 && entry->isComplex()) {
        const ComplexTypeEntry *centry = static_cast<const ComplexTypeEntry *>(entry);
        ComplexTypeEntry *class_entry = meta_class->typeEntry();
        if (class_entry != 0 && centry->include().isValid())
            class_entry->addExtraInclude(centry->include());
    }

    if (type->hasInstantiations()) {
        QList<AbstractMetaType *> instantiations = type->instantiations();
        foreach (AbstractMetaType *instantiation, instantiations)
            add_extra_include_for_type(meta_class, instantiation);
    }
}

static void add_extra_includes_for_function(AbstractMetaClass *meta_class, const AbstractMetaFunction *meta_function)
{
    Q_ASSERT(meta_class != 0);
    Q_ASSERT(meta_function != 0);
    add_extra_include_for_type(meta_class, meta_function->type());

    AbstractMetaArgumentList arguments = meta_function->arguments();
    foreach (AbstractMetaArgument *argument, arguments)
        add_extra_include_for_type(meta_class, argument->type());
}

void AbstractMetaClass::fixFunctions()
{
    if (m_functions_fixed)
        return;
    else
        m_functions_fixed = true;

    AbstractMetaClass *super_class = baseClass();
    AbstractMetaFunctionList funcs = functions();

//     printf("fix functions for %s\n", qPrintable(name()));

    if (super_class != 0)
        super_class->fixFunctions();
    int iface_idx = 0;
    while (super_class || iface_idx < interfaces().size()) {
//         printf(" - base: %s\n", qPrintable(super_class->name()));

        // Since we always traverse the complete hierarchy we are only
        // interrested in what each super class implements, not what
        // we may have propagated from their base classes again.
        AbstractMetaFunctionList super_funcs;
        if (super_class) {

            // Super classes can never be final
            if (super_class->isFinalInTargetLang()) {
                ReportHandler::warning("Final class '" + super_class->name() + "' set to non-final, as it is extended by other classes");
                *super_class -= AbstractMetaAttributes::FinalInTargetLang;
            }
            super_funcs = super_class->queryFunctions(AbstractMetaClass::ClassImplements);
        } else {
            super_funcs = interfaces().at(iface_idx)->queryFunctions(AbstractMetaClass::NormalFunctions);
        }

        QSet<AbstractMetaFunction *> funcs_to_add;
        for (int sfi=0; sfi<super_funcs.size(); ++sfi) {
            AbstractMetaFunction *sf = super_funcs.at(sfi);

            if (sf->isRemovedFromAllLanguages(sf->implementingClass()))
                continue;

            // we generally don't care about private functions, but we have to get the ones that are
            // virtual in case they override abstract functions.
            bool add = (sf->isNormal() || sf->isSignal() || sf->isEmptyFunction());
            for (int fi=0; fi<funcs.size(); ++fi) {
                AbstractMetaFunction *f = funcs.at(fi);
                if (f->isRemovedFromAllLanguages(f->implementingClass()))
                    continue;

                uint cmp = f->compareTo(sf);

                if (cmp & AbstractMetaFunction::EqualModifiedName) {
//                     printf("   - %s::%s similar to %s::%s %x vs %x\n",
//                            qPrintable(sf->implementingClass()->typeEntry()->qualifiedCppName()),
//                            qPrintable(sf->name()),
//                            qPrintable(f->implementingClass()->typeEntry()->qualifiedCppName()),
//                            qPrintable(f->name()),
//                            sf->attributes(),
//                            f->attributes());

                    add = false;
                    if (cmp & AbstractMetaFunction::EqualArguments) {

//                         if (!(cmp & AbstractMetaFunction::EqualReturnType)) {
//                             ReportHandler::warning(QString("%1::%2 and %3::%4 differ in retur type")
//                                                    .arg(sf->implementingClass()->name())
//                                                    .arg(sf->name())
//                                                    .arg(f->implementingClass()->name())
//                                                    .arg(f->name()));
//                         }

                        // Same function, propegate virtual...
                        if (!(cmp & AbstractMetaFunction::EqualAttributes)) {
                            if (!f->isEmptyFunction()) {
                                if (!sf->isFinalInCpp() && f->isFinalInCpp()) {
                                    *f -= AbstractMetaAttributes::FinalInCpp;
    //                                 printf("   --- inherit virtual\n");
                                }
                                if (!sf->isFinalInTargetLang() && f->isFinalInTargetLang()) {
                                    *f -= AbstractMetaAttributes::FinalInTargetLang;
    //                                 printf("   --- inherit virtual\n");
                                }
                                if (!f->isFinalInTargetLang() && f->isPrivate()) {
                                    f->setFunctionType(AbstractMetaFunction::EmptyFunction);
                                    f->setVisibility(AbstractMetaAttributes::Protected);
                                    *f += AbstractMetaAttributes::FinalInTargetLang;
                                    ReportHandler::warning(QString("private virtual function '%1' in '%2'")
                                        .arg(f->signature())
                                        .arg(f->implementingClass()->name()));
                                }
                            }
                        }

                        if (f->visibility() != sf->visibility()) {
                            QString warn = QString("visibility of function '%1' modified in class '%2'")
                                .arg(f->name()).arg(name());
                            ReportHandler::warning(warn);

                            // If new visibility is private, we can't
                            // do anything. If it isn't, then we
                            // prefer the parent class's visibility
                            // setting for the function.
                            if (!f->isPrivate() && !sf->isPrivate())
                                f->setVisibility(sf->visibility());

                            // Private overrides of abstract functions have to go into the class or
                            // the subclasses will not compile as non-abstract classes.
                            // But they don't need to be implemented, since they can never be called.
                            if (f->isPrivate() && sf->isAbstract()) {
                                f->setFunctionType(AbstractMetaFunction::EmptyFunction);
                                f->setVisibility(sf->visibility());
                                *f += AbstractMetaAttributes::FinalInTargetLang;
                                *f += AbstractMetaAttributes::FinalInCpp;
                            }
                        }

                        // Set the class which first declares this function, afawk
                        f->setDeclaringClass(sf->declaringClass());

                        if (sf->isFinalInTargetLang() && !sf->isPrivate() && !f->isPrivate() && !sf->isStatic() && !f->isStatic()) {
                            // Shadowed funcion, need to make base class
                            // function non-virtual
                            if (f->implementingClass() != sf->implementingClass() && f->implementingClass()->inheritsFrom(sf->implementingClass())) {

                                // Check whether the superclass method has been redefined to non-final

                                bool hasNonFinalModifier = false;
                                bool isBaseImplPrivate = false;
                                FunctionModificationList mods = sf->modifications(sf->implementingClass());
                                foreach (FunctionModification mod, mods) {
                                    if (mod.isNonFinal()) {
                                        hasNonFinalModifier = true;
                                        break;
                                    } else if (mod.isPrivate()) {
                                        isBaseImplPrivate = true;
                                        break;
                                    }
                                }

                                if (!hasNonFinalModifier && !isBaseImplPrivate) {
                                    ReportHandler::warning(QString::fromLatin1("Shadowing: %1::%2 and %3::%4; Java code will not compile")
                                                        .arg(sf->implementingClass()->name())
                                                        .arg(sf->signature())
                                                        .arg(f->implementingClass()->name())
                                                        .arg(f->signature()));
                                }
                            }
                        }

                    }

                    if (cmp & AbstractMetaFunction::EqualDefaultValueOverload) {
                        AbstractMetaArgumentList arguments;
                        if (f->arguments().size() < sf->arguments().size())
                            arguments = sf->arguments();
                        else
                            arguments = f->arguments();

                        for (int i=0; i<arguments.size(); ++i)
                            arguments[i]->setDefaultValueExpression(QString());
                    }


                    // Otherwise we have function shadowing and we can
                    // skip the thing...
                } else if (cmp & AbstractMetaFunction::EqualName && !sf->isSignal()) {

                    // In the case of function shadowing where the function name has been altered to
                    // avoid conflict, we don't copy in the original.
                    add = false;
                }

            }

            if (add)
                funcs_to_add << sf;
        }

        foreach (AbstractMetaFunction *f, funcs_to_add)
            funcs << f->copy();

        if (super_class)
            super_class = super_class->baseClass();
        else
            iface_idx++;
    }

    bool hasPrivateConstructors = false;
    bool hasPublicConstructors = false;
    foreach (AbstractMetaFunction *func, funcs) {
        FunctionModificationList mods = func->modifications(this);
        foreach (const FunctionModification &mod, mods) {
            if (mod.isRenameModifier()) {
//                 qDebug() << name() << func->originalName() << func << " from "
//                          << func->implementingClass()->name() << "renamed to" << mod.renamedTo();
                func->setName(mod.renamedTo());
            }
        }

        // Make sure class is abstract if one of the functions is
        if (func->isAbstract()) {
            (*this) += AbstractMetaAttributes::Abstract;
            (*this) -= AbstractMetaAttributes::Final;
        }

        if (func->isConstructor()) {
            if (func->isPrivate())
                hasPrivateConstructors = true;
            else
                hasPublicConstructors = true;
        }



        // Make sure that we include files for all classes that are in use

        if (!func->isRemovedFrom(this, TypeSystem::ShellCode))
            add_extra_includes_for_function(this, func);
    }

    if (hasPrivateConstructors && !hasPublicConstructors) {
        (*this) += AbstractMetaAttributes::Abstract;
        (*this) -= AbstractMetaAttributes::Final;
    }

    foreach (AbstractMetaFunction *f1, funcs) {
        foreach (AbstractMetaFunction *f2, funcs) {
            if (f1 != f2) {
                uint cmp = f1->compareTo(f2);
                if ((cmp & AbstractMetaFunction::EqualName)
                    && !f1->isFinalInCpp()
                    && f2->isFinalInCpp()) {
                    *f2 += AbstractMetaAttributes::FinalOverload;
//                     qDebug() << f2 << f2->implementingClass()->name() << "::" << f2->name() << f2->arguments().size() << " vs " << f1 << f1->implementingClass()->name() << "::" << f1->name() << f1->arguments().size();
//                     qDebug() << "    " << f2;
//                     AbstractMetaArgumentList f2Args = f2->arguments();
//                     foreach (AbstractMetaArgument *a, f2Args)
//                         qDebug() << "        " << a->type()->name() << a->name();
//                     qDebug() << "    " << f1;
//                     AbstractMetaArgumentList f1Args = f1->arguments();
//                     foreach (AbstractMetaArgument *a, f1Args)
//                         qDebug() << "        " << a->type()->name() << a->name();

                }
            }
        }
    }

    setFunctions(funcs);
}


QString AbstractMetaType::minimalSignature() const
{
    QString minimalSignature;
    if (isConstant())
        minimalSignature += "const ";
    minimalSignature += typeEntry()->qualifiedCppName();
    if (hasInstantiations()) {
        QList<AbstractMetaType *> instantiations = this->instantiations();
        minimalSignature += "<";
        for (int i=0;i<instantiations.size();++i) {
            if (i > 0)
                minimalSignature += ",";
            minimalSignature += instantiations.at(i)->minimalSignature();
        }
        minimalSignature += ">";
    }

    if (isReference())
        minimalSignature += "&";
    for (int j=0; j<indirections(); ++j)
        minimalSignature += "*";

    return minimalSignature;
}

bool AbstractMetaType::hasNativeId() const
{
    return (isQObject() || isValue() || isObject()) && typeEntry()->isNativeIdBased();
}


/*******************************************************************************
 * Other stuff...
 */


AbstractMetaEnum *AbstractMetaClassList::findEnum(const EnumTypeEntry *entry) const
{
    Q_ASSERT(entry->isEnum());

    QString qualified_name = entry->qualifiedCppName();
    int pos = qualified_name.lastIndexOf("::");

    QString enum_name;
    QString class_name;

    if (pos > 0) {
        enum_name = qualified_name.mid(pos + 2);
        class_name = qualified_name.mid(0, pos);
    } else {
        enum_name = qualified_name;
        class_name = TypeDatabase::globalNamespaceClassName(entry);
    }

    AbstractMetaClass *meta_class = findClass(class_name);
    if (!meta_class) {
        ReportHandler::warning(QString("AbstractMeta::findEnum(), unknown class '%1' in '%2'")
                               .arg(class_name).arg(entry->qualifiedCppName()));
        return 0;
    }

    return meta_class->findEnum(enum_name);
}

AbstractMetaEnumValue *AbstractMetaEnumValueList::find(const QString &name) const
{
    for (int i=0; i<size(); ++i) {
        if (name == at(i)->name())
            return at(i);
    }
    return 0;
}

AbstractMetaEnumValue *AbstractMetaClassList::findEnumValue(const QString &name) const
{
    QStringList lst = name.split(QLatin1String("::"));

    Q_ASSERT_X(lst.size() == 2, "AbstractMetaClassList::findEnumValue()", "Expected qualified enum");


    QString prefixName = lst.at(0);
    QString enumName = lst.at(1);

    AbstractMetaClass *cl = findClass(prefixName);
    if (cl)
        return cl->findEnumValue(enumName, 0);

    ReportHandler::warning(QString("no matching enum '%1'").arg(name));
    return 0;
}

/*!
 * Searches the list after a class that mathces \a name; either as
 * C++, Java base name or complete Java package.class name.
 */

AbstractMetaClass *AbstractMetaClassList::findClass(const QString &name) const
{
    if (name.isEmpty())
        return 0;

    foreach (AbstractMetaClass *c, *this) {
        if (c->qualifiedCppName() == name)
            return c;
    }

    foreach (AbstractMetaClass *c, *this) {
        if (c->fullName() == name)
            return c;
    }

    foreach (AbstractMetaClass *c, *this) {
        if (c->name() == name)
            return c;
    }

    return 0;
}
