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
        timerUpdate = new QTimer (this);
        timerUpdate->setSingleShot (true);
        timerUpdate->setInterval (30*1000);
        connect (timerUpdate, &QTimer::timeout,
                 this, [this](){
            if (!devInfoFutureWatcher.isRunning ()) {
                auto future = QtConcurrent::run (&DeviceInfoWgt::updateDevInfo, this);
                devInfoFutureWatcher.setFuture (future);
            }
            else
                timerUpdate->start ();
        });
        timerUpdate->start ();
    }
}

void DeviceInfoWgt::init ()
{
    ui->labelIMEI_value->clear ();
    ui->labelManufacturer_value->clear ();
    ui->labelModel_value->clear ();
    ui->labelVersion_value->clear ();
    ui->labelSerialNumber_value->clear ();
    ui->labelDevicePhoneNumber_value->clear ();

    ui->labelConnectionStatus_value->clear ();

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
    }
    else {
        ui->labelConnectionStatus_value->setText ("DEVICE NOT DETECTED");
        ui->pushButtonConnectDevice->setText ("Connect to device");
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
    timerUpdate->start ();

    emit sigDevInfo (newDevInfo);
}

DeviceInfoWgt::~DeviceInfoWgt ()
{
    delete ui;
}

void DeviceInfoWgt::postToWebApp ()
{
    if ("success" != ui->labelConnectionStatus_value->property ("Status").toString ())
        return;
    //send to WebApp
    {
        QJsonObject obj;
        obj["UserEmail"]        = Credentionals::instance ().userEmail ();
        obj["Password"]         = Credentionals::instance ().password ();
        obj["Imei"]             = ui->labelIMEI_value->text().toLongLong ();
        obj["Manufacturer"]     = ui->labelManufacturer_value->text ();
        obj["Model"]            = ui->labelModel_value->text ();
        obj["Version"]          = ui->labelVersion_value->text ().toInt ();
        obj["SerialNumber"]     = ui->labelSerialNumber_value->text ();
        obj["PhoneNUmber"]      = ui->labelDevicePhoneNumber_value->text ();
        QJsonDocument doc(obj);

        QString endpoint = defEndpoint;
        const QUrl url(endpoint+"/device");
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply* reply = m_manager->post (request, doc.toJson ());

        connect(reply, &QNetworkReply::finished, this, [=](){
            if(reply->error() != QNetworkReply::NoError){
                emit sigError   ("Error: WebApp",
                                "Webapp error: error occured during sending post to WebApp",
                                "An error occured during sending requests to WebApp. See \"Details\"\n"
                                "section to get more detailed information about the error.",
                                reply->errorString());
            }
            reply->deleteLater();
        });
    }
}

DeviceInfoWgt::DeviceInfo DeviceInfoWgt::updateDevInfo ()
{
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

