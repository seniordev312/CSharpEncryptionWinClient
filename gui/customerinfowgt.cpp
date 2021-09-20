#include "customerinfowgt.h"
#include "ui_customerinfowgt.h"

#include <QStyle>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegularExpressionValidator>

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

    auto reg = QRegularExpression ("[0-9]*");
    ui->lineEditBusinessPhone->setValidator (new QRegularExpressionValidator(reg, ui->lineEditBusinessPhone));
    ui->lineEditHomePhone->setValidator (new QRegularExpressionValidator(reg, ui->lineEditHomePhone));
    ui->lineEditStickerNumber->setValidator (new QRegularExpressionValidator(reg, ui->lineEditStickerNumber));

    connect (ui->lineEditFirstName, &QLineEdit::textChanged,
             this, &CustomerInfoWgt::onComplete);
    connect (ui->lineEditLastName, &QLineEdit::textChanged,
             this, &CustomerInfoWgt::onComplete);
    connect (ui->lineEditHomePhone, &QLineEdit::textChanged,
             this, &CustomerInfoWgt::onComplete);
    connect (ui->lineEditStickerNumber, &QLineEdit::textChanged,
             this, &CustomerInfoWgt::onComplete);
}

CustomerInfoWgt::Data CustomerInfoWgt::getData ()
{
    Data res;

    res.FName       = ui->lineEditFirstName->text ();
    res.LName       = ui->lineEditLastName->text ();
    res.HPhone      = ui->lineEditHomePhone->text ();
    res.BPhone      = ui->lineEditBusinessPhone->text ();
    res.Sticker     = ui->lineEditStickerNumber->text ();

    res.OptRestrct  = ( ui->comboBoxOptionalRestrictions->currentText () == "Yes" );
    if (res.OptRestrct) {
        for (int i=0; i<ui->listWidgetOptionalRestrictions->count (); i++) {
            auto item = ui->listWidgetOptionalRestrictions->item (i);
            auto text = item->text ();
            auto isOn = (item->checkState() != Qt::Unchecked);
            if ("Camera" == text)
                res.Cam = isOn;
            else if ("Gallery" == text)
                res.Galry = isOn;
            else if ("Music" == text)
                res.Music = isOn;
            else if ("SD Card" == text)
                res.SDCard = isOn;
            else if ("File Manager" == text)
                res.FMngr = isOn;
            else if ("BT File Transfer" == text)
                res.BTFmngr = isOn;
            else if ("Outgoing Calls WL" == text)
                res.OutCalWL = isOn;
            else if ("Incoming Calls WL" == text)
                res.CallWL = isOn;;
        }
    }

    //???????????????????
    res.Blutoth;

    res.ParntBlock  = ( ui->comboBoxParentalBlock->currentText () == "Yes" );
    res.ParntCode   = ui->lineEditParentalCode->text ();
    res.ParntNum    = ui->lineEditParentalPhoneNumber->text ();

    return res;
}

void CustomerInfoWgt::onComplete ()
{
    bool isComplete = !(ui->lineEditFirstName->text().isEmpty()
        || ui->lineEditLastName->text().isEmpty()
        || ui->lineEditHomePhone->text().isEmpty()
        || ui->lineEditStickerNumber->text().isEmpty());
    emit sigComplete (isComplete);

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
