#!/usr/bin/python2
import codecs
import glob
import logging
import os
import polib
import re

PO_GLOB = 'src/translations/*.po'
DESKTOP_PATH = 'dist/clementine.desktop'

class ConfigParser(object):
  """
  Better version of the ConfigParser from the stdlib that handles unicode.
  """

  SECTION_RE = re.compile('^\[(.*)\]\s*$')
  VALUE_RE = re.compile('^([^\s=]+)\s*=\s*(.*)')

  def __init__(self, encoding='utf-8'):
    self._sections = []
    self.encoding = encoding

  def read(self, filename):
    with codecs.open(filename, 'r', self.encoding) as handle:
      current_section = None

      for line in handle:
        match = self.SECTION_RE.match(line)
        if match:
          current_section = self._add_section(match.group(1))
          continue

        match = self.VALUE_RE.match(line)
        if match:
          current_section['items'].append(
              (match.group(1), match.group(2).strip()))
          continue

  def _add_section(self, section_name):
    section = {
      'name': section_name,
      'items': [],
    }
    self._sections.append(section)
    return section

  def _find_section(self, section_name):
    return [
      section
      for section in self._sections
      if section['name'] == section_name
    ][0]

  def _find_item_value(self, section, key):
    return [
      item[1]
      for item in section['items']
      if item[0] == key
    ][0]

  def sections(self):
    return [x['name'] for x in self._sections]

  def get(self, section_name, key):
    try:
      return self._find_item_value(self._find_section(section_name), key)
    except IndexError:
      return None

  def set(self, section_name, key, value):
    try:
      section = self._find_section(section_name)
    except IndexError:
      section = self._add_section(section_name)

    for index, (existing_key, _existing_value) in enumerate(section['items']):
      if existing_key == key:
        section['items'][index] = (key, value)
        return

    section['items'].append((key, value))

  def write(self, filename):
    with codecs.open(filename, 'w', self.encoding) as handle:
      for section in self._sections:
        handle.write('[%s]\n' % section['name'])
        for key, value in section['items']:
          handle.write('%s=%s\n' % (key, value))

        handle.write('\n')


def main():
  logging.basicConfig(level=logging.DEBUG,
                      format="%(asctime)s %(levelname)-7s %(message)s")

  config_parser = ConfigParser()
  config_parser.read(DESKTOP_PATH)

  shortcuts = [
    {
      'section_name': section_name,
      'original': config_parser.get(section_name, 'Name'),
    }
    for section_name in config_parser.sections()
    if section_name.endswith('Shortcut Group')
  ]

  for po_filename in glob.glob(PO_GLOB):
    language = os.path.splitext(os.path.basename(po_filename))[0]
    config_key = 'Name[%s]' % language
    po_file = polib.pofile(po_filename)

    logging.info('Reading language %s', po_filename)

    for shortcut in shortcuts:
      original = shortcut['original']
      entry = po_file.find(original)
      if entry is not None and entry.msgstr and entry.msgstr != original:
        logging.info('Setting %s for %s to %s',
            config_key, shortcut['section_name'], entry.msgstr)
        config_parser.set(shortcut['section_name'], config_key, entry.msgstr)

  config_parser.write(DESKTOP_PATH)


if __name__ == '__main__':
  main()
