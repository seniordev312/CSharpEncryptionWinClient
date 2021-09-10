#ifndef SIGNWGT_H
#define SIGNWGT_H

#include <QWidget>

class QNetworkAccessManager;

namespace Ui {
class SignWgt;
}

class SignWgt : public QWidget
{
    Q_OBJECT

public:
    explicit SignWgt(QWidget *parent = nullptr);

    ~SignWgt();

    void clear ();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::SignWgt *ui;

    QNetworkAccessManager* m_manager {nullptr};

    void onlyInviationView (bool activated, bool init = false);

private slots:
    void onQuestion ();

    void onSend ();

    void onActivationCodeChanged ();

    void checkConditionsSignUp ();

    void onSignUp ();

signals:
    void sigLogin ();

    void sigSuccess ();

    void sigError (QString title, QString what, QString where, QString details);
};

#endif // SIGNWGT_H
