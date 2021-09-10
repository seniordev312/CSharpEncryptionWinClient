#ifndef CREDENTIONALS_H
#define CREDENTIONALS_H

#include "rsaencryption.h"

class Credentionals
{
public:
    static Credentionals& instance();

    void setData (QString userEmail, QString userName, QString password);

    QString userEmail ();

    QString userName ();

    QString password ();

private:
    Credentionals() = default;

    ~Credentionals() = default;

    RsaEncryption m_rsaEncryption;

    QString data (QString settingsKey);

};

#endif // CREDENTIONALS_H
