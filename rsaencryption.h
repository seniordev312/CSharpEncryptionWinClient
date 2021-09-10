#pragma once
#include <QString>
#include <memory>

class RsaEncryption
{
public:
    RsaEncryption();

    ~RsaEncryption();

    void generate();

    QByteArray decryptPriBase64(QByteArray &data);

    QByteArray decryptPri(QByteArray &data);

    QString publicKey();

    static QByteArray encryptData(const QByteArray& key, const QByteArray& data);

    QByteArray encryptPub(const QByteArray& data);

private:
    struct Private;
    std::unique_ptr<Private> m_impl;
};
