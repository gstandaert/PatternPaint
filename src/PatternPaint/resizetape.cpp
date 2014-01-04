#include "resizetape.h"
#include "ui_resizetape.h"

ResizeTape::ResizeTape(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResizeTape)
{
    ui->setupUi(this);

    ui->tapeSize->setValidator(new QDoubleValidator(this));
}

ResizeTape::~ResizeTape()
{
    delete ui;
}

void ResizeTape::setSize(int size) {
    ui->tapeSize->setText(QString::number(size));
}

int ResizeTape::size()
{
    return ui->tapeSize->text().toInt();
}
