#pragma once

#include <QString>
#include <QProcess>


class AdbWrapper
{
public:
    typedef struct ps {
        ps(int pid, QString name) : pid(pid), name(name){};
        int pid;
        QString name;
    } Process;

    static AdbWrapper &Inst() {
        static AdbWrapper instance;
        return instance;
    }

    bool checkDevices (bool & isError, QProcess::ProcessError & error);

    bool ping (bool & isError, QProcess::ProcessError & error);

    bool waitDevice ();

    QString getIMEI (bool & isError, QProcess::ProcessError & error);

    QString getManufacturer (bool & isError, QProcess::ProcessError & error);

    QString getModel (bool & isError, QProcess::ProcessError & error);

    QString getVersion (bool & isError, QProcess::ProcessError & error);

    QString getSerialNumber (bool & isError, QProcess::ProcessError & error);

    QString getDevicePhoneNumber (bool & isError, QProcess::ProcessError & error);

    bool copyFileToDevice(const QString& localFile, const QString deviceFolder, bool & isError, QProcess::ProcessError & error);

    bool copyFileFromDevice(const QString& deviceFile, const QString& localFolder, bool & isError, QProcess::ProcessError & error);

    QString adbPath();

    bool installApk(const QString& apkFilePath, QString& resp, bool & isError, QProcess::ProcessError & error);

    bool runApk(const QString& apkName, QString& outResp, bool & isError, QProcess::ProcessError & error);

    bool checkFileOnDevice(const QString& deviceFolder, const QString& fileName, QString &outResp, bool & isError, QProcess::ProcessError & error);

    void clearFolderOnDevice(const QString& deviceFolder, QString &outResp, bool & isError, QProcess::ProcessError & error);

    QList<Process> getRunningProcesses(bool &isError, QProcess::ProcessError & error);

    Process getProcessByName(QString name, bool &isError, QProcess::ProcessError & error);

    bool killProcessByName(QString name, bool &isError, QProcess::ProcessError & error);

    //error
    static QString errorWhat (const QProcess::ProcessError & error);

    static QString errorWhere ();

    static QString errorDetails (const QProcess::ProcessError & error);


private:
    AdbWrapper();

    QByteArray runAdb (QStringList args, bool &isError, QProcess::ProcessError & error);

    QString getProp (QString prop, bool &isError, QProcess::ProcessError & error);

    QString getGlobalSetting (QString setting, bool &isError, QProcess::ProcessError & error);

    bool checkIsCDMA (bool &isError, QProcess::ProcessError & error);

    QString callIphonesubinfo (QString number, bool &isError, QProcess::ProcessError & error);

};
