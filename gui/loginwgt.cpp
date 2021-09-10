#include "loginwgt.h"
#include "ui_loginwgt.h"

#include <QShowEvent>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageAuthenticationCode>
#include <QJsonDocument>
#include <QJsonObject>

#include "settingkeys.h"
#include "utils.h"
#include "credentionals.h"

LoginWgt::LoginWgt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWgt)
{
    ui->setupUi(this);

    connect (ui->labelForgotPassword, &QLabel::linkActivated,
             this, &LoginWgt::sigForgotPasswordActivated);
    connect (ui->labelSignUp, &QLabel::linkActivated,
             this, &LoginWgt::sigSignUp);
    connect (ui->pushButtonLogin, &QPushButton::clicked,
             this, &LoginWgt::onLogin);
    connect (ui->lineEditEmail, &QLineEdit::textChanged,
             this, &LoginWgt::checkConditionsLogin);
    connect (ui->lineEditPassword, &QLineEdit::textChanged,
             this, &LoginWgt::checkConditionsLogin);
    connect (ui->pushButtonQuestion, &QPushButton::clicked,
             this, &LoginWgt::onQuestion);


    ui->lineEditPassword->setEchoMode (QLineEdit::Password);

    //event filter
    ui->lineEditEmail->installEventFilter (this);
    ui->lineEditPassword->installEventFilter (this);

    m_manager = new QNetworkAccessManager(this);

    //ui to future
    ui->labelForgotPassword->hide ();

}

bool LoginWgt::eventFilter (QObject *watched, QEvent *event)
{
    if (QEvent::KeyRelease == event->type ()
        && (ui->lineEditEmail == watched || ui->lineEditPassword == watched)) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (Qt::Key_Return == keyEvent->key ()
            && ui->pushButtonLogin->isEnabled ()) {
            onLogin ();
            return true;
        }
    }
    return QWidget::eventFilter (watched, event);
}

void LoginWgt::onQuestion ()
{
    auto mode = ui->lineEditPassword->echoMode ();
    bool onView = mode == QLineEdit::Normal;
    ui->lineEditPassword->setEchoMode (onView ? QLineEdit::Password : QLineEdit::Normal);
    changeProperty (ui->pushButtonQuestion, "visiblePassw", !onView);
}

void LoginWgt::checkConditionsLogin ()
{
    ui->pushButtonLogin->setEnabled (!ui->lineEditEmail->text ().isEmpty ()
                                     && !ui->lineEditPassword->text ().isEmpty ());
}

void LoginWgt::onLogin ()
{
    auto email = ui->lineEditEmail->text ();
    auto password = ui->lineEditPassword->text ();
    auto hashPassw = QString::fromStdString (QMessageAuthenticationCode::hash(password.toUtf8(),
                              (QByteArray)defHashKey,QCryptographicHash::Sha256).toBase64().toStdString ());

    //send to WebApp
    {
        QJsonObject obj;
        obj["UserEmail"]    = email;
        obj["Password"]     = hashPassw;
        QJsonDocument doc(obj);

        QString endpoint = defEndpoint;
        const QUrl url(endpoint+"/login");
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
                QJsonParseError error;
                auto resp = QString (reply->readAll ());
                QJsonDocument doc = QJsonDocument::fromJson(resp.toUtf8(), &error);
                if (doc.isNull ()) {
                    emit sigError   ("Error: WebApp",
                                    "Webapp error: error occured during parsing reply from WebApp",
                                    "An error occured during sending requests to WebApp. See \"Details\"\n"
                                    "section to get more detailed information about the error.",
                                    error.errorString());
                    return;
                }
                auto obj = doc.object ();
                if ("0" == obj["Res"].toString ()) {
                    changeProperty (ui->labelLoginStatus, "Status", "fail");
                    ui->labelLoginStatus->setText ("Credentionals are not valid");
                }
                else {
                    auto name = obj["Name"].toString ();
                    Credentionals::instance ().setData (email, name, hashPassw);
                    emit sigSuccess ();
                }
            }
            reply->deleteLater();
        });
    }
}

void LoginWgt::clear ()
{
    auto lineEdits = findChildren<QLineEdit *>();
    foreach (auto edit, lineEdits)
        edit->clear ();
    ui->labelLoginStatus->clear ();
}

void LoginWgt::showEvent(QShowEvent *event)
{
    ui->lineEditEmail->setFocus ();
    QWidget::showEvent(event);
}

LoginWgt::~LoginWgt()
{
    delete ui;
}
