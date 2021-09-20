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

    struct Data {
        QString FName;

        QString LName;

        QString HPhone;

        QString BPhone;

        QString Sticker;

        bool OptRestrct {false};

        bool Cam {false};

        bool Galry {false};

        bool Blutoth {false};

        bool Music {false};

        bool SDCard {false};

        bool FMngr {false};

        bool BTFmngr {false};

        bool OutCalWL {false};

        bool CallWL {false};

        bool ParntBlock {false};

        QString ParntCode;

        QString ParntNum;

    };

    explicit CustomerInfoWgt(QWidget *parent = nullptr);

    ~CustomerInfoWgt();

    void init ();

    Data getData ();

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
