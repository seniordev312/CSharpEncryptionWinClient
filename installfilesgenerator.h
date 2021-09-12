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

    enum InstFileIndex {PasscodeFile, ChallengeFile, CountWebFiles,
                        AesKeyFile = CountWebFiles, IdFile, CountFiles};

public:
    InstallFilesGenerator(const QString& folder);

    static void createFilesContents ();

    static bool generateAES_en(QByteArray aesKey, QString & passcode, QString & challenge); //to web posts

    bool generate(QByteArray rsaPulicKey, QString id, QStringList& outList);

private:
    void generateFile (const QString &fullPath, int index);

    static QByteArray generatePasscode ();

    static QByteArray generateChallenge ();

    QString m_folder;

    static QByteArray generateAES256Key ();

    static QByteArray generateIV ();

    static QList <InstFileStruct> fileContents; // <fileName, fileContent>

};
