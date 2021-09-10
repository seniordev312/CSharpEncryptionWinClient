#ifndef LOGINSIGNWGT_H
#define LOGINSIGNWGT_H

#include <QWidget>

namespace Ui {
class LoginSignWgt;
}

class LoginSignWgt : public QWidget
{
    Q_OBJECT

public:
    explicit LoginSignWgt(QWidget *parent = nullptr);

    ~LoginSignWgt();

    void clear ();

private:
    Ui::LoginSignWgt *ui;

private slots:
    void onForgotPasswordActivated ();

    void onSignUp ();

    void onLogin ();

signals:
    void sigSuccess ();

    void sigError (QString title, QString what, QString where, QString details);
};

#endif // LOGINSIGNWGT_H
