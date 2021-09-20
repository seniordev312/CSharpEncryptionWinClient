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

    static bool generateAES_en (QByteArray & aesKey, QByteArray & iv, QString &passcode, QString &challenge); //to web posts

    bool generate (QByteArray rsaPulicKey, QString id, QStringList& outList);

private:
    void generateFile (const QString &fullPath, int index);

    static QByteArray generatePasscode ();

    static QByteArray generateChallenge ();

    QString m_folder;

    static QByteArray generateAES256Key ();

    static QByteArray generateIV ();

    static QList <QByteArray> fileWebContents; // <fileContent>

    static QList <InstFileStruct> fileAdbContents; // <fileName, fileContent>

};
