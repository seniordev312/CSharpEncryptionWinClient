#ifndef CREDENTIONALS_H
#define CREDENTIONALS_H

#include "rsaencryption.h"

class Credentionals
{
public:
    static Credentionals& instance ();

    void setData (QString userName, QString hashPassword, QString name = "");

    QByteArray authHeader ();

private:
    Credentionals () = default;

    ~Credentionals () = default;

    RsaEncryption m_rsaEncryption;

};

#endif // CREDENTIONALS_H
