#include <QApplication>
#include <QFile>
#include <QWebFrame>
#include <QWebPage>
#include <QXmlStreamWriter>
#include <QtDebug>

int ShowUsage() {
  qWarning() << "Usage:" << qApp->arguments()[0].toUtf8().constData() << "sites.js\n";
  qWarning() << "This tool parses a sites.js file from the Ultimate Lyrics Amarok script";
  qWarning() << "and outputs an XML file that can be used by Clementine.";
  return 2;
}

void WriteRuleItem(QXmlStreamWriter& writer, const QVariant& value) {
  if (value.type() == QVariant::String) {
    writer.writeStartElement("item");
    writer.writeAttribute("tag", value.toString());
    writer.writeEndElement();
  } else if (value.type() == QVariant::List) {
    QVariantList list = value.toList();
    writer.writeStartElement("item");
    writer.writeAttribute("begin", list[0].toString());
    writer.writeAttribute("end", list[1].toString());
    writer.writeEndElement();
  }
}

void WriteRules(QXmlStreamWriter& writer, const QString& name, const QVariantMap& map) {
  for (int i=1 ; i<=4 ; ++i) {
    const QString map_name = name + (i == 1 ? "" : QString::number(i));

    if (!map.contains(map_name))
      continue;

    writer.writeStartElement(name);

    QVariant value = map[map_name];
    if (value.type() == QVariant::String) {
      WriteRuleItem(writer, value);
    } else if (value.type() == QVariant::List) {
      foreach (const QVariant& child, value.toList()) {
        WriteRuleItem(writer, child);
      }
    }

    writer.writeEndElement();
  }
}

void WriteList(QXmlStreamWriter& writer, const QString& name, const QVariantMap& map) {
  if (!map.contains(name))
    return;

  QVariant value = map[name];
  if (value.type() == QVariant::String) {
    writer.writeStartElement(name);
    writer.writeAttribute("value", value.toString());
    writer.writeEndElement();
  } else if (value.type() == QVariant::List) {
    foreach (const QVariant& child, value.toList()) {
      writer.writeStartElement(name);
      writer.writeAttribute("value", child.toString());
      writer.writeEndElement();
    }
  }
}

void WriteUrlFormat(QXmlStreamWriter& writer, const QVariantList& list) {
  foreach (const QVariant& child, list) {
    if (child.type() != QVariant::Map)
      continue;
    QVariantMap map = child.toMap();
    if (!map.contains("rep") || !map.contains("punct"))
      continue;

    writer.writeStartElement("urlFormat");
    writer.writeAttribute("replace", map["punct"].toString());
    writer.writeAttribute("with", map["rep"].toString());
    writer.writeEndElement();
  }
}

void WriteProvider(QXmlStreamWriter& writer, const QString& name, const QVariant& data) {
  QVariantMap map = data.toMap();
  if (!map.contains("url") || map.contains("getReply"))
    return;

  writer.writeStartElement("provider");
  writer.writeAttribute("name", name);
  writer.writeAttribute("title", map["title"].toString());
  writer.writeAttribute("charset", map["charset"].toString());
  writer.writeAttribute("url", map["url"].toString());

  WriteUrlFormat(writer, map["urlFormat"].toList());

  WriteRules(writer, "extract", map);
  WriteRules(writer, "exclude", map);
  WriteList(writer, "invalidIndicator", map);

  writer.writeEndElement();
}

int main(int argc, char** argv) {
  QApplication a(argc, argv);

  // Parse commandline arguments
  if (a.arguments().count() != 2)
    return ShowUsage();

  QString sites_filename = a.arguments()[1];
  if (!QFile::exists(sites_filename)) {
    qWarning() << "Error:" << sites_filename << "does not exist";
    return 1;
  }

  // Load the javascript file
  QFile sites_file(sites_filename);
  if (!sites_file.open(QIODevice::ReadOnly)) {
    qWarning() << "Error: could not open" << sites_filename;
    return 1;
  }
  QString javascript = QString::fromUtf8(sites_file.readAll());
  javascript = javascript.section("\n}", 0, 0, QString::SectionIncludeTrailingSep);
  javascript.append(";\n(siteDescriptors)");

  // Parse the javascript
  QWebPage page;
  QVariant data = page.mainFrame()->evaluateJavaScript(javascript);

  // Open the document
  QFile stdout(NULL);
  stdout.open(1, QIODevice::WriteOnly);
  QXmlStreamWriter writer(&stdout);
  writer.setAutoFormatting(true);
  writer.setAutoFormattingIndent(2);

  // Begin writing
  writer.writeStartDocument();
  writer.writeStartElement("lyricproviders");

  foreach (const QString& provider_name, data.toMap().keys()) {
    WriteProvider(writer, provider_name, data.toMap()[provider_name]);
  }

  writer.writeEndElement();
  writer.writeEndDocument();

  return 0;
}
