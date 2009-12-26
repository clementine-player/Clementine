#include "radioservice.h"

RadioService::RadioService(const QString& name, QObject *parent)
  : QObject(parent),
    name_(name)
{
}
