#pragma once
#include <QString>
#include <QStringList>

class InstallFilesGenerator
{
public:
    InstallFilesGenerator(const QString& folder);

    static void createFilesContents ();

    static bool generateAES_en(QByteArray aesKey, QString & file_passcode, QString & file_challenge); //to web posts

    bool generate(const QByteArray& rsaPulicKey, QStringList& outList);

private:
    void generateFile(const QString &fullPath, int index);

    QString m_folder;

    static QByteArray generateAES256Key();

    static QByteArray generateIV();

    static QByteArrayList  fileContents;
};
