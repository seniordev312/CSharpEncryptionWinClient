#pragma once

#include <QString>
#include <QStringList>
#include <QList>

class InstallFilesGenerator
{
    struct InstFileStruct
    {
        QString name;
        QByteArray content;
    };

    enum InstFileWeb {PasscodeFile, ChallengeFile, CountWebFiles};

    enum instFileAdb {ChallengeAdbFile, PasscodeAdbFile, AesKeyFile, IdFile, CountAdbFiles};

public:
    InstallFilesGenerator (const QString& folder);

    static void createFilesContents ();

    static bool generateAESPassChallenge_en (QByteArray & aesKey, QByteArray & iv, QString &passcode, QString &challenge); //to web posts

    bool generate (QByteArray rsaPulicKey, QString id, QStringList& outList);

    static bool generateApk2 (QByteArray rsaPulicKey, QByteArray apkCode, QString & apkFilePath, QString & keyFilePath);

private:
    static bool encodeAES (QByteArray & aesKey, QByteArray & iv, QByteArray &src);

    static void generateFile (const QString &fullPath, int index);

    static QByteArray generatePasscode ();

    static QByteArray generateChallenge ();

    QString m_folder;

    static QByteArray generateAES256Key ();

    static QByteArray generateIV ();

    static bool generateAES256File (QByteArray rsaPulicKey, QString folder, QStringList& outList, QByteArray & aes256Key, QByteArray & iv);

    static QList <QByteArray> fileWebContents; // <fileContent>

    static QList <InstFileStruct> fileAdbContents; // <fileName, fileContent>

};
