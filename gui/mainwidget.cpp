#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QDebug>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>

#include "settingkeys.h"
#include "errorhandlingdlg.h"
#include "utils.h"
#include "credentionals.h"
#include "installfilesgenerator.h"

MainWgt::MainWgt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWgt)
{
    ui->setupUi(this);
    setWindowTitle("WINDOWS INSTALLER");
    //init step wgts
    {
        ui->widgetStep1->setDescription("Login/Sign Up");
        ui->widgetStep2->setDescription("Device Information");
        ui->widgetStep3->setDescription("Customer Information");
        ui->widgetStep4->setDescription("Installing...");
        ui->widgetStep5->setDescription("Finished");
    }

    //login/sign up
    {
        connect (ui->loginSignUp, &LoginSignWgt::sigSuccess,
                 this, &MainWgt::onLoginSignUp);
        connect (ui->loginSignUp, &LoginSignWgt::sigError,
                 this, &MainWgt::onErrorHandling);
    }

    //installing...
    {
        connect (ui->installing, &InstallingWgt::sigSuccess,
                 this, &MainWgt::onSuccessInstall);
        connect (ui->installing, &InstallingWgt::sigFail,
                 this, &MainWgt::onFailInstall);
        connect (ui->installing, &InstallingWgt::sigError,
                 this, &MainWgt::onErrorHandling);
    }

    //deviceInfo
    {
        connect (ui->deviceInfo, &DeviceInfoWgt::sigError,
                 this, &MainWgt::onErrorHandling);
        connect (ui->deviceInfo, &DeviceInfoWgt::sigDevInfo,
                 this, &MainWgt::onDevInfo);      
    }

    //customer info
    {
        connect (ui->customerInfo, &CustomerInfoWgt::sigError,
                 this, &MainWgt::onErrorHandling);
        connect (ui->customerInfo, &CustomerInfoWgt::sigComplete,
                 this, &MainWgt::onCustomerComplete);
    }

    //bottom widget
    {
        connect (ui->pushButtonLogout, &QPushButton::clicked,
                 this, &MainWgt::onLogout);
        connect (ui->pushButtonNext, &QPushButton::clicked,
                 this, &MainWgt::goToNextStep);
        connect (ui->pushButtonStart, &QPushButton::clicked,
                 this, &MainWgt::onStart);
        connect (ui->pushButtonStartNew, &QPushButton::clicked,
                 this, &MainWgt::onStartNew);
    }

    m_manager = new QNetworkAccessManager (this);

    changeDeviceDetected (false);
    goToNextStep ();
}

void MainWgt::onCustomerComplete (bool isComplete)
{
    ui->pushButtonStart->setEnabled (isComplete);
}

void MainWgt::changeDeviceDetected (bool isDetected)
{
    changeProperty (ui->labelDeviceStatusValue, "Status", isDetected ? "success" : "fail");
    if (isDetected)
        ui->labelDeviceStatusValue->setText ("CONNECTED");
    else
        ui->labelDeviceStatusValue->setText ("DEVICE NOT DETECTED");
}

void MainWgt::onDevInfo (const DeviceInfoWgt::DeviceInfo & info)
{
    changeDeviceDetected (info.isConnected);
    ui->pushButtonNext->setEnabled (info.isConnected);
}

void MainWgt::onErrorHandling (QString title, QString what, QString where, QString details)
{
    auto dlg = new ErrorHandlingDlg (title, what, where, details, this);
    dlg->exec();
}

void MainWgt::onSuccessInstall ()
{
    ui->finished->setSuccess ();
    goToNextStep ();
}

void MainWgt::onFailInstall ()
{
    ui->finished->setFail ();
    goToNextStep ();
}

void MainWgt::onLogout ()
{
    changeDeviceDetected (false);
    curStep = SignUpSteps::login_signup;
    goToCurStep ();
}

void MainWgt::onStart ()
{
#ifdef WEBAPI

    auto deviceData     = ui->deviceInfo->getData ();
    auto customerData   = ui->customerInfo->getData ();

    QByteArray aesKey;
    QByteArray iv;
    QString passcode;
    QString challenge;
    InstallFilesGenerator::generateAES_en (aesKey, iv, passcode, challenge);

    QJsonObject obj;

    //B0
    QJsonObject objB0;
    {
        objB0 ["IMEI"]          = deviceData.IMEI;
        objB0 ["Manufacturer"]  = deviceData.Manufacturer;
        objB0 ["Model"]         = deviceData.Model;
        objB0 ["Version"]       = deviceData.Version;
        objB0 ["Serial"]        = deviceData.Serial;
        objB0 ["PNumber"]       = deviceData.PNumber;
        objB0 ["FName"]         = customerData.FName;
        objB0 ["LName"]         = customerData.LName;
        objB0 ["HPhone"]        = customerData.HPhone;
        objB0 ["BPhone"]        = customerData.BPhone;
        objB0 ["Sticker"]       = customerData.Sticker;
        objB0 ["SecKey"]        = QString (aesKey);
        objB0 ["SecIV"]         = QString (iv);

        //??????????????
        objB0 ["AppVer"];
        objB0 ["Brand"];
    }
    obj ["B0"] = QString (RsaEncryption::encryptData (defWebAppPublicKey, QJsonDocument (objB0).toJson ()));

    //B1
    QJsonObject objB1;
    {
        objB1 ["OptRestrct"]    = customerData.OptRestrct;
        objB1 ["Cam"]           = customerData.Cam;
        objB1 ["Galry"]         = customerData.Galry;
        objB1 ["Music"]         = customerData.Music;
        objB1 ["SDCard"]        = customerData.SDCard;
        objB1 ["FMngr"]         = customerData.FMngr;
        objB1 ["BTFmngr"]       = customerData.BTFmngr;
        objB1 ["OutCalWL"]      = customerData.OutCalWL;
        objB1 ["CallWL"]        = customerData.CallWL;
        objB1 ["ParntBlock"]    = customerData.ParntBlock;
        objB1 ["ParntCode"]     = customerData.ParntCode;
        objB1 ["ParntNum"]      = customerData.ParntNum;

        //??????????????
        objB1 ["Blutoth"]       = customerData.Blutoth;

        //??????????????
        objB0 ["ParntVerfd"];
    }
    obj ["B1"] = QString (RsaEncryption::encryptData (defWebAppPublicKey, QJsonDocument (objB1).toJson ()));

    obj ["B2"] = passcode;
    obj ["B3"] = challenge;

    QJsonDocument doc (obj);

    const QUrl url (defWebAppEndpoint);
    QNetworkRequest request (url);
    request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader ("Type", "3");
    //authenication
    {
        QString username    = Credentionals::instance ().userName ();
        QString hashPassw   = Credentionals::instance ().password ();

        //before encryption
        QJsonObject objAuth;
        objAuth["username"] = username;
        objAuth["password"] = hashPassw;
        QJsonDocument docAuth (objAuth);

        auto authData = RsaEncryption::encryptData (defWebAppPublicKey, docAuth.toJson ());

        request.setRawHeader ("Auth", authData);
    }
    QNetworkReply* reply = m_manager->post (request, doc.toJson ());

    connect(reply, &QNetworkReply::finished, this, [=](){
        if(reply->error() == QNetworkReply::NoError){
            auto resp = reply->readAll ();
            auto doc = QJsonDocument::fromJson (resp);
            auto obj = doc.object ();
            QString id = obj ["DeviceID"].toString ();
            ui->installing->setIdDevice (id);
            curStep = SignUpSteps::installing;
            goToCurStep ();

        }else{
            auto resp = reply->readAll ();
            onErrorHandling   ( "Error: WebApp",
                                "Webapp error: error occured during checking invitation in WebApp",
                                "An error occured during sending requests to WebApp. See \"Details\"\n"
                                "section to get more detailed information about the error.",
                                reply->errorString() + resp);
        }
        reply->deleteLater();
    });
#else
    curStep = SignUpSteps::installing;
    goToCurStep ();
#endif
}

void MainWgt::onStartNew ()
{
    changeDeviceDetected (false);
    curStep = SignUpSteps::deviceInfo;
    goToCurStep ();
}

void MainWgt::onLoginSignUp ()
{
    {
#if 1
        QSettings settings;
        auto name = settings.value (defAppName).toString ();
#else
        auto name = Credentionals::instance().userName();
#endif
        ui->labelStatus->setText ("Logged in as : " + name);
    }
    goToNextStep ();
}

void MainWgt::goToCurStep ()
{
    int iStep = static_cast<int> (curStep);
    for(int i=1; i<iStep; i++) {
        auto stepWgt = findChild<StepWgt *>("widgetStep" + QString::number(i));
        stepWgt->setMode("Completed");
    }

    {
        auto stepWgt = findChild<StepWgt *>("widgetStep" + QString::number(iStep));
        stepWgt->setMode("Current");
    }

    for(int i=iStep+1; i<=static_cast<int>(SignUpSteps::num_steps); i++) {
        auto stepWgt = findChild<StepWgt *>("widgetStep" + QString::number(i));
        stepWgt->setMode("NotCompleted");
    }

    if (curStep == SignUpSteps::finished) {
        auto stepWgt = findChild<StepWgt *>("widgetStep" + QString::number(static_cast<int>(SignUpSteps::finished)));
        stepWgt->setMode("Completed");
    }


    ui->pushButtonLogout->show ();
    ui->pushButtonStart->show ();
    ui->pushButtonNext->hide ();
    ui->pushButtonStartNew->hide ();
    ui->pushButtonLogout->setEnabled (true);
    ui->pushButtonStart->setEnabled (true);

    if(curStep == SignUpSteps::login_signup) {
        ui->labelStatus->setText ("Not logged in");
        ui->stackedWidgetWorkArea->setCurrentWidget (ui->loginSignUp);
        ui->loginSignUp->clear ();
        ui->pushButtonLogout->hide ();
        ui->pushButtonStart->hide ();
    }
    else if(curStep == SignUpSteps::deviceInfo) {
        ui->stackedWidgetWorkArea->setCurrentWidget (ui->deviceInfo);
        ui->deviceInfo->init ();
        ui->pushButtonStart->hide ();
        ui->pushButtonNext->show ();
    }
    else if(curStep == SignUpSteps::customerInfo) {
        ui->stackedWidgetWorkArea->setCurrentWidget (ui->customerInfo);
        ui->customerInfo->init ();
        ui->pushButtonStart->setEnabled (false);
    }
    else if(curStep == SignUpSteps::installing) {
        ui->stackedWidgetWorkArea->setCurrentWidget (ui->installing);
        ui->installing->startInstalling ();
        ui->pushButtonLogout->setEnabled (false);
        ui->pushButtonStart->setEnabled (false);
    }
    else if(curStep == SignUpSteps::finished) {
        ui->stackedWidgetWorkArea->setCurrentWidget (ui->finished);
        ui->pushButtonStart->hide ();
        ui->pushButtonStartNew->show ();
    }
}

void MainWgt::goToNextStep ()
{
    if (SignUpSteps::num_steps == curStep) {
        qCritical () << "can not go to next step, last step";
        return;
    }
    curStep = static_cast<SignUpSteps> (static_cast<int> (curStep) + 1);
    goToCurStep ();
}

MainWgt::~MainWgt()
{
    delete ui;
}
