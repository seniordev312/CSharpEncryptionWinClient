#include "deviceinfowgt.h"
#include "ui_deviceinfowgt.h"

#include <QtConcurrent>
#include <QProcess>
#include <QMetaEnum>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "adbwrapper.h"
#include "utils.h"
#include "credentionals.h"

DeviceInfoWgt::DeviceInfoWgt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceInfoWgt)
{
    ui->setupUi(this);

    m_manager = new QNetworkAccessManager(this);

    connect (ui->pushButtonConnectDevice, &QPushButton::clicked,
             this, &DeviceInfoWgt::onConnectDevice);
    connect (&devInfoFutureWatcher, &QFutureWatcher<DeviceInfo>::finished,
             this, &DeviceInfoWgt::onDevInfoUpdated);

    //timerUpdate
    {
//        timerUpdate = new QTimer (this);
//        timerUpdate->setSingleShot (true);
//        timerUpdate->setInterval (30*1000);
//        connect (timerUpdate, &QTimer::timeout,
//                 [this](){
//            if (!devInfoFutureWatcher.isRunning ()) {
//                auto future = QtConcurrent::run (&DeviceInfoWgt::updateDevInfo, this);
//                devInfoFutureWatcher.setFuture (future);
//            }
//            else
//                timerUpdate->start ();
//        });
//        timerUpdate->start ();
    }

    //timerPing
    {
        timerPing = new QTimer (this);
        timerPing->setInterval (500);
        connect (timerPing, &QTimer::timeout,
                 [this](){
            bool isError = false;
            QProcess::ProcessError error;
            bool isConnected = AdbWrapper::ping (isError, error);

            changeProperty (ui->labelConnectionStatus_value, "Status", isConnected ? "success" : "fail");
            if (isConnected) {
                ui->labelConnectionStatus_value->setText ("CONNECTED");
                ui->pushButtonConnectDevice->setText ("Reconnect");
            }
            else {
                ui->labelConnectionStatus_value->setText ("DEVICE NOT DETECTED");
                ui->pushButtonConnectDevice->setText ("Connect to device");
                init ();
                timerPing->stop ();
            }

            if (isError) {
                emit sigError ( "Error: Adb Module",
                                AdbWrapper::errorWhat(error),
                                AdbWrapper::errorWhere(),
                                AdbWrapper::errorDetails(error));
                timerPing->stop ();
            }

            emit sigConnected (isConnected);
        });
    }

    connect (&devReadyFutureWatcher, &QFutureWatcher<void>::finished,
             [this](){
                if (!devInfoFutureWatcher.isRunning ()) {
                    auto future = QtConcurrent::run (&DeviceInfoWgt::updateDevInfo, this);
                    devInfoFutureWatcher.setFuture (future);
                }
            });
    auto future = QtConcurrent::run (&DeviceInfoWgt::waitDevice, this);
    devReadyFutureWatcher.setFuture (future);
}

DeviceInfoWgt::Data DeviceInfoWgt::getData ()
{
    DeviceInfoWgt::Data res;

    res.IMEI            = ui->labelIMEI_value->text ();
    res.Manufacturer    = ui->labelManufacturer_value->text ();
    res.Model           = ui->labelModel_value->text ();
    res.Version         = ui->labelVersion_value->text ();
    res.Serial          = ui->labelSerialNumber_value->text ();
    res.PNumber         = ui->labelDevicePhoneNumber_value->text ();

    return res;
}

void DeviceInfoWgt::init ()
{
    ui->labelIMEI_value->clear ();
    ui->labelManufacturer_value->clear ();
    ui->labelModel_value->clear ();
    ui->labelVersion_value->clear ();
    ui->labelSerialNumber_value->clear ();
    ui->labelDevicePhoneNumber_value->clear ();

    changeProperty (ui->labelConnectionStatus_value, "Status", "fail");
    ui->labelConnectionStatus_value->setText ("DEVICE NOT DETECTED");

    ui->pushButtonConnectDevice->setEnabled (true);
    onConnectDevice ();
}

void DeviceInfoWgt::onConnectDevice ()
{   
    if (!devInfoFutureWatcher.isRunning ()) {
        auto future = QtConcurrent::run (&DeviceInfoWgt::updateDevInfo, this);
        devInfoFutureWatcher.setFuture (future);
    }
    ui->pushButtonConnectDevice->setEnabled (false);
}

void DeviceInfoWgt::onDevInfoUpdated ()
{
    auto newDevInfo = devInfoFutureWatcher.result ();

    changeProperty (ui->labelConnectionStatus_value, "Status", newDevInfo.isConnected ? "success" : "fail");
    if (newDevInfo.isConnected) {
        ui->labelConnectionStatus_value->setText ("CONNECTED");
        ui->pushButtonConnectDevice->setText ("Reconnect");
        timerPing->start ();
    }
    else {
        ui->labelConnectionStatus_value->setText ("DEVICE NOT DETECTED");
        ui->pushButtonConnectDevice->setText ("Connect to device");
        auto future = QtConcurrent::run (&DeviceInfoWgt::waitDevice, this);
        devReadyFutureWatcher.setFuture (future);
    }
    ui->labelIMEI_value->setText (newDevInfo.imei);
    ui->labelManufacturer_value->setText (newDevInfo.manufacturer);
    ui->labelModel_value->setText (newDevInfo.model);
    ui->labelVersion_value->setText (newDevInfo.version);
    ui->labelSerialNumber_value->setText (newDevInfo.serialNumber);
    ui->labelDevicePhoneNumber_value->setText (newDevInfo.devicePhoneNumber);

    ui->pushButtonConnectDevice->setEnabled (true);

    if (newDevInfo.isError)
        emit sigError ( "Error: Adb Module",
                        AdbWrapper::errorWhat(newDevInfo.error),
                        AdbWrapper::errorWhere(),
                        AdbWrapper::errorDetails(newDevInfo.error));
    //timerUpdate->start ();

    emit sigConnected (newDevInfo.isConnected);
}

DeviceInfoWgt::~DeviceInfoWgt ()
{
    delete ui;
}

void DeviceInfoWgt::waitDevice ()
{
    AdbWrapper::waitDevice ();
}

DeviceInfoWgt::DeviceInfo DeviceInfoWgt::updateDevInfo ()
{
    timerPing->stop ();

    DeviceInfo res;

    res.isConnected = AdbWrapper::checkDevices (res.isError, res.error);
    if (!res.isError)
        res.imei = AdbWrapper::getIMEI (res.isError, res.error);
    if (!res.isError)
        res.manufacturer = AdbWrapper::getManufacturer (res.isError, res.error);
    if (!res.isError)
        res.model = AdbWrapper::getModel (res.isError, res.error);
    if (!res.isError)
        res.version = AdbWrapper::getVersion (res.isError, res.error);
    if (!res.isError)
        res.serialNumber = AdbWrapper::getSerialNumber (res.isError, res.error);
    if (!res.isError)
        res.devicePhoneNumber = AdbWrapper::getDevicePhoneNumber (res.isError, res.error);

    return res;
}

