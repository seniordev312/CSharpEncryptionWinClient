#ifndef DEVICEINFOWGT_H
#define DEVICEINFOWGT_H

#include <QWidget>
#include <QFutureWatcher>
#include <QProcess>

#include <atomic>

class QTimer;
class QNetworkAccessManager;

namespace Ui {
class DeviceInfoWgt;
}

class DeviceInfoWgt : public QWidget
{
    Q_OBJECT

public:

    struct Data {
        QString IMEI;

        QString Manufacturer;

        QString Model;

        QString Version;

        QString Serial;

        QString PNumber;
    };

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

    Data getData ();

private:
    Ui::DeviceInfoWgt *ui;

    DeviceInfo updateDevInfo ();

    QFutureWatcher <DeviceInfo> devInfoFutureWatcher;

    //QTimer * timerUpdate {nullptr};

    QTimer * timerPing {nullptr};

    QNetworkAccessManager* m_manager {nullptr};

    void waitDevice ();

    QFutureWatcher <void> devReadyFutureWatcher;

    std::atomic_bool isFinishThreads {false};

private slots:
    void onDevInfoUpdated ();

    void onConnectDevice ();

signals:
    void sigError (QString title, QString what, QString where, QString details);

    void sigConnected (bool isConnected);

};

Q_DECLARE_METATYPE(DeviceInfoWgt::DeviceInfo);

#endif // DEVICEINFOWGT_H
