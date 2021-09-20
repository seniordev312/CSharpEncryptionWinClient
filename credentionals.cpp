#include "credentionals.h"

#include <QSettings>

#include "settingkeys.h"

Credentionals& Credentionals::instance ()
{
    static Credentionals singleton;
    return singleton;
}

void Credentionals::setData (QString userName, QString name, QString password)
{
    m_rsaEncryption.generate ();

    auto userName_ = m_rsaEncryption.encryptPub ( userName.toUtf8 ());
    auto name_ = m_rsaEncryption.encryptPub (name.toUtf8 ());
    auto password_ = m_rsaEncryption.encryptPub (password.toUtf8 ());

    QSettings settings;
    settings.setValue (defAppUserName, userName_);
    settings.setValue (defAppPassword, password_);
    settings.setValue (defAppName, name_);
}

QString Credentionals::data (QString settingsKey)
{
    QString res;
    QSettings settings;
    QByteArray ba = settings.value (settingsKey).toByteArray ();
    res = m_rsaEncryption.decryptPri (ba);
    return res;
}

QString Credentionals::userName ()
{
    return data (defAppUserName);
}

QString Credentionals::name ()
{
    return data (defAppName);
}

QString Credentionals::password ()
{
    return data (defAppPassword);
}
