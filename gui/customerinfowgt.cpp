#include "customerinfowgt.h"
#include "ui_customerinfowgt.h"

#include <QStyle>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "credentionals.h"
#include "utils.h"

CustomerInfoWgt::CustomerInfoWgt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomerInfoWgt)
{
    ui->setupUi(this);

    m_manager = new QNetworkAccessManager(this);

    connect (ui->comboBoxParentalBlock, &QComboBox::currentTextChanged,
             this, &CustomerInfoWgt::onParentalBlockChanged);
    connect (ui->comboBoxCommunity, &QComboBox::currentTextChanged,
             this, &CustomerInfoWgt::onCommunityChanged);
    connect (ui->comboBoxOptionalRestrictions, &QComboBox::currentTextChanged,
             this, &CustomerInfoWgt::onOptionalRestrictionsChanged);

    auto lineEdits = findChildren<QLineEdit *> ();
    foreach (auto edit, lineEdits) {
        QPalette pal;
        pal.setColor(QPalette::PlaceholderText, Qt::darkGray);
        edit->setPalette(pal);
    }

    onParentalBlockChanged ();
    onCommunityChanged ();
    onOptionalRestrictionsChanged ();
}

void CustomerInfoWgt::postToWebApp ()
{
    //send to WebApp
    {
        QJsonObject obj;
        obj["UserEmail"]        = Credentionals::instance ().userEmail ();
        obj["Password"]         = Credentionals::instance ().password ();
        obj["FirstName"]        = ui->lineEditFirstName->text ();
        obj["LastName"]         = ui->lineEditLastName->text ();
        obj["HomePhone"]        = ui->lineEditHomePhone->text ();
        obj["BusinessPhone"]    = ui->lineEditBusinessPhone->text ();
        obj["StickerNumber"]    = ui->lineEditStickerNumber->text ();
        if (ui->comboBoxFilterLevel->isHidden())
            obj["FilterLevel"]  = ui->comboBoxCommunity->currentText ();
        else
            obj["FilterLevel"]  = ui->comboBoxFilterLevel->currentText ();
        obj["IsOptionalRestrictions"]    = ( ui->comboBoxOptionalRestrictions->currentText () == "Yes" );

        bool isHidden = ui->listWidgetOptionalRestrictions->isHidden();
        for (int i=0; i<ui->listWidgetOptionalRestrictions->count (); i++) {
            auto item = ui->listWidgetOptionalRestrictions->item (i);
            auto text = item->text ();
            auto isOn = isHidden && (item->checkState() != Qt::Unchecked);
            if ("Camera" == text)
                obj["IsCamera"] = isOn;
            else if ("Gallery" == text)
                obj["IsGallery"] = isOn;
            else if ("Music" == text)
                obj["IsMusic"] = isOn;
            else if ("SD Card" == text)
                obj["IsSdCard"] = isOn;
            else if ("File Manager" == text)
                obj["IsFileManager"] = isOn;
            else if ("BT File Transfer" == text)
                obj["IsBtFileTransfer"] = isOn;
            else if ("Outgoing Calls WL" == text)
                obj["IsOutgoingCallsWL"] = isOn;
            else if ("Incoming Calls WL" == text)
                obj["IsIncomingCallsWL"] = isOn;
        }

        obj["IsParentalBlock"]      = ( ui->comboBoxParentalBlock->currentText () == "Yes" );
        obj["ParentalBlockCode"]    = ui->lineEditParentalCode->text ();
        obj["ParentalPhoneNumber"]  = ui->lineEditParentalPhoneNumber->text ();

        QJsonDocument doc(obj);

        QString endpoint = defEndpoint;
        const QUrl url(endpoint+"/userInfo");
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply* reply = m_manager->post (request, doc.toJson ());

        connect(reply, &QNetworkReply::finished, this, [=](){
            if(reply->error() != QNetworkReply::NoError){
                emit sigError   ("Error: WebApp",
                                "Webapp error: error occured during sending post to WebApp",
                                "An error occured during sending requests to WebApp. See \"Details\"\n"
                                "section to get more detailed information about the error.",
                                reply->errorString());
            }
            reply->deleteLater();
        });
    }

}

void CustomerInfoWgt::init ()
{
    auto lineEdits = findChildren <QLineEdit *> ();
    foreach (auto edit, lineEdits)
        edit->clear ();
    auto comboBoxes = findChildren <QComboBox *> ();
    foreach (auto box, comboBoxes)
        box->setCurrentIndex (0);

    ui->checkBoxBatchInstallMode->setChecked (false);
    for (int i=0; i<ui->listWidgetOptionalRestrictions->count (); i++)
        ui->listWidgetOptionalRestrictions->item(i)->setCheckState (Qt::Unchecked);
}

void CustomerInfoWgt::onOptionalRestrictionsChanged ()
{
    bool isOn = ("Yes" == ui->comboBoxOptionalRestrictions->currentText ());
    ui->listWidgetOptionalRestrictions->setHidden (!isOn);
}

void CustomerInfoWgt::onCommunityChanged ()
{
    bool isOn = ("Meshimer" == ui->comboBoxCommunity->currentText ());
    ui->labelFilterLevel->setHidden (!isOn);
    ui->comboBoxFilterLevel->setHidden (!isOn);

    auto labels = ui->groupBoxCommunityFilterLevel->findChildren <QLabel *> ();
    foreach (auto wgt, labels)
        wgt->setEnabled (isOn);
    auto boxes = ui->groupBoxCommunityFilterLevel->findChildren <QComboBox *> ();
    foreach (auto wgt, boxes)
        wgt->setEnabled (isOn);
    auto edits = ui->groupBoxCommunityFilterLevel->findChildren <QLineEdit *> ();
    foreach (auto wgt, edits)
        wgt->setEnabled (isOn);
    ui->listWidgetOptionalRestrictions->setEnabled (isOn);

    ui->labelCommunity->setEnabled (true);
    ui->comboBoxCommunity->setEnabled (true);
}

void CustomerInfoWgt::onParentalBlockChanged ()
{
    bool isOn = ("Yes" == ui->comboBoxParentalBlock->currentText ());
    ui->lineEditParentalPhoneNumber->setHidden (!isOn);
    ui->lineEditParentalCode->setHidden (!isOn);
}

CustomerInfoWgt::~CustomerInfoWgt()
{
    delete ui;
}
