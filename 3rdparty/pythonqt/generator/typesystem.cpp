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

#include "typesystem.h"
#include "generator.h"

#include "customtypes.h"

#include <reporthandler.h>

#include <memory>

#include <QtXml>

QString strings_Object = QLatin1String("Object");
QString strings_String = QLatin1String("String");
QString strings_Thread = QLatin1String("Thread");
QString strings_char = QLatin1String("char");
QString strings_java_lang = QLatin1String("java.lang");
QString strings_jchar = QLatin1String("jchar");
QString strings_jobject = QLatin1String("jobject");

static void addRemoveFunctionToTemplates(TypeDatabase *db);

class StackElement
{
    public:
    enum ElementType {
        None = 0x0,

        // Type tags (0x1, ... , 0xff)
        ObjectTypeEntry      = 0x1,
        ValueTypeEntry       = 0x2,
        InterfaceTypeEntry   = 0x3,
        NamespaceTypeEntry   = 0x4,
        ComplexTypeEntryMask = 0xf,

        // Non-complex type tags (0x10, 0x20, ... , 0xf0)
        PrimitiveTypeEntry   = 0x10,
        EnumTypeEntry        = 0x20,
        TypeEntryMask        = 0xff,

        // Simple tags (0x100, 0x200, ... , 0xf00)
        ExtraIncludes               = 0x100,
        Include                     = 0x200,
        ModifyFunction              = 0x300,
        ModifyField                 = 0x400,
        Root                        = 0x500,
        CustomMetaConstructor       = 0x600,
        CustomMetaDestructor        = 0x700,
        ArgumentMap                 = 0x800,
        SuppressedWarning           = 0x900,
        Rejection                   = 0xa00,
        LoadTypesystem              = 0xb00,
        RejectEnumValue             = 0xc00,
        Template                    = 0xd00,
        TemplateInstanceEnum        = 0xe00,
        Replace                     = 0xf00,
        SimpleMask                  = 0xf00,

        // Code snip tags (0x1000, 0x2000, ... , 0xf000)
        InjectCode =           0x1000,
        InjectCodeInFunction = 0x2000,
        CodeSnipMask =         0xf000,

        // Function modifier tags (0x010000, 0x020000, ... , 0xf00000)
        Access                   = 0x010000,
        Removal                  = 0x020000,
        Rename                   = 0x040000,
        ModifyArgument           = 0x080000,
        FunctionModifiers        = 0xff0000,

        // Argument modifier tags (0x01000000 ... 0xf0000000)
        ConversionRule           = 0x01000000,
        ReplaceType              = 0x02000000,
        ReplaceDefaultExpression = 0x04000000,
        RemoveArgument           = 0x08000000,
        DefineOwnership          = 0x10000000,
        RemoveDefaultExpression  = 0x20000000,
        NoNullPointers           = 0x40000000,
        ReferenceCount           = 0x80000000,
        ArgumentModifiers        = 0xff000000
    };

    StackElement(StackElement *p) : entry(0), type(None), parent(p){ }

    TypeEntry *entry;
    ElementType type;
    StackElement *parent;

    union {
        TemplateInstance *templateInstance;
        TemplateEntry *templateEntry;
        CustomFunction *customFunction;
    } value;
};

class Handler : public QXmlDefaultHandler
{
public:
    Handler(TypeDatabase *database, bool generate)
        : m_database(database), m_generate(generate ? TypeEntry::GenerateAll : TypeEntry::GenerateForSubclass)
    {
        m_current_enum = 0;
        current = 0;

        tagNames["rejection"] = StackElement::Rejection;
        tagNames["primitive-type"] = StackElement::PrimitiveTypeEntry;
        tagNames["object-type"] = StackElement::ObjectTypeEntry;
        tagNames["value-type"] = StackElement::ValueTypeEntry;
        tagNames["interface-type"] = StackElement::InterfaceTypeEntry;
        tagNames["namespace-type"] = StackElement::NamespaceTypeEntry;
        tagNames["enum-type"] = StackElement::EnumTypeEntry;
        tagNames["extra-includes"] = StackElement::ExtraIncludes;
        tagNames["include"] = StackElement::Include;
        tagNames["inject-code"] = StackElement::InjectCode;
        tagNames["modify-function"] = StackElement::ModifyFunction;
        tagNames["modify-field"] = StackElement::ModifyField;
        tagNames["access"] = StackElement::Access;
        tagNames["remove"] = StackElement::Removal;
        tagNames["rename"] = StackElement::Rename;
        tagNames["typesystem"] = StackElement::Root;
        tagNames["custom-constructor"] = StackElement::CustomMetaConstructor;
        tagNames["custom-destructor"] = StackElement::CustomMetaDestructor;
        tagNames["argument-map"] = StackElement::ArgumentMap;
        tagNames["suppress-warning"] = StackElement::SuppressedWarning;
        tagNames["load-typesystem"] = StackElement::LoadTypesystem;
        tagNames["define-ownership"] = StackElement::DefineOwnership;
        tagNames["replace-default-expression"] = StackElement::ReplaceDefaultExpression;
        tagNames["reject-enum-value"] = StackElement::RejectEnumValue;
        tagNames["replace-type"] = StackElement::ReplaceType;
        tagNames["conversion-rule"] = StackElement::ConversionRule;
        tagNames["modify-argument"] = StackElement::ModifyArgument;
        tagNames["remove-argument"] = StackElement::RemoveArgument;
        tagNames["remove-default-expression"] = StackElement::RemoveDefaultExpression;
        tagNames["template"] = StackElement::Template;
        tagNames["insert-template"] = StackElement::TemplateInstanceEnum;
        tagNames["replace"] = StackElement::Replace;
        tagNames["no-null-pointer"] = StackElement::NoNullPointers;
        tagNames["reference-count"] = StackElement::ReferenceCount;
    }

    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &atts);
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);

    QString errorString() const { return m_error; }
    bool error(const QXmlParseException &exception);
    bool fatalError(const QXmlParseException &exception);
    bool warning(const QXmlParseException &exception);

    bool characters(const QString &ch);

private:
    void fetchAttributeValues(const QString &name, const QXmlAttributes &atts,
                              QHash<QString, QString> *acceptedAttributes);

    bool importFileElement(const QXmlAttributes &atts);
    bool convertBoolean(const QString &, const QString &, bool);

    TypeDatabase *m_database;
    StackElement* current;
    QString m_defaultPackage;
    QString m_defaultSuperclass;
    QString m_error;
    TypeEntry::CodeGeneration m_generate;

    EnumTypeEntry *m_current_enum;

    CodeSnipList m_code_snips;
    FunctionModificationList m_function_mods;
    FieldModificationList m_field_mods;

    QHash<QString, StackElement::ElementType> tagNames;
};

bool Handler::error(const QXmlParseException &e)
{
    qWarning("Error: line=%d, column=%d, message=%s\n",
             e.lineNumber(), e.columnNumber(), qPrintable(e.message()));
    return false;
}

bool Handler::fatalError(const QXmlParseException &e)
{
    qWarning("Fatal error: line=%d, column=%d, message=%s\n",
             e.lineNumber(), e.columnNumber(), qPrintable(e.message()));

    return false;
}

bool Handler::warning(const QXmlParseException &e)
{
    qWarning("Warning: line=%d, column=%d, message=%s\n",
             e.lineNumber(), e.columnNumber(), qPrintable(e.message()));

    return false;
}

void Handler::fetchAttributeValues(const QString &name, const QXmlAttributes &atts,
                                   QHash<QString, QString> *acceptedAttributes)
{
    Q_ASSERT(acceptedAttributes != 0);

    for (int i=0; i<atts.length(); ++i) {
        QString key = atts.localName(i).toLower();
        QString val = atts.value(i);

        if (!acceptedAttributes->contains(key)) {
            ReportHandler::warning(QString("Unknown attribute for '%1': '%2'").arg(name).arg(key));
        } else {
            (*acceptedAttributes)[key] = val;
        }
    }
}

bool Handler::endElement(const QString &, const QString &localName, const QString &)
{
    QString tagName = localName.toLower();
    if(tagName == "import-file")
        return true;

    if (!current)
        return true;

    switch (current->type) {
    case StackElement::ObjectTypeEntry:
    case StackElement::ValueTypeEntry:
    case StackElement::InterfaceTypeEntry:
    case StackElement::NamespaceTypeEntry:
        {
            ComplexTypeEntry *centry = static_cast<ComplexTypeEntry *>(current->entry);
            centry->setFunctionModifications(m_function_mods);
            centry->setFieldModifications(m_field_mods);
            centry->setCodeSnips(m_code_snips);

            if (centry->designatedInterface()) {
                centry->designatedInterface()->setCodeSnips(m_code_snips);
                centry->designatedInterface()->setFunctionModifications(m_function_mods);
            }
            m_code_snips = CodeSnipList();
            m_function_mods = FunctionModificationList();
            m_field_mods = FieldModificationList();
        }
        break;
    case StackElement::CustomMetaConstructor:
        {
            current->entry->setCustomConstructor(*current->value.customFunction);
            delete current->value.customFunction;
        }
        break;
    case StackElement::CustomMetaDestructor:
        {
            current->entry->setCustomDestructor(*current->value.customFunction);
            delete current->value.customFunction;
        }
        break;
    case StackElement::EnumTypeEntry:
        m_current_enum = 0;
        break;
    case StackElement::Template:
        m_database->addTemplate(current->value.templateEntry);
        break;
    case StackElement::TemplateInstanceEnum:
        if(current->parent->type == StackElement::InjectCode){
            m_code_snips.last().addTemplateInstance(current->value.templateInstance);
        }else if(current->parent->type == StackElement::Template){
            current->parent->value.templateEntry->addTemplateInstance(current->value.templateInstance);
        }else if(current->parent->type == StackElement::CustomMetaConstructor || current->parent->type == StackElement::CustomMetaConstructor){
            current->parent->value.customFunction->addTemplateInstance(current->value.templateInstance);
        }else if(current->parent->type == StackElement::ConversionRule){
            m_function_mods.last().argument_mods.last().conversion_rules.last().addTemplateInstance(current->value.templateInstance);
        }else if(current->parent->type == StackElement::InjectCodeInFunction){
            m_function_mods.last().snips.last().addTemplateInstance(current->value.templateInstance);
        }
        break;
    default:
        break;
    }

    StackElement *child = current;
    current=current->parent;
    delete(child);

    return true;
}

bool Handler::characters(const QString &ch)
{
    if(current->type == StackElement::Template){
        current->value.templateEntry->addCode(ch);
        return true;
    }

    if (current->type == StackElement::CustomMetaConstructor || current->type == StackElement::CustomMetaDestructor){
        current->value.customFunction->addCode(ch);
        return true;
    }

    if (current->type == StackElement::ConversionRule){
        m_function_mods.last().argument_mods.last().conversion_rules.last().addCode(ch);
        return true;
    }

    if  (current->parent){
        if ((current->type & StackElement::CodeSnipMask) != 0) {
            switch (current->parent->type) {
                case StackElement::Root:
                    ((TypeSystemTypeEntry *) current->parent->entry)->snips.last().addCode(ch);
                    break;
                case StackElement::ModifyFunction:
                    m_function_mods.last().snips.last().addCode(ch);
                    break;
                case StackElement::NamespaceTypeEntry:
                case StackElement::ObjectTypeEntry:
                case StackElement::ValueTypeEntry:
                case StackElement::InterfaceTypeEntry:
                    m_code_snips.last().addCode(ch);
                    break;
                default:
                    Q_ASSERT(false);
            };
            return true;
        }
    }

    return true;
}

bool Handler::importFileElement(const QXmlAttributes &atts)
{
    QString fileName = atts.value("name");
    if(fileName.isEmpty()){
        m_error = "Required attribute 'name' missing for include-file tag.";
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        file.setFileName(":/trolltech/generator/" + fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_error = QString("Could not open file: '%1'").arg(fileName);
            return false;
        }
    }

    QString quoteFrom = atts.value("quote-after-line");
    bool foundFromOk = quoteFrom.isEmpty();
    bool from = quoteFrom.isEmpty();

    QString quoteTo = atts.value("quote-before-line");
    bool foundToOk = quoteTo.isEmpty();
    bool to = true;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if(from && to && line.contains(quoteTo)) {
            to = false;
            foundToOk = true;
            break;
        }
        if(from && to)
            characters(line + "\n");
        if(!from && line.contains(quoteFrom)) {
            from = true;
            foundFromOk = true;
        }
    }
    if(!foundFromOk || !foundToOk){
        QString fromError = QString("Could not find quote-after-line='%1' in file '%2'.").arg(quoteFrom).arg(fileName);
        QString toError = QString("Could not find quote-before-line='%1' in file '%2'.").arg(quoteTo).arg(fileName);

        if(!foundToOk)
            m_error = toError;
        if(!foundFromOk)
            m_error = fromError;
        if(!foundFromOk && !foundToOk)
            m_error = fromError + " " + toError;
        return false;
    }

    return true;
}

bool Handler::convertBoolean(const QString &_value, const QString &attributeName, bool defaultValue)
{
    QString value = _value.toLower();
    if (value == "true" || value == "yes") {
        return true;
    } else if (value == "false" || value == "no") {
        return false;
    } else {
        QString warn = QString("Boolean value '%1' not supported in attribute '%2'. Use 'yes' or 'no'. Defaulting to '%3'.")
            .arg(value).arg(attributeName).arg(defaultValue ? "yes" : "no");

        ReportHandler::warning(warn);
        return defaultValue;
    }
}

bool Handler::startElement(const QString &, const QString &n,
                           const QString &, const QXmlAttributes &atts)
{
    QString tagName = n.toLower();
    if(tagName == "import-file"){
        return importFileElement(atts);
    }

    std::auto_ptr<StackElement> element(new StackElement(current));

    if (!tagNames.contains(tagName)) {
        m_error = QString("Unknown tag name: '%1'").arg(tagName);
        return false;
    }

    element->type = tagNames[tagName];
    if (element->type & StackElement::TypeEntryMask) {
        if (current->type != StackElement::Root) {
            m_error = "Nested types not supported";
            return false;
        }

        QHash<QString, QString> attributes;
        attributes["name"] = QString();

        switch (element->type) {
        case StackElement::PrimitiveTypeEntry:
            attributes["java-name"] = QString();
            attributes["jni-name"] = QString();
            attributes["preferred-conversion"] = "yes";
            attributes["preferred-java-type"] = "yes";
            break;
        case StackElement::EnumTypeEntry:
            attributes["flags"] = "no";
            attributes["upper-bound"] = QString();
            attributes["lower-bound"] = QString();
            attributes["force-integer"] = "no";
            attributes["extensible"] = "no";

            break;

        case StackElement::ObjectTypeEntry:
        case StackElement::ValueTypeEntry:
            attributes["force-abstract"] = QString("no");
            attributes["deprecated"] = QString("no");
            // fall throooough
        case StackElement::InterfaceTypeEntry:
            attributes["default-superclass"] = m_defaultSuperclass;
            attributes["polymorphic-id-expression"] = QString();
            attributes["delete-in-main-thread"] = QString("no");
            // fall through
        case StackElement::NamespaceTypeEntry:
            attributes["java-name"] = QString();
            attributes["package"] = m_defaultPackage;
            attributes["expense-cost"] = "1";
            attributes["expense-limit"] = "none";
            attributes["polymorphic-base"] = QString("no");
            attributes["generate"] = QString("yes");
            attributes["target-type"] = QString();
            attributes["generic-class"] = QString("no");
            break;
        default:
            ; // nada
        };

        fetchAttributeValues(tagName, atts, &attributes);

        QString name = attributes["name"];

        // We need to be able to have duplicate primitive type entries, or it's not possible to
        // cover all primitive java types (which we need to do in order to support fake
        // meta objects)
        if (element->type != StackElement::PrimitiveTypeEntry) {
            TypeEntry *tmp = m_database->findType(name);
            if (tmp != 0) {
                ReportHandler::warning(QString("Duplicate type entry: '%1'").arg(name));
            }
        }

        if (name.isEmpty()) {
            m_error = "no 'name' attribute specified";
            return false;
        }
        switch (element->type) {
        case StackElement::PrimitiveTypeEntry:
            {
                QString java_name = attributes["java-name"];
                QString jni_name = attributes["jni-name"];
                QString preferred_conversion = attributes["preferred-conversion"].toLower();
                QString preferred_java_type = attributes["preferred-java-type"].toLower();

                if (java_name.isEmpty())
                    java_name = name;
                if (jni_name.isEmpty())
                    jni_name = name;

                PrimitiveTypeEntry *type = new PrimitiveTypeEntry(name);
                type->setCodeGeneration(m_generate);
                type->setTargetLangName(java_name);
                type->setJniName(jni_name);

                type->setPreferredConversion(convertBoolean(preferred_conversion, "preferred-conversion", true));
                type->setPreferredTargetLangType(convertBoolean(preferred_java_type, "preferred-java-type", true));

                element->entry = type;
            }
            break;
        case StackElement::EnumTypeEntry: {
            QStringList names = name.split(QLatin1String("::"));

            if (names.size() == 1) {
                m_current_enum = new EnumTypeEntry(QString(), name);
            }
            else
                m_current_enum =
                    new EnumTypeEntry(QStringList(names.mid(0, names.size() - 1)).join("::"),
                                      names.last());
            element->entry = m_current_enum;
            m_current_enum->setCodeGeneration(m_generate);
            m_current_enum->setTargetLangPackage(m_defaultPackage);
            m_current_enum->setUpperBound(attributes["upper-bound"]);
            m_current_enum->setLowerBound(attributes["lower-bound"]);
            m_current_enum->setForceInteger(convertBoolean(attributes["force-integer"], "force-integer", false));
            m_current_enum->setExtensible(convertBoolean(attributes["extensible"], "extensible", false));

            // put in the flags parallel...
            if (!attributes["flags"].isEmpty() && attributes["flags"].toLower() != "no") {
                FlagsTypeEntry *ftype = new FlagsTypeEntry("QFlags<" + name + ">");
                ftype->setOriginator(m_current_enum);
                ftype->setOriginalName(attributes["flags"]);
                ftype->setCodeGeneration(m_generate);
                QString origname = ftype->originalName();

                QStringList lst = origname.split("::");
                if (QStringList(lst.mid(0, lst.size() - 1)).join("::") != m_current_enum->javaQualifier()) {
                    ReportHandler::warning(QString("enum %1 and flags %2 differ in qualifiers")
                                           .arg(m_current_enum->javaQualifier())
                                           .arg(lst.at(0)));
                }

                ftype->setFlagsName(lst.last());
                m_current_enum->setFlags(ftype);

                m_database->addFlagsType(ftype);
                m_database->addType(ftype);
            }
            }
            break;

        case StackElement::InterfaceTypeEntry:
            {
                ObjectTypeEntry *otype = new ObjectTypeEntry(name);
                QString javaName = attributes["java-name"];
                if (javaName.isEmpty())
                    javaName = name;
                InterfaceTypeEntry *itype =
                    new InterfaceTypeEntry(InterfaceTypeEntry::interfaceName(javaName));

                if (!convertBoolean(attributes["generate"], "generate", true))
                    itype->setCodeGeneration(TypeEntry::GenerateForSubclass);
                else
                    itype->setCodeGeneration(m_generate);
                otype->setDesignatedInterface(itype);
                itype->setOrigin(otype);
                element->entry = otype;
            }
            // fall through
        case StackElement::NamespaceTypeEntry:
            if (element->entry == 0) {
                element->entry = new NamespaceTypeEntry(name);
            }
            // fall through
        case StackElement::ObjectTypeEntry:
            if (element->entry == 0) {
                element->entry = new ObjectTypeEntry(name);
            }
            // fall through
        case StackElement::ValueTypeEntry:
            {
                if (element->entry == 0) {
                    element->entry = new ValueTypeEntry(name);
                }

                ComplexTypeEntry *ctype = static_cast<ComplexTypeEntry *>(element->entry);
                ctype->setTargetLangPackage(attributes["package"]);
                ctype->setDefaultSuperclass(attributes["default-superclass"]);
                ctype->setGenericClass(convertBoolean(attributes["generic-class"], "generic-class", false));

                if (!convertBoolean(attributes["generate"], "generate", true))
                    element->entry->setCodeGeneration(TypeEntry::GenerateForSubclass);
                else
                    element->entry->setCodeGeneration(m_generate);

                QString javaName = attributes["java-name"];
                if (!javaName.isEmpty())
                    ctype->setTargetLangName(javaName);

                // The expense policy
                QString limit = attributes["expense-limit"];
                if (!limit.isEmpty() && limit != "none") {
                    ExpensePolicy ep;
                    ep.limit = limit.toInt();
                    ep.cost = attributes["expense-cost"];
                    ctype->setExpensePolicy(ep);
                }

                ctype->setIsPolymorphicBase(convertBoolean(attributes["polymorphic-base"], "polymorphic-base", false));
                ctype->setPolymorphicIdValue(attributes["polymorphic-id-expression"]);

                if (element->type == StackElement::ObjectTypeEntry || element->type == StackElement::ValueTypeEntry) {
                    if (convertBoolean(attributes["force-abstract"], "force-abstract", false))
                        ctype->setTypeFlags(ctype->typeFlags() | ComplexTypeEntry::ForceAbstract);
                    if (convertBoolean(attributes["deprecated"], "deprecated", false))
                        ctype->setTypeFlags(ctype->typeFlags() | ComplexTypeEntry::Deprecated);
                }

                if (element->type == StackElement::InterfaceTypeEntry ||
                    element->type == StackElement::ValueTypeEntry ||
                    element->type == StackElement::ObjectTypeEntry) {
                    if (convertBoolean(attributes["delete-in-main-thread"], "delete-in-main-thread", false))
			            ctype->setTypeFlags(ctype->typeFlags() | ComplexTypeEntry::DeleteInMainThread);
                }

                QString targetType = attributes["target-type"];
                if (!targetType.isEmpty() && element->entry->isComplex())
                    static_cast<ComplexTypeEntry *>(element->entry)->setTargetType(targetType);

                // ctype->setInclude(Include(Include::IncludePath, ctype->name()));
                ctype = ctype->designatedInterface();
                if (ctype != 0)
                    ctype->setTargetLangPackage(attributes["package"]);

            }
            break;
        default:
            Q_ASSERT(false);
        };

        if (element->entry)
            m_database->addType(element->entry);
        else
            ReportHandler::warning(QString("Type: %1 was rejected by typesystem").arg(name));

    } else if (element->type != StackElement::None) {
        bool topLevel = element->type == StackElement::Root
      || element->type == StackElement::SuppressedWarning
      || element->type == StackElement::Rejection
      || element->type == StackElement::LoadTypesystem
      || element->type == StackElement::InjectCode
      || element->type == StackElement::Template;

        if (!topLevel && current->type == StackElement::Root) {
            m_error = QString("Tag requires parent: '%1'").arg(tagName);
            return false;
        }

        StackElement topElement = current==0 ? StackElement(0) : *current;
        element->entry = topElement.entry;

        QHash<QString, QString> attributes;
        switch (element->type) {
        case StackElement::Root:
            attributes["package"] = QString();
            attributes["default-superclass"] = QString();
            break;
        case StackElement::LoadTypesystem:
            attributes["name"] = QString();
            attributes["generate"] = "yes";
            break;
        case StackElement::NoNullPointers:
            attributes["default-value"] = QString();
            break;
        case StackElement::SuppressedWarning:
            attributes["text"] = QString();
            break;
        case StackElement::ReplaceDefaultExpression:
            attributes["with"] = QString();
            break;
        case StackElement::DefineOwnership:
            attributes["class"] = "java";
            attributes["owner"] = "";
            break;
        case StackElement::ModifyFunction:
            attributes["signature"] = QString();
            attributes["access"] = QString();
            attributes["remove"] = QString();
            attributes["rename"] = QString();
            attributes["deprecated"] = QString("no");
            attributes["associated-to"] = QString();
            attributes["virtual-slot"] = QString("no");
            break;
        case StackElement::ModifyArgument:
            attributes["index"] = QString();
	        attributes["replace-value"] = QString();
            attributes["invalidate-after-use"] = QString("no");
            break;
        case StackElement::ModifyField:
            attributes["name"] = QString();
            attributes["write"] = "true";
            attributes["read"] = "true";
            break;
        case StackElement::Access:
            attributes["modifier"] = QString();
            break;
        case StackElement::Include:
            attributes["file-name"] = QString();
            attributes["location"] = QString();
            break;
        case StackElement::CustomMetaConstructor:
            attributes["name"] = topElement.entry->name().toLower() + "_create";
            attributes["param-name"] = "copy";
            break;
        case StackElement::CustomMetaDestructor:
            attributes["name"] = topElement.entry->name().toLower() + "_delete";
            attributes["param-name"] = "copy";
            break;
        case StackElement::ReplaceType:
            attributes["modified-type"] = QString();
            break;
        case StackElement::InjectCode:
            attributes["class"] = "java";
            attributes["position"] = "beginning";
            break;
        case StackElement::ConversionRule:
            attributes["class"] = "";
            break;
        case StackElement::RejectEnumValue:
            attributes["name"] = "";
            break;
        case StackElement::ArgumentMap:
            attributes["index"] = "1";
            attributes["meta-name"] = QString();
            break;
        case StackElement::Rename:
            attributes["to"] = QString();
            break;
        case StackElement::Rejection:
            attributes["class"] = "*";
            attributes["function-name"] = "*";
            attributes["field-name"] = "*";
            attributes["enum-name"] = "*";
            break;
        case StackElement::Removal:
            attributes["class"] = "all";
            break;
        case StackElement::Template:
            attributes["name"] = QString();
            break;
        case StackElement::TemplateInstanceEnum:
            attributes["name"] = QString();
            break;
        case StackElement::Replace:
            attributes["from"] = QString();
            attributes["to"] = QString();
            break;
        case StackElement::ReferenceCount:
            attributes["action"] = QString();
            attributes["variable-name"] = QString();
            attributes["thread-safe"] = QString("no");
            attributes["declare-variable"] = QString();
            attributes["access"] = QString("private");
            attributes["conditional"] = QString("");
            break;
        default:
            ; // nada
        };

        if (attributes.count() > 0)
            fetchAttributeValues(tagName, atts, &attributes);

        switch (element->type) {
        case StackElement::Root:
            m_defaultPackage = attributes["package"];
            m_defaultSuperclass = attributes["default-superclass"];
            element->type = StackElement::Root;
            element->entry = new TypeSystemTypeEntry(m_defaultPackage);
            TypeDatabase::instance()->addType(element->entry);
            break;
        case StackElement::LoadTypesystem:
            {
                QString name = attributes["name"];
                if (name.isEmpty()) {
                    m_error = "No typesystem name specified";
                    return false;
                }

                if (!m_database->parseFile(name, convertBoolean(attributes["generate"], "generate", true))) {
                    m_error = QString("Failed to parse: '%1'").arg(name);
                    return false;
                }
            }
            break;
        case StackElement::RejectEnumValue: {
            if (!m_current_enum) {
                m_error = "<reject-enum-value> node must be used inside a <enum-type> node";
                return false;
            }
            QString name = attributes["name"];

            bool added = false;
            if (!name.isEmpty()) {
                added = true;
                m_current_enum->addEnumValueRejection(name);
            }

            } break;
        case StackElement::ReplaceType:
            {
                if (topElement.type != StackElement::ModifyArgument) {
                    m_error = "Type replacement can only be specified for argument modifications";
                    return false;
                }

                if (attributes["modified-type"].isEmpty()) {
                    m_error = "Type replacement requires 'modified-type' attribute";
                    return false;
                }

                m_function_mods.last().argument_mods.last().modified_type = attributes["modified-type"];
            }
            break;
        case StackElement::ConversionRule:
            {
                if (topElement.type != StackElement::ModifyArgument) {
                    m_error = "Conversion rules can only be specified for argument modification";
                    return false;
                }

                static QHash<QString, TypeSystem::Language> languageNames;
                if (languageNames.isEmpty()) {
                    languageNames["native"] = TypeSystem::NativeCode;
                    languageNames["shell"] = TypeSystem::ShellCode;
                }

                CodeSnip snip;
                QString languageAttribute = attributes["class"].toLower();
                TypeSystem::Language lang = languageNames.value(languageAttribute, TypeSystem::NoLanguage);
                if (lang == TypeSystem::NoLanguage) {
                    m_error = QString("unsupported class attribute: '%1'").arg(languageAttribute);
                    return false;
                }

                snip.language = lang;
                m_function_mods.last().argument_mods.last().conversion_rules.append(snip);
            }

            break;
        case StackElement::ModifyArgument:
            {
                if (topElement.type != StackElement::ModifyFunction) {
                    m_error = QString::fromLatin1("argument modification requires function"
                                                  " modification as parent, was %1")
                              .arg(topElement.type, 0, 16);
                    return false;
                }

                QString index = attributes["index"];
                if (index == "return")
                    index = "0";
                else if (index == "this")
                    index = "-1";

                bool ok = false;
                int idx = index.toInt(&ok);
                if (!ok) {
                    m_error = QString("Cannot convert '%1' to integer").arg(index);
                    return false;
                }

		        QString replace_value = attributes["replace-value"];

		        if (!replace_value.isEmpty() && idx != 0) {
		            m_error = QString("replace-value is only supported for return values (index=0).");
		            return false;
		        }

		        ArgumentModification argumentModification = ArgumentModification(idx);
		        argumentModification.replace_value = replace_value;
                argumentModification.reset_after_use = convertBoolean(attributes["invalidate-after-use"], "invalidate-after-use", false);
                m_function_mods.last().argument_mods.append(argumentModification);
            }
            break;
        case StackElement::NoNullPointers:
            {
                if (topElement.type != StackElement::ModifyArgument) {
                    m_error = "no-null-pointer requires argument modification as parent";
                    return false;
                }

                m_function_mods.last().argument_mods.last().no_null_pointers = true;
                if (m_function_mods.last().argument_mods.last().index == 0) {
                    m_function_mods.last().argument_mods.last().null_pointer_default_value = attributes["default-value"];
                } else if (!attributes["default-value"].isEmpty()) {
                    ReportHandler::warning("default values for null pointer guards are only effective for return values");
                }
            }
            break;
        case StackElement::DefineOwnership:
            {
                if (topElement.type != StackElement::ModifyArgument) {
                    m_error = "define-ownership requires argument modification as parent";
                    return false;
                }

                static QHash<QString, TypeSystem::Language> languageNames;
                if (languageNames.isEmpty()) {
                    languageNames["java"] = TypeSystem::TargetLangCode;
                    languageNames["shell"] = TypeSystem::ShellCode;
                }

                QString classAttribute = attributes["class"].toLower();
                TypeSystem::Language lang = languageNames.value(classAttribute, TypeSystem::NoLanguage);
                if (lang == TypeSystem::NoLanguage) {
                    m_error = QString("unsupported class attribute: '%1'").arg(classAttribute);
                    return false;
                }

                static QHash<QString, TypeSystem::Ownership> ownershipNames;
                if (ownershipNames.isEmpty()) {
                    ownershipNames["java"] = TypeSystem::TargetLangOwnership;
                    ownershipNames["c++"] = TypeSystem::CppOwnership;
                    ownershipNames["default"] = TypeSystem::DefaultOwnership;
                }

                QString ownershipAttribute = attributes["owner"].toLower();
                TypeSystem::Ownership owner = ownershipNames.value(ownershipAttribute, TypeSystem::InvalidOwnership);
                if (owner == TypeSystem::InvalidOwnership) {
                    m_error = QString("unsupported owner attribute: '%1'").arg(ownershipAttribute);
                    return false;
                }

                m_function_mods.last().argument_mods.last().ownerships[lang] = owner;
            }
            break;
        case StackElement::SuppressedWarning:
            if (attributes["text"].isEmpty())
                ReportHandler::warning("Suppressed warning with no text specified");
            else
                m_database->addSuppressedWarning(attributes["text"]);
            break;
        case StackElement::ArgumentMap:
            {
                if (!(topElement.type & StackElement::CodeSnipMask)) {
                    m_error = "Argument maps requires code injection as parent";
                    return false;
                }

                bool ok;
                int pos = attributes["index"].toInt(&ok);
                if (!ok) {
                    m_error = QString("Can't convert position '%1' to integer")
                              .arg(attributes["position"]);
                    return false;
                }

                if (pos <= 0) {
                    m_error = QString("Argument position %1 must be a positive number").arg(pos);
                    return false;
                }

                QString meta_name = attributes["meta-name"];
                if (meta_name.isEmpty()) {
                    ReportHandler::warning("Empty meta name in argument map");
                }

                if (topElement.type == StackElement::InjectCodeInFunction) {
                    m_function_mods.last().snips.last().argumentMap[pos] = meta_name;
                } else {
                    ReportHandler::warning("Argument maps are only useful for injection of code "
                                           "into functions.");
                }
            }
            break;
        case StackElement::Removal:
            {
                if (topElement.type != StackElement::ModifyFunction) {
                    m_error = "Function modification parent required";
                    return false;
                }

                static QHash<QString, TypeSystem::Language> languageNames;
                if (languageNames.isEmpty()) {
                    languageNames["java"] = TypeSystem::TargetLangAndNativeCode;
                    languageNames["all"] = TypeSystem::All;
                }

                QString languageAttribute = attributes["class"].toLower();
                TypeSystem::Language lang = languageNames.value(languageAttribute, TypeSystem::NoLanguage);
                if (lang == TypeSystem::NoLanguage) {
                    m_error = QString("unsupported class attribute: '%1'").arg(languageAttribute);
                    return false;
                }

                m_function_mods.last().removal = lang;
            }
            break;
        case StackElement::Rename:
        case StackElement::Access:
            {
                if (topElement.type != StackElement::ModifyField
                    && topElement.type != StackElement::ModifyFunction) {
                    m_error = "Function or field modification parent required";
                    return false;
                }

                Modification *mod = 0;
                if (topElement.type == StackElement::ModifyFunction)
                    mod = &m_function_mods.last();
                else
                    mod = &m_field_mods.last();

                QString modifier;
                if (element->type == StackElement::Rename) {
                    modifier = "rename";
                    QString renamed_to = attributes["to"];
                    if (renamed_to.isEmpty()) {
                        m_error = "Rename modifier requires 'to' attribute";
                        return false;
                    }

                    if (topElement.type == StackElement::ModifyFunction)
                        mod->setRenamedTo(renamed_to);
                    else
                        mod->setRenamedTo(renamed_to);
                } else {
                    modifier = attributes["modifier"].toLower();
                }

                if (modifier.isEmpty()) {
                    m_error = "No access modification specified";
                    return false;
                }

                static QHash<QString, FunctionModification::Modifiers> modifierNames;
                if (modifierNames.isEmpty()) {
                    modifierNames["private"] = Modification::Private;
                    modifierNames["public"] = Modification::Public;
                    modifierNames["protected"] = Modification::Protected;
                    modifierNames["friendly"] = Modification::Friendly;
                    modifierNames["rename"] = Modification::Rename;
                    modifierNames["final"] = Modification::Final;
                    modifierNames["non-final"] = Modification::NonFinal;
                }

                if (!modifierNames.contains(modifier)) {
                    m_error = QString("Unknown access modifier: '%1'").arg(modifier);
                    return false;
                }

                mod->modifiers |= modifierNames[modifier];
            }
            break;
        case StackElement::RemoveArgument:
            if (topElement.type != StackElement::ModifyArgument) {
                m_error = "Removing argument requires argument modification as parent";
                return false;
            }

            m_function_mods.last().argument_mods.last().removed = true;

            break;

        case StackElement::ModifyField:
            {
                QString name = attributes["name"];
                if (name.isEmpty())
                    break;
                FieldModification fm;
                fm.name = name;
                fm.modifiers = 0;

                QString read = attributes["read"];
                QString write = attributes["write"];

                if (read == "true") fm.modifiers |= FieldModification::Readable;
                if (write == "true") fm.modifiers |= FieldModification::Writable;

                m_field_mods << fm;
            }
            break;
        case StackElement::ModifyFunction:
            {
                if (!(topElement.type & StackElement::ComplexTypeEntryMask)) {
                    m_error = QString::fromLatin1("Modify function requires complex type as parent"
                                                  ", was=%1").arg(topElement.type, 0, 16);
                    return false;
                }
                QString signature = attributes["signature"];

                signature = QMetaObject::normalizedSignature(signature.toLocal8Bit().constData());
                if (signature.isEmpty()) {
                    m_error = "No signature for modified function";
                    return false;
                }

                FunctionModification mod;
                mod.signature = signature;

                QString access = attributes["access"].toLower();
                if (!access.isEmpty()) {
                    if (access == QLatin1String("private"))
                        mod.modifiers |= Modification::Private;
                    else if (access == QLatin1String("protected"))
                        mod.modifiers |= Modification::Protected;
                    else if (access == QLatin1String("public"))
                        mod.modifiers |= Modification::Public;
                    else if (access == QLatin1String("final"))
                        mod.modifiers |= Modification::Final;
                    else if (access == QLatin1String("non-final"))
                        mod.modifiers |= Modification::NonFinal;
                    else {
                        m_error = QString::fromLatin1("Bad access type '%1'").arg(access);
                        return false;
                    }
                }

                if (convertBoolean(attributes["deprecated"], "deprecated", false)) {
                    mod.modifiers |= Modification::Deprecated;
                }

                QString remove = attributes["remove"].toLower();
                if (!remove.isEmpty()) {
                    if (remove == QLatin1String("all"))
                        mod.removal = TypeSystem::All;
                    else if (remove == QLatin1String("java"))
                        mod.removal = TypeSystem::TargetLangAndNativeCode;
                    else {
                        m_error = QString::fromLatin1("Bad removal type '%1'").arg(remove);
                        return false;
                    }
                }

                QString rename = attributes["rename"];
                if (!rename.isEmpty()) {
                    mod.renamedToName = rename;
                    mod.modifiers |= Modification::Rename;
                }

                QString association = attributes["associated-to"];
                if (!association.isEmpty())
                    mod.association = association;

                mod.modifiers |= (convertBoolean(attributes["virtual-slot"], "virtual-slot", false) ? Modification::VirtualSlot : 0);

                m_function_mods << mod;
            }
            break;
        case StackElement::ReplaceDefaultExpression:
            if (!(topElement.type & StackElement::ModifyArgument)) {
                m_error = "Replace default expression only allowed as child of argument modification";
                return false;
            }

            if (attributes["with"].isEmpty()) {
               m_error = "Default expression replaced with empty string. Use remove-default-expression instead.";
               return false;
            }

            m_function_mods.last().argument_mods.last().replaced_default_expression = attributes["with"];
            break;
        case StackElement::RemoveDefaultExpression:
            m_function_mods.last().argument_mods.last().removed_default_expression = true;
            break;
        case StackElement::CustomMetaConstructor:
        case StackElement::CustomMetaDestructor:
            {
                CustomFunction *func = new CustomFunction(attributes["name"]);
                func->param_name = attributes["param-name"];
                element->value.customFunction = func;
            }
            break;
        case StackElement::ReferenceCount:
            {
                if (topElement.type != StackElement::ModifyArgument) {
                    m_error = "reference-count must be child of modify-argument";
                    return false;
                }

                ReferenceCount rc;
                rc.threadSafe = convertBoolean(attributes["thread-safe"], "thread-safe", false);

                static QHash<QString, ReferenceCount::Action> actions;
                if (actions.isEmpty()) {
                    actions["add"] = ReferenceCount::Add;
                    actions["add-all"] = ReferenceCount::AddAll;
                    actions["remove"] = ReferenceCount::Remove;
                    actions["set"] = ReferenceCount::Set;
                    actions["ignore"] = ReferenceCount::Ignore;
                }
                rc.action = actions.value(attributes["action"].toLower(), ReferenceCount::Invalid);

                rc.variableName = attributes["variable-name"];
                if (rc.action != ReferenceCount::Ignore && rc.variableName.isEmpty()) {
                    m_error = "variable-name attribute must be specified";
                    return false;
                }

                rc.declareVariable = attributes["declare-variable"];
                rc.conditional = attributes["conditional"];

                static QHash<QString, int> accessRights;
                if (accessRights.isEmpty()) {
                    accessRights["private"] = ReferenceCount::Private;
                    accessRights["public"] = ReferenceCount::Public;
                    accessRights["protected"] = ReferenceCount::Protected;
                    accessRights["friendly"] = ReferenceCount::Friendly;
                }
                rc.access = accessRights.value(attributes["access"].toLower(), 0);
                if (rc.access == 0) {
                    m_error = "unrecognized access value: " + attributes["access"];
                    return false;
                }

                if (rc.action == ReferenceCount::Invalid) {
                    m_error = "unrecognized value for action attribute. supported actions:";
                    foreach (QString action, actions.keys())
                        m_error += " " + action;
                }

                m_function_mods.last().argument_mods.last().referenceCounts.append(rc);
            }
            break;
        case StackElement::InjectCode:
            {
                if (((topElement.type & StackElement::ComplexTypeEntryMask) == 0)
                    && (topElement.type != StackElement::ModifyFunction)
                    && (topElement.type != StackElement::Root)) {
                    m_error = "wrong parent type for code injection";
                    return false;
                }

                static QHash<QString, TypeSystem::Language> languageNames;
                if (languageNames.isEmpty()) {
                    languageNames["java"] = TypeSystem::TargetLangCode;
                    languageNames["native"] = TypeSystem::NativeCode;
                    languageNames["shell"] = TypeSystem::ShellCode;
                    languageNames["shell-declaration"] = TypeSystem::ShellDeclaration;
                    languageNames["library-initializer"] = TypeSystem::PackageInitializer;
                    languageNames["destructor-function"] = TypeSystem::DestructorFunction;
                    languageNames["constructors"] = TypeSystem::Constructors;
                    languageNames["interface"] = TypeSystem::Interface;
                  languageNames["pywrap-cpp"] = TypeSystem::PyWrapperCode;
                  languageNames["pywrap-h"] = TypeSystem::PyWrapperDeclaration;
                }

                QString className = attributes["class"].toLower();
                if (!languageNames.contains(className)) {
                    m_error = QString("Invalid class specifier: '%1'").arg(className);
                    return false;
                }


                static QHash<QString, CodeSnip::Position> positionNames;
                if (positionNames.isEmpty()) {
                    positionNames["beginning"] = CodeSnip::Beginning;
                    positionNames["end"] = CodeSnip::End;
                    // QtScript
                    positionNames["prototype-initialization"] = CodeSnip::PrototypeInitialization;
                    positionNames["constructor-initialization"] = CodeSnip::ConstructorInitialization;
                    positionNames["constructor"] = CodeSnip::Constructor;
                }

                QString position = attributes["position"].toLower();
                if (!positionNames.contains(position)) {
                    m_error = QString("Invalid position: '%1'").arg(position);
                    return false;
                }

                CodeSnip snip;
                snip.language = languageNames[className];
                snip.position = positionNames[position];

                if (snip.language == TypeSystem::Interface && topElement.type != StackElement::InterfaceTypeEntry) {
                    m_error = "Interface code injections must be direct child of an interface type entry";
                    return false;
                }

                if (topElement.type == StackElement::ModifyFunction) {
                    FunctionModification mod = m_function_mods.last();
                    if (snip.language == TypeSystem::ShellDeclaration) {
                        m_error = "no function implementation in shell declaration in which to inject code";
                        return false;
                    }

                    m_function_mods.last().snips << snip;
                    element->type = StackElement::InjectCodeInFunction;
                } else if (topElement.type == StackElement::Root) {
                    ((TypeSystemTypeEntry *) element->entry)->snips << snip;

                } else if (topElement.type != StackElement::Root) {
                    m_code_snips << snip;
                }
            }
            break;
        case StackElement::Include:
            {
                QString location = attributes["location"].toLower();

                static QHash<QString, Include::IncludeType> locationNames;
                if (locationNames.isEmpty()) {
                    locationNames["global"] = Include::IncludePath;
                    locationNames["local"] = Include::LocalPath;
                    locationNames["java"] = Include::TargetLangImport;
                }

                if (!locationNames.contains(location)) {
                    m_error = QString("Location not recognized: '%1'").arg(location);
                    return false;
                }

                Include::IncludeType loc = locationNames[location];
                Include inc(loc, attributes["file-name"]);

                ComplexTypeEntry *ctype = static_cast<ComplexTypeEntry *>(element->entry);
                if (topElement.type & StackElement::ComplexTypeEntryMask) {
                    ctype->setInclude(inc);
                } else if (topElement.type == StackElement::ExtraIncludes) {
                    ctype->addExtraInclude(inc);
                } else {
                    m_error = "Only supported parents are complex types and extra-includes";
                    return false;
                }

                inc = ctype->include();
                IncludeList lst = ctype->extraIncludes();
                ctype = ctype->designatedInterface();
                if (ctype != 0) {
                    ctype->setExtraIncludes(lst);
                    ctype->setInclude(inc);
                }
            }
            break;
        case StackElement::Rejection:
            {
                QString cls = attributes["class"];
                QString function = attributes["function-name"];
                QString field = attributes["field-name"];
                QString enum_ = attributes["enum-name"];
                if (cls == "*" && function == "*" && field == "*" && enum_ == "*") {
                    m_error = "bad reject entry, neither 'class', 'function-name' nor "
                              "'field' specified";
                    return false;
                }
                m_database->addRejection(cls, function, field, enum_);
            }
            break;
        case StackElement::Template:
            element->value.templateEntry = new TemplateEntry(attributes["name"]);
            break;
        case StackElement::TemplateInstanceEnum:
            if (!(topElement.type & StackElement::CodeSnipMask) &&
                  (topElement.type != StackElement::Template) &&
                  (topElement.type != StackElement::CustomMetaConstructor) &&
                  (topElement.type != StackElement::CustomMetaDestructor) &&
                  (topElement.type != StackElement::ConversionRule))
            {
                m_error = "Can only insert templates into code snippets, templates, custom-constructors, custom-destructors or conversion-rule.";
                return false;
            }
            element->value.templateInstance = new TemplateInstance(attributes["name"]);
            break;
        case StackElement::Replace:
            if (topElement.type != StackElement::TemplateInstanceEnum) {
                m_error = "Can only insert replace rules into insert-template.";
                return false;
            }
            element->parent->value.templateInstance->addReplaceRule(attributes["from"],attributes["to"]);
            break;
        default:
            break; // nada
        };
    }

    current = element.release();
    return true;
}

TypeDatabase *TypeDatabase::instance()
{
    static TypeDatabase *db = new TypeDatabase();
    return db;
}

TypeDatabase::TypeDatabase() : m_suppressWarnings(true)
{
    addType(new StringTypeEntry("QString"));

    StringTypeEntry *e = new StringTypeEntry("QLatin1String");
    e->setPreferredConversion(false);
    addType(e);

    e = new StringTypeEntry("QStringRef");
    e->setPreferredConversion(false);
    addType(e);

    e = new StringTypeEntry("QXmlStreamStringRef");
    e->setPreferredConversion(false);
    addType(e);

    addType(new CharTypeEntry("QChar"));

    CharTypeEntry *c = new CharTypeEntry("QLatin1Char");
    c->setPreferredConversion(false);
    addType(c);

    {
        VariantTypeEntry *qvariant = new VariantTypeEntry("QVariant");
        qvariant->setCodeGeneration(TypeEntry::GenerateNothing);
        addType(qvariant);
    }

    {
        JObjectWrapperTypeEntry *wrapper = new JObjectWrapperTypeEntry("JObjectWrapper");
        wrapper->setCodeGeneration(TypeEntry::GenerateNothing);
        addType(wrapper);
    }

    addType(new ThreadTypeEntry());
    addType(new VoidTypeEntry());

    // Predefined containers...
    addType(new ContainerTypeEntry("QList", ContainerTypeEntry::ListContainer));
    addType(new ContainerTypeEntry("QStringList", ContainerTypeEntry::StringListContainer));
    addType(new ContainerTypeEntry("QLinkedList", ContainerTypeEntry::LinkedListContainer));
    addType(new ContainerTypeEntry("QVector", ContainerTypeEntry::VectorContainer));
    addType(new ContainerTypeEntry("QStack", ContainerTypeEntry::StackContainer));
    addType(new ContainerTypeEntry("QSet", ContainerTypeEntry::SetContainer));
    addType(new ContainerTypeEntry("QMap", ContainerTypeEntry::MapContainer));
    addType(new ContainerTypeEntry("QHash", ContainerTypeEntry::HashContainer));
    addType(new ContainerTypeEntry("QPair", ContainerTypeEntry::PairContainer));
    addType(new ContainerTypeEntry("QQueue", ContainerTypeEntry::QueueContainer));
    addType(new ContainerTypeEntry("QMultiMap", ContainerTypeEntry::MultiMapContainer));

    // Custom types...
    // ### QtScript: no custom handling of QModelIndex for now
//    addType(new QModelIndexTypeEntry());

    addRemoveFunctionToTemplates(this);
}

bool TypeDatabase::parseFile(const QString &filename, bool generate)
{
    QFile file(filename);
    Q_ASSERT(file.exists());
    QXmlInputSource source(&file);

    int count = m_entries.size();

    QXmlSimpleReader reader;
    Handler handler(this, generate);

    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);

    bool ok = reader.parse(&source, false);

    int newCount = m_entries.size();

    ReportHandler::debugSparse(QString::fromLatin1("Parsed: '%1', %2 new entries")
                               .arg(filename)
                               .arg(newCount - count));

    return ok;
}

QString PrimitiveTypeEntry::javaObjectName() const
{
    static QHash<QString, QString> table;
    if (table.isEmpty()) {
        table["boolean"] = "Boolean";
        table["byte"] = "Byte";
        table["char"] = "Character";
        table["short"] = "Short";
        table["int"] = "Integer";
        table["long"] = "Long";
        table["float"] = "Float";
        table["double"] = "Double";
    }
    Q_ASSERT(table.contains(targetLangName()));
    return table[targetLangName()];
}

ContainerTypeEntry *TypeDatabase::findContainerType(const QString &name)
{
    QString template_name = name;

    int pos = name.indexOf('<');
    if (pos > 0)
        template_name = name.left(pos);

    TypeEntry *type_entry = findType(template_name);
    if (type_entry && type_entry->isContainer())
        return static_cast<ContainerTypeEntry *>(type_entry);
    return 0;
}

PrimitiveTypeEntry *TypeDatabase::findTargetLangPrimitiveType(const QString &java_name)
{
    foreach (QList<TypeEntry *> entries, m_entries.values()) {
        foreach (TypeEntry *e, entries) {
            if (e && e->isPrimitive()) {
                PrimitiveTypeEntry *pe = static_cast<PrimitiveTypeEntry *>(e);
                if (pe->targetLangName() == java_name && pe->preferredConversion())
                    return pe;
            }
        }
    }

    return 0;
}

IncludeList TypeDatabase::extraIncludes(const QString &className)
{
    ComplexTypeEntry *typeEntry = findComplexType(className);
    if (typeEntry != 0)
        return typeEntry->extraIncludes();
    else
        return IncludeList();
}



QString Include::toString() const
{
    if (type == IncludePath)
        return "#include <" + name + '>';
    else if (type == LocalPath)
        return "#include \"" + name + "\"";
    else
        return "import " + name + ";";
}

QString Modification::accessModifierString() const
{
    if (isPrivate()) return "private";
    if (isProtected()) return "protected";
    if (isPublic()) return "public";
    if (isFriendly()) return "friendly";
    return QString();
}

FunctionModificationList ComplexTypeEntry::functionModifications(const QString &signature) const
{
    FunctionModificationList lst;
    for (int i=0; i<m_function_mods.count(); ++i) {
        FunctionModification mod = m_function_mods.at(i);
        if (mod.signature == signature) {
            lst << mod;
        }
    }

    return lst;
}

FieldModification ComplexTypeEntry::fieldModification(const QString &name) const
{
    for (int i=0; i<m_field_mods.size(); ++i)
        if (m_field_mods.at(i).name == name)
            return m_field_mods.at(i);
    FieldModification mod;
    mod.name = name;
    mod.modifiers = FieldModification::Readable | FieldModification::Writable;
    return mod;
}

QString ContainerTypeEntry::javaPackage() const
{
    if (m_type == PairContainer)
        return "com.trolltech.qt";
    return "java.util";
}

QString ContainerTypeEntry::targetLangName() const
{

    switch (m_type) {
    case StringListContainer: return "List";
    case ListContainer: return "List";
    case LinkedListContainer: return "LinkedList";
    case VectorContainer: return "List";
    case StackContainer: return "Stack";
    case QueueContainer: return "Queue";
    case SetContainer: return "Set";
    case MapContainer: return "SortedMap";
    case MultiMapContainer: return "SortedMap";
    case HashContainer: return "HashMap";
        //     case MultiHashCollectio: return "MultiHash";
    case PairContainer: return "QPair";
    default:
        qWarning("bad type... %d", m_type);
        break;
    }
    return QString();
}

QString ContainerTypeEntry::qualifiedCppName() const
{
    if (m_type == StringListContainer)
        return "QStringList";
    return ComplexTypeEntry::qualifiedCppName();
}

QString EnumTypeEntry::javaQualifier() const
{
    TypeEntry *te = TypeDatabase::instance()->findType(m_qualifier);
    if (te != 0)
        return te->targetLangName();
    else
        return m_qualifier;
}

QString EnumTypeEntry::jniName() const
{
    return "jint";
}

QString FlagsTypeEntry::jniName() const
{
    return "jint";
}

void EnumTypeEntry::addEnumValueRedirection(const QString &rejected, const QString &usedValue)
{
    m_enum_redirections << EnumValueRedirection(rejected, usedValue);
}

QString EnumTypeEntry::enumValueRedirection(const QString &value) const
{
    for (int i=0; i<m_enum_redirections.size(); ++i)
        if (m_enum_redirections.at(i).rejected == value)
            return m_enum_redirections.at(i).used;
    return QString();
}

QString FlagsTypeEntry::qualifiedTargetLangName() const
{
    return javaPackage() + "." + m_enum->javaQualifier() + "." + targetLangName();
}


void TypeDatabase::addRejection(const QString &class_name, const QString &function_name,
                                const QString &field_name, const QString &enum_name)
{
    TypeRejection r;
    r.class_name = class_name;
    r.function_name = function_name;
    r.field_name = field_name;
    r.enum_name = enum_name;

    m_rejections << r;
}

bool TypeDatabase::isClassRejected(const QString &class_name)
{
    if (!m_rebuild_classes.isEmpty())
        return !m_rebuild_classes.contains(class_name);

    foreach (const TypeRejection &r, m_rejections)
        if (r.class_name == class_name && r.function_name == "*" && r.field_name == "*" && r.enum_name == "*") {
            return true;
        }
    return false;
}

bool TypeDatabase::isEnumRejected(const QString &class_name, const QString &enum_name)
{
    foreach (const TypeRejection &r, m_rejections) {
        if (r.enum_name == enum_name
            && (r.class_name == class_name || r.class_name == "*")) {
            return true;
        }
    }

    return false;
}

bool TypeDatabase::isFunctionRejected(const QString &class_name, const QString &function_name)
{
    foreach (const TypeRejection &r, m_rejections)
        if (r.function_name == function_name &&
            (r.class_name == class_name || r.class_name == "*"))
            return true;
    return false;
}


bool TypeDatabase::isFieldRejected(const QString &class_name, const QString &field_name)
{
    foreach (const TypeRejection &r, m_rejections)
        if (r.field_name == field_name &&
            (r.class_name == class_name || r.class_name == "*"))
            return true;
    return false;
}

FlagsTypeEntry *TypeDatabase::findFlagsType(const QString &name) const
{
    FlagsTypeEntry *fte = (FlagsTypeEntry *) findType(name);
    return fte ? fte : (FlagsTypeEntry *) m_flags_entries.value(name);
}

QString TypeDatabase::globalNamespaceClassName(const TypeEntry * /*entry*/) {
    return QLatin1String("Global");
}


/*!
 * The Visual Studio 2002 compiler doesn't support these symbols,
 * which our typedefs unforntuatly expand to.
 */
QString fixCppTypeName(const QString &name)
{
    if (name == "long long") return "qint64";
    else if (name == "unsigned long long") return "quint64";
    return name;
}

QString formattedCodeHelper(QTextStream &s, Indentor &indentor, QStringList &lines) {
    bool multilineComment = false;
    bool lastEmpty = true;
    QString lastLine;
    while (!lines.isEmpty()) {
        const QString line = lines.takeFirst().trimmed();
        if (line.isEmpty()) {
            if (!lastEmpty)
                s << endl;
            lastEmpty = true;
            continue;
        } else {
            lastEmpty = false;
        }
        if (line.startsWith("/*"))
            multilineComment = true;

        if (multilineComment) {
            s << indentor;
            if (line.startsWith("*"))
                s << " ";
            s << line << endl;
            if (line.endsWith("*/"))
                multilineComment = false;
        } else if (line.startsWith("}")) {
            return line;
        } else if (line.endsWith("}")) {
            s << indentor << line << endl;
            return 0;
        } else if(line.endsWith("{")) {
            s << indentor << line << endl;
            QString tmp;
            {
                Indentation indent(indentor);
                tmp = formattedCodeHelper(s, indentor, lines);
            }
            if (!tmp.isNull()) {
                s << indentor << tmp << endl;
            }
            lastLine = tmp;
            continue;
        } else {
            s << indentor;
            if (!lastLine.isEmpty() &&
                !lastLine.endsWith(";") &&
                !line.startsWith("@") &&
                !line.startsWith("//") &&
                !lastLine.startsWith("//") &&
                !lastLine.endsWith("}") &&
                !line.startsWith("{"))
                s << "    ";
            s << line << endl;
        }
        lastLine = line;
    }
    return 0;
}


QTextStream &CodeSnip::formattedCode(QTextStream &s, Indentor &indentor) const
{
    QStringList lst(code().split("\n"));
    while (!lst.isEmpty()) {
        QString tmp = formattedCodeHelper(s, indentor, lst);
        if (!tmp.isNull()) {
            s << indentor << tmp << endl;
        }
    }
    s.flush();
    return s;
}

QString TemplateInstance::expandCode() const{
    TemplateEntry *templateEntry = TypeDatabase::instance()->findTemplate(m_name);
    if(templateEntry){
        QString res = templateEntry->code();
        foreach(QString key, replaceRules.keys()){
            res.replace(key, replaceRules[key]);
        }
        return "// TEMPLATE - " + m_name + " - START" + res + "// TEMPLATE - " + m_name + " - END";
    }
    else{
        ReportHandler::warning("insert-template referring to non-existing template '" + m_name + "'");
    }
    return QString();
}


QString CodeSnipAbstract::code() const{
    QString res;
    foreach(CodeSnipFragment *codeFrag, codeList){
        res.append(codeFrag->code());
    }
    return res;
}

QString CodeSnipFragment::code() const{
    if(m_instance)
        return m_instance->expandCode();
    else
        return m_code;
}

QString FunctionModification::toString() const
{
    QString str = signature + QLatin1String("->");
    if (modifiers & AccessModifierMask) {
        switch (modifiers & AccessModifierMask) {
        case Private: str += QLatin1String("private"); break;
        case Protected: str += QLatin1String("protected"); break;
        case Public: str += QLatin1String("public"); break;
        case Friendly: str += QLatin1String("friendly"); break;
        }
    }

    if (modifiers & Final) str += QLatin1String("final");
    if (modifiers & NonFinal) str += QLatin1String("non-final");

    if (modifiers & Readable) str += QLatin1String("readable");
    if (modifiers & Writable) str += QLatin1String("writable");

    if (modifiers & CodeInjection) {
        foreach (CodeSnip s, snips) {
            str += QLatin1String("\n//code injection:\n");
            str += s.code();
        }
    }

    if (modifiers & Rename) str += QLatin1String("renamed:") + renamedToName;

    if (modifiers & Deprecated) str += QLatin1String("deprecate");

    if (modifiers & ReplaceExpression) str += QLatin1String("replace-expression");

    return str;
}

static void removeFunction(ComplexTypeEntry *e, const char *signature)
{
    FunctionModification mod;
    mod.signature = QMetaObject::normalizedSignature(signature);
    mod.removal = TypeSystem::All;

    e->addFunctionModification(mod);
}




static void injectCode(ComplexTypeEntry *e,
                       const char *signature,
                       const QByteArray &code,
                       const ArgumentMap &args)
{
    CodeSnip snip;
    snip.language = TypeSystem::NativeCode;
    snip.position = CodeSnip::Beginning;
    snip.addCode(QString::fromLatin1(code));
    snip.argumentMap = args;

    FunctionModification mod;
    mod.signature = QMetaObject::normalizedSignature(signature);
    mod.snips << snip;
    mod.modifiers = Modification::CodeInjection;
    e->addFunctionModification(mod);
}


static void addRemoveFunctionToTemplates(TypeDatabase *db)
{
    ContainerTypeEntry *qvector = db->findContainerType(QLatin1String("QVector"));
    removeFunction(qvector, "constData() const");
    removeFunction(qvector, "data() const");
    removeFunction(qvector, "data()");
    removeFunction(qvector, "first()");
    removeFunction(qvector, "last()");
    removeFunction(qvector, "operator[](int)");
    removeFunction(qvector, "operator[](int) const");
    removeFunction(qvector, "operator=(QVector<T>)");

    ContainerTypeEntry *qlist = db->findContainerType(QLatin1String("QList"));
    removeFunction(qlist, "constData() const");
    removeFunction(qlist, "data() const");
    removeFunction(qlist, "data()");
    removeFunction(qlist, "back()");
    removeFunction(qlist, "front()");
    removeFunction(qlist, "first()");
    removeFunction(qlist, "last()");
    removeFunction(qlist, "operator[](int)");
    removeFunction(qlist, "operator[](int) const");
    removeFunction(qlist, "operator=(QList<T>)");

    ContainerTypeEntry *qqueue = db->findContainerType(QLatin1String("QQueue"));
    removeFunction(qqueue, "head() const");

    // QtScript: The next part is Java-specific, skip it for now...
    return;

    ArgumentMap args1;
    args1[1] = QLatin1String("$1");
    ArgumentMap args2 = args1;
    args2[2] = QLatin1String("$2");

    QByteArray code =
        "\nif ($1 >= __qt_this->size() || $1 < 0) {"
        "\n   __jni_env->ThrowNew(__jni_env->FindClass(\"java/lang/IndexOutOfBoundsException\"),"
        "\n                       QString::fromLatin1(\"Accessing container of size %3 at %4\")"
        "\n                       .arg(__qt_this->size()).arg($1).toLatin1());"
        "\n   return;"
        "\n}";

    QByteArray code_with_return = QByteArray(code).replace("return;", "return 0;");

    QByteArray code_index_length =
        "\nif ($1 < 0 || $2 < 0 || ($1 + $2) >= __qt_this->size()) {"
        "\n   __jni_env->ThrowNew(__jni_env->FindClass(\"java/lang/IndexOutOfBoundsException\"),"
        "\n                       QString::fromLatin1(\"Accessing container of size %3 from %4 to %5\")"
        "\n                       .arg(__qt_this->size()).arg($1).arg($1+$2).toLatin1());"
        "\n   return;"
        "\n}";

    QByteArray code_non_empty =
        "\nif (__qt_this->isEmpty()) {"
        "\n   __jni_env->ThrowNew(__jni_env->FindClass(\"java/lang/IndexOutOfBoundsException\"),"
        "\n                       QString::fromLatin1(\"Accessing empty container...\").toLatin1());"
        "\n   return;"
        "\n}";

    QByteArray code_two_indices =
        "\nif ($1 < 0 || $2 < 0 || $1 >= __qt_this->size() || $2 >= __qt_this->size()) {"
        "\n   __jni_env->ThrowNew(__jni_env->FindClass(\"java/lang/IndexOutOfBoundsException\"),"
        "\n                       QString::fromLatin1(\"Accessing container of size %3 from %4 to %5\")"
        "\n                       .arg(__qt_this->size()).arg($1).arg($1+$2).toLatin1());"
        "\n   return;"
        "\n}";

    { // QVector safty...
        injectCode(qvector, "at(int) const", code_with_return, args1);
        injectCode(qvector, "replace(int,T)", code, args1);
        injectCode(qvector, "remove(int)", code, args1);
        injectCode(qvector, "remove(int, int)", code_index_length, args2);
        injectCode(qvector, "pop_back()", code_non_empty, ArgumentMap());
        injectCode(qvector, "pop_front()", code_non_empty, ArgumentMap());
    }

    { // QList safty...
        injectCode(qlist, "at(int) const", code_with_return, args1);
        injectCode(qlist, "replace(int, T)", code, args1);
        injectCode(qlist, "pop_back()", code_non_empty, ArgumentMap());
        injectCode(qlist, "pop_front()", code_non_empty, ArgumentMap());
        injectCode(qlist, "swap(int, int)", code_two_indices, args2);
        injectCode(qlist, "move(int, int)", code_two_indices, args2);
        injectCode(qlist, "removeAt(int)", code, args1);
        injectCode(qlist, "takeAt(int)", code_with_return, args1);
    }

}
