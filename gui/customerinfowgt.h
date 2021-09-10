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

signals:
    void sigError (QString title, QString what, QString where, QString details);
};

#endif // CUSTOMERINFOWGT_H
