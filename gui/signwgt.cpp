#include "signwgt.h"
#include "ui_signwgt.h"

#include <QKeyEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageAuthenticationCode>
#include <QJsonDocument>
#include <QJsonObject>

#include "utils.h"
#include "settingkeys.h"
#include "credentionals.h"

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
    ui->lineEditEmail->installEventFilter (this);
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
    auto email = ui->lineEditEmail->text ();
    auto password = ui->lineEditPassword->text ();
    auto hashPassw = QMessageAuthenticationCode::hash(password.toUtf8(),
                     (QByteArray)defHashKey,QCryptographicHash::Sha256).toBase64();
    auto name = ui->lineEditName->text ();

    //send to WebApp
    {
        QJsonObject obj;
        obj["UserName"]     = name;
        obj["UserEmail"]    = email;
        obj["Password"]     = QString::fromStdString (hashPassw.toStdString ());
        QJsonDocument doc(obj);

        QString endpoint = defEndpoint;
        const QUrl url(endpoint+"/signUp");
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply* reply = m_manager->post (request, doc.toJson ());

        connect(reply, &QNetworkReply::finished, this, [=](){
            if(reply->error() != QNetworkReply::NoError){
                emit sigError   ("Error: WebApp",
                                "Webapp error: error occured during sending post to WebApp",
                                "An error occured during sending requests to WebApp. See \"Details\"\n"
                                "section to get more detailed information about the error.",
                                reply->errorString());
            }
            else {
                QString resp = QString(reply->readAll());
                if ("1" == resp) {
                    //store locally
                    {
                        Credentionals::instance ().setData (email, name, hashPassw);
                    }

                    emit sigSuccess ();
                }
            }
            reply->deleteLater();
        });
    }
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
    auto code = ui->lineEditInvitationCode->text ().toUtf8 ();

    QString endpoint = defEndpoint;
    const QUrl url(endpoint+"/invitation");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = m_manager->post(request, code);

    connect(reply, &QNetworkReply::finished, this, [=](){
        if(reply->error() == QNetworkReply::NoError){
            QString resp = QString(reply->readAll());
            onlyInviationView ("1" == resp);
        }else{
            emit sigError   ("Error: WebApp",
                            "Webapp error: error occured during checking invitation in WebApp",
                            "An error occured during sending requests to WebApp. See \"Details\"\n"
                            "section to get more detailed information about the error.",
                            reply->errorString());
        }
        reply->deleteLater();
    });
}

void SignWgt::checkConditionsSignUp ()
{
    bool cahSignUp = true;

    changeProperty (ui->labelRePasswordStatus, "Status", "");
    ui->labelRePasswordStatus->clear ();

    if (!ui->checkBoxTermsPrivacy->isChecked ())
        cahSignUp = false;
    if (ui->lineEditPassword->text().isEmpty())
        cahSignUp = false;
    if (ui->lineEditPassword->text() != ui->lineEditRetypePassword->text()) {
        cahSignUp = false;
        changeProperty (ui->labelRePasswordStatus, "Status", "fail");
        ui->labelRePasswordStatus->setText ("Password not the same");
    }

    ui->pushButtonSignUp->setEnabled (cahSignUp);
}

void SignWgt::onlyInviationView (bool activated, bool init)
{
    auto wgts = findChildren <QWidget *> ();
    foreach (auto wgt, wgts)
        wgt->setHidden (!activated);

    if (activated) {
        changeProperty (ui->labelStatusInvitationCode, "Status", "success");
        ui->labelStatusInvitationCode->setText ("Invation accepted");

    }
    else {
        if (!init) {
            changeProperty (ui->labelStatusInvitationCode, "Status", "fail");
            ui->labelStatusInvitationCode->setText("Invation declined");
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
