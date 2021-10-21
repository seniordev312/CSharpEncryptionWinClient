#include "signwgt.h"
#include "ui_signwgt.h"

#include <QKeyEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageAuthenticationCode>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QRegularExpression>
#include <QMessageBox>

#include "utils.h"
#include "settingkeys.h"
#include "credentionals.h"
#include "rsaencryption.h"

#define defInvitationCode "1234"

SignWgt::SignWgt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SignWgt)
{
    ui->setupUi(this);

    connect (ui->labelLogin, &QLabel::linkActivated,
             this, &SignWgt::sigLogin);
    connect (ui->pushButtonQuestion, &QPushButton::clicked,
             this, &SignWgt::onQuestion);
    connect (ui->pushButtonInvitationCode, &QPushButton::clicked,
             this, &SignWgt::onSend);
    connect (ui->pushButtonSignUp, &QPushButton::clicked,
             this, &SignWgt::onSignUp);
    connect (ui->lineEditInvitationCode, &QLineEdit::textChanged,
             this, &SignWgt::onActivationCodeChanged);
    connect (ui->checkBoxTermsPrivacy, &QCheckBox::stateChanged,
             this, &SignWgt::checkConditionsSignUp);
    connect (ui->lineEditPassword, &QLineEdit::textChanged,
             this, &SignWgt::checkConditionsSignUp);
    connect (ui->lineEditRetypePassword, &QLineEdit::textChanged,
             this, &SignWgt::checkConditionsSignUp);

    onlyInviationView (false, true);
    checkConditionsSignUp ();
    ui->pushButtonInvitationCode->setEnabled (false);
    ui->lineEditPassword->setEchoMode (QLineEdit::Password);
    ui->lineEditRetypePassword->setEchoMode (QLineEdit::Password);

    //event filters
    ui->lineEditInvitationCode->installEventFilter (this);
    ui->lineEditPassword->installEventFilter (this);
    ui->lineEditName->installEventFilter (this);
    ui->lineEditUsername->installEventFilter (this);
    ui->lineEditRetypePassword->installEventFilter (this);

    m_manager = new QNetworkAccessManager(this);
}

SignWgt::~SignWgt ()
{
    delete ui;
}

bool SignWgt::eventFilter (QObject *watched, QEvent *event)
{
    if (QEvent::KeyRelease == event->type () ) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (ui->lineEditInvitationCode == watched) {
            if (Qt::Key_Return == keyEvent->key ()
                && ui->pushButtonInvitationCode->isEnabled ()) {
                onSend ();
                return true;
            }
        }
        else {
            if (Qt::Key_Return == keyEvent->key ()
                && ui->pushButtonSignUp->isEnabled ()) {
                onSignUp ();
                return true;
            }
        }
    }
    return QWidget::eventFilter (watched, event);
}

void SignWgt::clear ()
{
    auto lineEdits = findChildren<QLineEdit *>();
    foreach (auto edit, lineEdits)
        edit->clear ();
    ui->labelStatusInvitationCode->clear ();
    ui->checkBoxTermsPrivacy->setChecked (false);
    onlyInviationView (false, true);
}

void SignWgt::onSignUp ()
{
#ifndef WEBAPI
    QSettings settings;
    auto email = ui->lineEditUsername->text ();
    settings.setValue (defAppUserName, email);
    auto password = ui->lineEditPassword->text ();
    settings.setValue (defAppPassword, password);
    auto name = ui->lineEditName->text ();
    settings.setValue (defAppName, name);
    emit sigSuccess ();
#else
    auto email = ui->lineEditUsername->text ();
    auto password = ui->lineEditPassword->text ();
    auto hashPassw = QCryptographicHash::hash(password.toUtf8(),
                     QCryptographicHash::Sha256).toHex();
    auto name = ui->lineEditName->text ();

    //send to WebApp
    {
        QJsonObject obj;
        obj["username"]     = email;
        obj["password"]     = QString::fromStdString (hashPassw.toStdString ());
        obj["code"]         = code_;
        QJsonDocument doc(obj);

        QJsonObject objEncrypted;
        objEncrypted["Load"] = QString (RsaEncryption::encryptData (defWebAppPublicKey, doc.toJson ()));
        QJsonDocument docEnctypted (objEncrypted);

        const QUrl url(defWebAppEndpoint);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader ("Type", "2");
        QNetworkReply* reply = m_manager->post (request, docEnctypted.toJson ());

        connect(reply, &QNetworkReply::finished, this, [=](){
            if(reply->error() != QNetworkReply::NoError){
                emit sigError   ("Error setting password for user " + ui->lineEditUsername->text (),
                                "Webapp error: error occured during sending post to WebApp",
                                "An error occured during sending requests to WebApp. See \"Details\"\n"
                                "section to get more detailed information about the error.",
                                reply->errorString());
            }
            else {
                //store locally
                {
                    Credentionals::instance ().setData (email, name, hashPassw);
                }

                emit sigSuccess (ui->lineEditName->text ());
            }
            reply->deleteLater();
        });
    }
#endif
}

void SignWgt::onQuestion ()
{
    auto mode = ui->lineEditPassword->echoMode ();
    ui->lineEditPassword->setEchoMode (mode == QLineEdit::Normal ? QLineEdit::Password : QLineEdit::Normal);
    ui->lineEditRetypePassword->setEchoMode (mode == QLineEdit::Normal ? QLineEdit::Password : QLineEdit::Normal);
}

void SignWgt::onActivationCodeChanged ()
{
    ui->pushButtonInvitationCode->setEnabled (!ui->lineEditInvitationCode->text().isEmpty ());
}

void SignWgt::onSend ()
{
#ifndef WEBAPI
    onlyInviationView (defInvitationCode == ui->lineEditInvitationCode->text ());
    ui->lineEditUsername->setText ("Username");
    ui->lineEditName->setText ("Name");
#else

    //before encryption
    QJsonObject obj;
    obj["code"] = ui->lineEditInvitationCode->text ();
    QJsonDocument doc (obj);
    qDebug () << doc.toJson ();
    QJsonObject objEncrypted;
    auto encryptData = QString (RsaEncryption::encryptData (defWebAppPublicKey, doc.toJson ()).toBase64 ());
    qDebug () << encryptData;
    objEncrypted["Load"] = encryptData;
    QJsonDocument docEnctypted (objEncrypted);
    qDebug () << docEnctypted.toJson ();
    const QUrl url (defWebAppEndpoint);
    QNetworkRequest request (url);
    request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader ("Type", "1");
    QNetworkReply* reply = m_manager->post (request, docEnctypted.toJson ());

    auto code = ui->lineEditInvitationCode->text ();
    code_.clear ();

    connect(reply, &QNetworkReply::finished, this, [=](){
        if(reply->error() == QNetworkReply::NoError){
            auto resp = reply->readAll ();
            auto doc = QJsonDocument::fromJson (resp);
            auto obj = doc.object ();
            if (true ==  obj ["Valid"].toBool ()) {
                ui->lineEditUsername->setText (obj ["Username"].toString ());
                ui->lineEditName->setText (obj ["Name"].toString ());
                code_ = code;
            }

            onlyInviationView (obj["Valid"].toBool());
            QMessageBox::information (this, "Success", "Password Successfully Set");
        }else{
            auto resp = reply->readAll ();
            emit sigError   ("Error: WebApp",
                            "Webapp error: error occured during checking invitation in WebApp",
                            "An error occured during sending requests to WebApp. See \"Details\"\n"
                            "section to get more detailed information about the error.",
                            reply->errorString() + resp);
            onlyInviationView (false);
        }
        reply->deleteLater();
    });
#endif
}

void SignWgt::checkConditionsSignUp ()
{
    bool canSignUp = true;

    changeProperty (ui->labelRePasswordStatus, "Status", "");
    ui->labelRePasswordStatus->clear ();
    changeProperty (ui->labelPasswordStatus, "Status", "");
    ui->labelPasswordStatus->clear ();

    auto password = ui->lineEditPassword->text();

    bool containsChar = false;
    for (int i=0; i<password.size (); i++) {
        if (password.at (i).isLetter ())
            containsChar = true;
    }

    if (!ui->checkBoxTermsPrivacy->isChecked ())
        canSignUp = false;
    else if (password.length() < 7) {
        canSignUp = false;
        changeProperty (ui->labelPasswordStatus, "Status", "fail");
        ui->labelPasswordStatus->setText ("Less than 7 symbols");
    }
    else if (!password.contains (QRegularExpression ("[0-9]"))
             || !password.contains (QRegularExpression ("!@#$%^&()*"))
             || !containsChar) {
        canSignUp = false;
        changeProperty (ui->labelPasswordStatus, "Status", "fail");
        ui->labelPasswordStatus->setText ("Must contain alphanumeric, upper and lower casing, special characters");
    }
    else if (ui->lineEditPassword->text() != ui->lineEditRetypePassword->text()) {
        canSignUp = false;
        changeProperty (ui->labelRePasswordStatus, "Status", "fail");
        ui->labelRePasswordStatus->setText ("Password not the same");
    }

    ui->pushButtonSignUp->setEnabled (canSignUp);
}

void SignWgt::onlyInviationView (bool activated, bool init)
{
    auto wgts = findChildren <QWidget *> ();
    foreach (auto wgt, wgts)
        wgt->setHidden (!activated);

    if (activated) {
        changeProperty (ui->labelStatusInvitationCode, "Status", "success");
        ui->labelStatusInvitationCode->setText ("");

    }
    else {
        if (!init) {
            changeProperty (ui->labelStatusInvitationCode, "Status", "fail");
            ui->labelStatusInvitationCode->setText("Invalid or Expired Code");
        }
        else
            ui->labelStatusInvitationCode->clear ();

        ui->labelStatusInvitationCode->show ();
        ui->labelInvitationCode->show ();
        ui->invitationWgt->show ();
        ui->lineEditInvitationCode->show ();
        ui->pushButtonInvitationCode->show ();
        ui->labelLogin->show ();
    }

}
