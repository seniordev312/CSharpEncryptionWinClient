#include "credentionals.h"

#include <QSettings>
#include <QJsonObject>
#include <QJsonDocument>

#include "settingkeys.h"
#include "gui/utils.h"

Credentionals& Credentionals::instance ()
{
    static Credentionals singleton;
    return singleton;
}

void Credentionals::setData (QString userName, QString hashPassword, QString name)
{
    m_rsaEncryption.generate ();

    QSettings settings;

    //before encryption
    QJsonObject objAuth;
    objAuth["username"] = userName;
    objAuth["password"] = hashPassword;
    QJsonDocument docAuth (objAuth);

    auto authData = RsaEncryption::encryptData (defWebAppPublicKey, docAuth.toJson (), RSA_PKCS1_PADDING).toBase64();

    settings.setValue (defAuth, authData);
}

QByteArray Credentionals::authHeader ()
{
    QSettings settings;
    qDebug () << settings.value (defAuth).toByteArray ();
    return settings.value (defAuth).toByteArray ();
}

