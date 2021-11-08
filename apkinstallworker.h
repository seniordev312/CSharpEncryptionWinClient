
#include <QObject>
#include <QRunnable>
#include <QMutex>

class ApkInstallWorker : public QObject, public QRunnable
{
    Q_OBJECT
public:

    ApkInstallWorker(QByteArray apkFileData
                     , QByteArray keyDecrypted
                     , QString packageName
                     , QString deviceFolder
                     , QString publicKeyFileName
                     , QString publicKeyFileNameApk1
                     , QString localFolder
                     , QString id);

    enum InstallStates { ClearDeviceFolderState
                         , InstallApk1
                         , KillApk1
                         , StartApk1
                         , WaitPublicKeyStateApk1
                         , ReceivePublicKeyStateApk1
                         , LoadPublicKeyStateApk1
                         , ReEncryptApk2
                         , PushInstallFiles
                         //, RunApkState
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
    void reEncryptApk ();
    void doWaitPublicKeyApk1();
    bool doPushInstallFiles();
    bool doRunApk (QString packageName);
    void doWaitPublicKey (const QString & pubKey);
    void doReceivePublicKey (const QString & pubKey);
    void doLoadPublicKey (const QString & pubKey);
    void doGenerateInstallFiles();
    void clearLocalFolder();

    QMutex m_cancelMutex;
    QString m_deviceFolder;
    QString m_publicKeyFileName;
    QString m_publicKeyFileNameApk1;
    QString m_packageName;
    QByteArray m_apkFileData;
    QByteArray m_keyDecrypted;
    QString m_localFolder;
    QString m_id;
    InstallStates m_state;
    bool m_canceled;
    QString m_lastError;
    QByteArray m_apkRsaPublicKeyData;
    QStringList m_installFileList;
    QString m_apk2FilePath;
    QString m_keyApk2FilePath;
    QStringList m_filesToClean;
};
