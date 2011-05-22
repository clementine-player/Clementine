import clementine
import logging


class ClementineLogHandler(logging.Handler):
  def emit(self, record):
    clementine.pythonengine.HandleLogRecord(
      record.levelno, record.name, record.lineno, record.getMessage())


def setup_logging():
  root = logging.getLogger()
  root.addHandler(ClementineLogHandler())
  root.setLevel(logging.NOTSET)
