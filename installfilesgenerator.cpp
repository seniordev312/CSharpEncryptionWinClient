#include "installfilesgenerator.h"

#include <QFile>
#include <QBuffer>
#include <QRandomGenerator>
#include <QDebug>
#include <QMessageAuthenticationCode>

#include "math.h"

#include "aesencryption.h"
#include "rsaencryption.h"
#include "gui/utils.h"

QList <QByteArray> InstallFilesGenerator::fileWebContents (CountWebFiles);

QList <InstallFilesGenerator::InstFileStruct> InstallFilesGenerator::fileAdbContents (CountAdbFiles);

InstallFilesGenerator::InstallFilesGenerator(const QString &folder):m_folder(folder)
{

}

void InstallFilesGenerator::createFilesContents ()
{
    auto passcodeWeb = generatePasscode ();
    auto passcodeAdb = QCryptographicHash::hash (passcodeWeb,
                                                 QCryptographicHash::Sha512).toHex ();

    fileWebContents[PasscodeFile]  = {passcodeWeb};
    fileWebContents[ChallengeFile] = {generateChallenge ()};

    fileAdbContents[AesKeyFile]    = {"aes.key", ""};
    fileAdbContents[AesKeyFile]    = {"aes.key", ""};
    fileAdbContents[IdFile]        = {"id.txt", ""};
}

//to web posts
bool InstallFilesGenerator::generateAES_en (QByteArray & aesKey, QByteArray & iv, QString &passcode, QString &challenge)
{
    createFilesContents ();
    AesEncryption encryption;
    bool ok = true;
    aesKey = generateAES256Key();
    iv = generateIV();
    for (int i = 0; i < CountWebFiles; i++) {
        QBuffer buffSource;
        buffSource.setBuffer (&fileWebContents[i]);
        QBuffer buffEncrypted;
        ok = encryption.encryptIODevice (&buffSource, &buffEncrypted, aesKey, iv);
        if (!ok)
            break;
        if (1==i)
            passcode = buffEncrypted.buffer();
        else if (2==i)
            challenge = buffEncrypted.buffer();
    }



    return ok;
}

QByteArray InstallFilesGenerator::generatePasscode ()
{
    QByteArray res;
    for (int i=0; i<200; i++) {
        quint64 pass =  QRandomGenerator::global ()->bounded (static_cast<quint64> (pow (10, 15)),
                                                              static_cast<quint64> (pow (10, 16) - 1));

        res.append (pass).append("\r\n");
    }
    return res;
}

QByteArray InstallFilesGenerator::generateChallenge ()
{
    QByteArray res;

    QByteArray source;
    QByteArray allSymbols = "0123456789abcdefghjklmnpqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@#$%*&{}[]()/'\"\\`~,;:.!?<>^+-=_";
    std::string strSeed = allSymbols.toStdString();
    std::seed_seq seed(strSeed.begin(), strSeed.end());
    QRandomGenerator randomGenerator(seed);
    for (int iChar=0; iChar<640; iChar++) {
        int index = randomGenerator.bounded(allSymbols.size());
        source.append (allSymbols[index]);
    }

    QByteArray lastHash = source;
    for (int i=0; i<20; i++) {
        lastHash = QCryptographicHash::hash (lastHash,
                                             QCryptographicHash::Sha512).toHex ();
        res += lastHash;
    }

    return res;
}

bool InstallFilesGenerator::generate(QByteArray rsaPulicKey, QString id,  QStringList &outList)
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
        fileAdbContents[IdFile].content = id.toUtf8();
        for (int index = 0; index < CountAdbFiles;index++){
            if (IdFile ==index)
                continue;
            QString baseFileName =QString("file_%1").arg(fileAdbContents[index].name);
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
        QString data = fileAdbContents[index].content;
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
