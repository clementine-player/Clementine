#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <iostream>

class QNetworkRequest;
class QString;
class QUrl;

std::ostream& operator <<(std::ostream& stream, const QString& str);
std::ostream& operator <<(std::ostream& stream, const QUrl& url);
std::ostream& operator <<(std::ostream& stream, const QNetworkRequest& req);

#endif  // TEST_UTILS_H
