#ifndef LOGINWGT_H
#define LOGINWGT_H

#include <QWidget>

class QShowEvent;
class QNetworkAccessManager;

namespace Ui {
class LoginWgt;
}

class LoginWgt : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWgt (QWidget *parent = nullptr);

    ~LoginWgt ();

    void clear ();

protected:
    void showEvent (QShowEvent *event) override;

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::LoginWgt *ui;

    QNetworkAccessManager* m_manager;

private slots:
    void onQuestion ();

    void onLogin ();

    void checkConditionsLogin ();

signals:
    void sigForgotPasswordActivated ();

    void sigSignUp ();

    void sigSuccess ();

    void sigError (QString title, QString what, QString where, QString details);
};

#endif // LOGINWGT_H
