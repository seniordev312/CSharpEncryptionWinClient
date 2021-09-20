#ifndef CREDENTIONALS_H
#define CREDENTIONALS_H

#include "rsaencryption.h"

class Credentionals
{
public:
    static Credentionals& instance();

    void setData (QString userName, QString name, QString password);

    QString userName ();

    QString name ();

    QString password ();

private:
    Credentionals() = default;

    ~Credentionals() = default;

    RsaEncryption m_rsaEncryption;

    QString data (QString settingsKey);

};

#endif // CREDENTIONALS_H
