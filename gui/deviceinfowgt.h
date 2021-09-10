#ifndef DEVICEINFOWGT_H
#define DEVICEINFOWGT_H

#include <QWidget>
#include <QFutureWatcher>
#include <QProcess>

class QTimer;
class QNetworkAccessManager;

namespace Ui {
class DeviceInfoWgt;
}

class DeviceInfoWgt : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceInfoWgt(QWidget *parent = nullptr);

    ~DeviceInfoWgt();

    struct DeviceInfo {
        bool isConnected {false};

        QString imei;

        QString manufacturer;

        QString model;

        QString version;

        QString serialNumber;

        QString devicePhoneNumber;

        bool isError {false};

        QProcess::ProcessError error {QProcess::UnknownError};

    };

    void init ();

    void postToWebApp ();

private:
    Ui::DeviceInfoWgt *ui;

    DeviceInfo updateDevInfo ();

    QFutureWatcher <DeviceInfo> devInfoFutureWatcher;

    QTimer * timerUpdate {nullptr};

    QNetworkAccessManager* m_manager {nullptr};

private slots:
    void onDevInfoUpdated ();

    void onConnectDevice ();

signals:
    void sigError (QString title, QString what, QString where, QString details);

    void sigDevInfo (const DeviceInfoWgt::DeviceInfo & info);

};

Q_DECLARE_METATYPE(DeviceInfoWgt::DeviceInfo);

#endif // DEVICEINFOWGT_H
