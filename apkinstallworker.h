
#include <QObject>
#include <QRunnable>
#include <QMutex>

class ApkInstallWorker : public QObject, public QRunnable
{
    Q_OBJECT
public:

    ApkInstallWorker(QString apkFilePath
                     , QString packageName
                     , QString deviceFolder
                     , QString publicKeyFileName
                     , QString localFolder
                     , QString id);

    enum InstallStates { ClearDeviceFolderState
         , PushApkState
         , RunApkState
         , WaitPublicKeyState
         , ReceivePublicKeyState
         , LoadPublicKeyState
         , GenerateInstallFilesState
         , PushInstallFilesState
         , CompleteState };

    void run() override;

    void cancel();
    bool isCanceled();

signals:
    void started();
    void finished();
    void message(QString msg);
    void sigError (QString title, QString what, QString where, QString details);
private:
    bool doPushApk();
    bool doRunApk();
    void doWaitPublicKey();
    void doReceivePublicKey();
    void doLoadPublicKey();
    void doGenerateInstallFiles();
    void doPushInstallFiles();
    void clearLocalFolder();

    QMutex m_cancelMutex;
    QString m_deviceFolder;
    QString m_publicKeyFileName;
    QString m_packageName;
    QString m_apkFilePath;
    QString m_localFolder;
    QString m_id;
    InstallStates m_state;
    bool m_canceled;
    QString m_lastError;
    QByteArray m_apkRsaPublicKeyData;
    QStringList m_installFileList;
    QStringList m_filesToClean;
};
