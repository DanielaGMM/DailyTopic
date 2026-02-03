#include "credentials.h"
#include "ui_credentials.h"

#include "QMessageBox"

Credentials::Credentials(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Credentials)
{
    ui->setupUi(this);
}

Credentials::~Credentials()
{
    delete ui;
}

void Credentials::on_pbnVerifyPassword_clicked()
{
    const QString stPassword = ui->lnePassword->text();
    if (stPassword == "Romix87")
    {
        emit enableEditor();
        this->close();
    }
    else if (stPassword == "")
    {
        QMessageBox::warning(this, tr("Warning"), tr("Insert a password"));
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("What do you think you are doing?"));
    }
}

