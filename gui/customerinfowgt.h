#ifndef CUSTOMERINFOWGT_H
#define CUSTOMERINFOWGT_H

#include <QWidget>
class QNetworkAccessManager;

namespace Ui {
class CustomerInfoWgt;
}

class CustomerInfoWgt : public QWidget
{
    Q_OBJECT

public:
    explicit CustomerInfoWgt(QWidget *parent = nullptr);

    ~CustomerInfoWgt();

    void init ();

    void postToWebApp ();

private:
    Ui::CustomerInfoWgt *ui;

    QNetworkAccessManager* m_manager {nullptr};

private slots:
    void onParentalBlockChanged ();

    void onCommunityChanged ();

    void onOptionalRestrictionsChanged ();

    void onComplete ();

signals:
    void sigError (QString title, QString what, QString where, QString details);

    void sigComplete (bool isComplete);

};

#endif // CUSTOMERINFOWGT_H
