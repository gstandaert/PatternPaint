#ifndef RESIZETAPE_H
#define RESIZETAPE_H

#include <QDialog>

namespace Ui {
class ResizeTape;
}

class ResizeTape : public QDialog
{
    Q_OBJECT

public:
    explicit ResizeTape(QWidget *parent = 0);
    ~ResizeTape();

    void setSize(int size);
    int size();

private:
    Ui::ResizeTape *ui;
};

#endif // RESIZETAPE_H
