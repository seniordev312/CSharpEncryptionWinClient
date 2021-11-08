#pragma once

#include <QString>
#include <QFile>
#include <QBuffer>

#include <openssl/evp.h>

typedef struct _cipher_params_t{
    unsigned char *key;
    unsigned char *iv;
    unsigned int encrypt;
    const EVP_CIPHER *cipher_type;
}cipher_params_t;

class AesEncryption
{
public:
    AesEncryption();

    int dectyptFile(const QString& encodedFilePath, const QByteArray& key, const QString& decodedFilePath);

    int decryptBuffer (QBuffer &source,  QBuffer &encoded, const QByteArray& key);

    int decrypt(const QString& encodedFilePath, const QString& decodedFilePath, const QByteArray& key, const QByteArray& iv);

    int encrypt(const QString& sourceFilePath,  const QString& encodedFilePath, const QByteArray& key, const QByteArray& iv);

    bool encryptIODevice(QIODevice *source,  QIODevice *encoded, QByteArray key, QByteArray iv);

    void file_encrypt_decrypt(cipher_params_t * params, QIODevice *ifp, QIODevice *ofp);

    void cleanup(cipher_params_t * params, QIODevice *ifp, QIODevice *ofp);

};
