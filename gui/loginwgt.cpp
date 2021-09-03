#include "loginwgt.h"
#include "ui_loginwgt.h"

#include <QShowEvent>
#include <QSettings>

#include "settingkeys.h"
#include "utils.h"

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
    //UI/UX test - in API part must be rewrited
    {
        QSettings settings;
        auto storedEmail = settings.value (defAppEmail).toString ();
        auto storedPassword = settings.value (defAppPassword).toString ();
        if (ui->lineEditEmail->text () == storedEmail &&
            ui->lineEditPassword->text () == storedPassword)
            emit sigSuccess ();
        else {
            changeProperty (ui->labelLoginStatus, "Status", "fail");
            ui->labelLoginStatus->setText ("Credentionals are not valid");
        }
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
