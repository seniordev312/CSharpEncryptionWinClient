#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QVariant>

class QWidget;

#define defEndpoint "http://localhost:8080"

void changeProperty(QWidget * obj, const char *property, QVariant value);

#endif // UTILS_H
