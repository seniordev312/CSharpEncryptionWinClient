#include "passwordwgt.h"
#include "ui_passwordwgt.h"

PasswordWgt::PasswordWgt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PasswordWgt)
{
    ui->setupUi(this);

    connect (ui->labelLogin, &QLabel::linkActivated,
             this, &PasswordWgt::sigLogin);
}

void PasswordWgt::clear ()
{
    ui->lineEditUsername->clear ();
}

PasswordWgt::~PasswordWgt()
{
    delete ui;
}
