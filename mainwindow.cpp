#include "ledwriter.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <systeminformation.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

// TODO: Change this when we connect to a tape, etc?
#define BLINKYTAPE_STRIP_HEIGHT 60
#define DEFAULT_ANIMATION_LENGTH 60

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // TODO: should this go somewhere else?
    qSetMessagePattern("%{type} %{function}: %{message}");

    // TODO: Run on windows to see if this works
    setWindowIcon(QIcon(":/resources/images/blinkytape.jpg"));

    // TODO: Standard init in QWidget we can override instead?
    ui->patternEditor->init(DEFAULT_ANIMATION_LENGTH,BLINKYTAPE_STRIP_HEIGHT);
    ui->colorPicker->init();

    // Our pattern editor wants to get some notifications
    connect(ui->colorPicker, SIGNAL(colorChanged(QColor)),
            ui->patternEditor, SLOT(setToolColor(QColor)));
    connect(ui->penSize, SIGNAL(valueChanged(int)),
            ui->patternEditor, SLOT(setToolSize(int)));

    // The draw timer tells the animation to advance
    drawTimer = new QTimer(this);
    connect(drawTimer, SIGNAL(timeout()), this, SLOT(drawTimer_timeout()));
    drawTimer->start(33);

    // Modify our UI when the tape connection status changes
    connect(&tape, SIGNAL(connectionStatusChanged(bool)),
            this,SLOT(on_tapeConnectionStatusChanged(bool)));

    // Respond to the uploader
    // TODO: Should this be a separate view? it seems weird to have it chillin
    // all static like.
    connect(&uploader, SIGNAL(progressChanged(float)),
            this, SLOT(on_uploaderProgressChanged(float)));
    connect(&uploader, SIGNAL(finished(bool)),
            this, SLOT(on_uploaderFinished(bool)));

    // Set some default values for the painting interface
    ui->penSize->setSliderPosition(2);
    ui->animationSpeed->setSliderPosition(30);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::drawTimer_timeout() {
    // TODO: move this state to somewhere; the patterneditor class maybe?
    static int n = 0;

    if(tape.isConnected()) {
        QByteArray ledData;

        QImage img = ui->patternEditor->getPattern();
        for(int i = 0; i < BLINKYTAPE_STRIP_HEIGHT; i++) {
            QRgb color = BlinkyTape::correctBrightness(img.pixel(n, i));
            ledData.append(qRed(color));
            ledData.append(qGreen(color));
            ledData.append(qBlue(color));
        }
        tape.sendUpdate(ledData);

        n = (n+1)%img.width();
        ui->patternEditor->setPlaybackRow(n);
    }
}


void MainWindow::on_tapeConnectDisconnect_clicked()
{
    if(tape.isConnected()) {
        qDebug() << "Disconnecting from tape";
        tape.close();
    }
    else {
        QList<QSerialPortInfo> tapes = BlinkyTape::findBlinkyTapes();
        qDebug() << "Tapes found:" << tapes.length();

        if(tapes.length() > 0) {
            // TODO: Try another one if this one fails?
            qDebug() << "Attempting to connect to tape on:" << tapes[0].portName();
            tape.open(tapes[0]);
        }
    }
}

void MainWindow::on_animationSpeed_valueChanged(int value)
{
    drawTimer->setInterval(1000/value);
}

void MainWindow::on_animationPlayPause_clicked()
{
    if(drawTimer->isActive()) {
        drawTimer->stop();
        ui->animationPlayPause->setText("Play");
    }
    else {
        drawTimer->start();
        ui->animationPlayPause->setText("Pause");
    }
}

void MainWindow::on_actionOpen_Animation_triggered()
{
    // TODO: Add a simple image gallery thing instead of this, and push
    // this to 'import' and 'export'
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Animation"), "", tr("Animation Files (*.png *.jpg *.bmp)"));

    if(fileName.length() == 0) {
        return;
    }

    // TODO: Test if the file exists?
    ui->patternEditor->init(fileName);
}

void MainWindow::on_actionSave_Animation_triggered()
{
    //TODO: Track if we already had an open file to enable this, add save as?

    // TODO: Add a simple image gallery thing instead of this, and push
    // this to 'import' and 'export'
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Animation"), "", tr("Animation Files (*.png *.jpg *.bmp)"));

    if(fileName.length() == 0) {
        return;
    }

    // TODO: Alert the user if this failed.
    if(!ui->patternEditor->getPattern().save(fileName)) {
        QMessageBox::warning(this, tr("Error"), tr("Error, cannot write file %1.")
                       .arg(fileName));
    }
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_uploadButton_clicked()
{
    if(!tape.isConnected()) {
        return;
    }

    // Convert the animation into a QByteArray
    // The RGB encoder just stores the data as R,G,B over and over again.
    QImage animation =  ui->patternEditor->getPattern();

    QByteArray ledData;
    QImage img = ui->patternEditor->getPattern();

    for(int frame = 0; frame < animation.width(); frame++) {
        for(int pixel = 0; pixel < animation.height(); pixel++) {
            QRgb color = BlinkyTape::correctBrightness(img.pixel(frame, pixel));
            ledData.append(qRed(color));
            ledData.append(qGreen(color));
            ledData.append(qBlue(color));
        }
    }

//    tape.uploadAnimation(ledData,ui->animationSpeed->value());
    uploader.startUpload(tape, ledData,ui->animationSpeed->value());
}

void MainWindow::on_tapeConnectionStatusChanged(bool connected)
{
    qDebug() << "status changed, connected=" << connected;
    if(connected) {
        ui->tapeConnectDisconnect->setText("Disconnect");
        ui->uploadButton->setEnabled(true);
    }
    else {
        ui->tapeConnectDisconnect->setText("Connect");
        ui->uploadButton->setEnabled(false);
    }
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "Pattern Paint",
                       "<b>PatternPaint</b> is companion software for the BlinkyTape. "
                       "It allows you to create animations for your BlinkyTape in "
                       "real-time, and save your designs to the BlinkyTape for "
                       "playback on-the-go.");
}

void MainWindow::on_actionSystem_Information_triggered()
{
    // TODO: store this somewhere, for later disposal.
    SystemInformation* info = new SystemInformation(this);
    info->show();
}

void MainWindow::on_uploaderProgressChanged(float progress)
{
    qDebug() << "Uploader progess:" << progress;
}

void MainWindow::on_uploaderFinished(bool result)
{
    qDebug() << "Uploader finished! Result:" << result;

    // Reconnect to the BlinkyTape
    if(!tape.isConnected()) {
        // TODO: Make connect() function that does this automagically?
        QList<QSerialPortInfo> tapes = BlinkyTape::findBlinkyTapes();
        qDebug() << "Tapes found:" << tapes.length();

        if(tapes.length() > 0) {
            // TODO: Try another one if this one fails?
            qDebug() << "Attempting to connect to tape on:" << tapes[0].portName();
            tape.open(tapes[0]);
        }
    }
}
