#include "customerinfowgt.h"
#include "ui_customerinfowgt.h"

#include <QStyle>

CustomerInfoWgt::CustomerInfoWgt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomerInfoWgt)
{
    ui->setupUi(this);

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

    auto wgts = ui->groupBoxCommunityFilterLevel->findChildren <QWidget *> ();
    foreach (auto wgt, wgts)
        wgt->setEnabled (isOn);
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
