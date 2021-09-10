#include "credentionals.h"

#include <QSettings>

#include "settingkeys.h"

Credentionals& Credentionals::instance ()
{
    static Credentionals singleton;
    return singleton;
}

void Credentionals::setData (QString userEmail, QString userName, QString password)
{
    m_rsaEncryption.generate ();

    auto userEmail_ = m_rsaEncryption.encryptPub ( userEmail.toUtf8 ());
    auto userName_ = m_rsaEncryption.encryptPub (userName.toUtf8 ());
    auto password_ = m_rsaEncryption.encryptPub (password.toUtf8 ());

    QSettings settings;
    settings.setValue (defAppEmail, userEmail_);
    settings.setValue (defAppPassword, password_);
    settings.setValue (defAppName, userName_);
}

QString Credentionals::data (QString settingsKey)
{
    QString res;
    QSettings settings;
    QByteArray ba = settings.value (settingsKey).toByteArray ();
    res = m_rsaEncryption.decryptPri (ba);
    return res;
}

QString Credentionals::userEmail ()
{
    return data (defAppEmail);
}

QString Credentionals::userName ()
{
    return data (defAppName);
}

QString Credentionals::password ()
{
    return data (defAppPassword);
}
