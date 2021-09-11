#include "installfilesgenerator.h"

#include <QFile>
#include <QBuffer>
#include <QRandomGenerator>
#include <QDebug>

#include "aesencryption.h"
#include "rsaencryption.h"

QByteArrayList InstallFilesGenerator::fileContents;

InstallFilesGenerator::InstallFilesGenerator(const QString &folder):m_folder(folder)
{

}

void InstallFilesGenerator::createFilesContents ()
{
    fileContents = {"Mock data content for file 1", "Mock data content for file 2"};
}

//to web posts
bool InstallFilesGenerator::generateAES_en (QByteArray aesKey, QString &file_passcode, QString &file_challenge)
{
    createFilesContents ();
    AesEncryption encryption;
    bool ok = true;
    for (int i = 0; i < fileContents.size(); i++) {
        QBuffer buffSource;
        buffSource.setBuffer (&fileContents[i]);
        QBuffer buffEncrypted;
        QByteArray iv = generateIV();
        ok = encryption.encryptIODevice (&buffSource, &buffEncrypted, aesKey, iv);
        if (!ok)
            break;
        if (1==i)
            file_passcode = buffEncrypted.buffer();
        else if (2==i)
            file_challenge = buffEncrypted.buffer();
    }
    return ok;
}

bool InstallFilesGenerator::generate(const QByteArray &rsaPulicKey, QStringList &outList)
{
    QByteArray aes256Key = generateAES256Key();
    QByteArray iv = generateIV();
    iv.append(aes256Key);

    //Encrypt key
    QByteArray encryptedAes256Key = RsaEncryption::encryptData(rsaPulicKey, iv);

    //Save to file
    QString aesFilePath = QString("%1/aes.key").arg(m_folder);
    QFile aesFile(aesFilePath);
    bool ok = aesFile.open(QIODevice::WriteOnly);
    if(ok){
        aesFile.write(encryptedAes256Key);
        aesFile.close();
        outList.append(aesFilePath);
    }

    if(ok)
    {
        AesEncryption encryption;
        for(int index = 0; index < 2;index++){
            QString baseFileName =QString("file_%1").arg(index);
            QString sourceFile = QString("%1/%2.txt").arg(m_folder, baseFileName);
            generateFile(sourceFile, index);
            //encode file
            QString encodedFile = QString("%1/%2.encoded").arg(m_folder, baseFileName);
            ok = (encryption.encrypt(sourceFile,encodedFile, aes256Key, iv) == 0);
            QFile::remove(sourceFile);
            outList.append(encodedFile);
            if(!ok){break;}
        }
    }

    return ok;
}

void InstallFilesGenerator::generateFile(const QString &fullPath, int index)
{
    QFile file(fullPath);
    if(file.open(QIODevice::ReadWrite)){
        QString data = fileContents[index];
        file.write(data.toLatin1());
        file.close();
    }
}

QByteArray InstallFilesGenerator::generateAES256Key()
{
    const int l = 32;
    QByteArray ba = QByteArray(l, 0);
    for(int i = 0; i < l;i++){
        ba[i] = static_cast<char>(QRandomGenerator::system()->bounded(255));
    }

    return ba;
}

QByteArray InstallFilesGenerator::generateIV()
{
    const int l = 16;
    QByteArray ba = QByteArray(l, 0);
    for(int i = 0; i < l;i++){
        ba[i] = static_cast<char>(QRandomGenerator::system()->bounded(255));
    }

    return ba;
}
