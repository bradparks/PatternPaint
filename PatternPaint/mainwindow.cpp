#include "pattern.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "colormodel.h"
#include "systeminformation.h"
#include "aboutpatternpaint.h"
#include "resizepattern.h"
#include "undocommand.h"
#include "colorchooser.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QDesktopServices>
#include <QtWidgets>
#include <QUndoGroup>
#include <QToolButton>

// TODO: Move this to pattern uploader or something?
#include "ColorSwirl_Sketch.h"

#define DEFAULT_PATTERN_HEIGHT 60
#define DEFAULT_PATTERN_LENGTH 100

#define MIN_TIMER_INTERVAL 10  // minimum interval to wait before firing a drawtimer update

#define CONNECTION_SCANNER_INTERVAL 100

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUi(this);


    // prepare undo/redo
    menuEdit->addSeparator();
    m_undoStackGroup = new QUndoGroup(this);
    m_undoAction = m_undoStackGroup->createUndoAction(this, tr("&Undo"));
    m_undoAction->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+Z")));
    m_undoAction->setEnabled(false);
    menuEdit->addAction(m_undoAction);

    m_redoAction = m_undoStackGroup->createRedoAction(this, tr("&Redo"));
    m_redoAction->setEnabled(false);
    m_redoAction->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+Y")));
    menuEdit->addAction(m_redoAction);

    m_undoStackGroup->addStack(patternEditor->getUndoStack());
    m_undoStackGroup->setActiveStack(patternEditor->getUndoStack());

    // instruments creation
    mCursorAction = new QAction(tr("Selection"), this);
    mCursorAction->setCheckable(true);
    mCursorAction->setIcon(QIcon(":/instruments/images/instruments-icons/cursor.png"));
    //connect(mCursorAction, SIGNAL(triggered(bool)), this, SLOT(instumentsAct(bool)));
    instruments->addAction(mCursorAction);

    mEraserAction = new QAction(tr("Eraser"), this);
    mEraserAction->setCheckable(true);
    mEraserAction->setIcon(QIcon(":/instruments/images/instruments-icons/lastic.png"));
    //connect(mEraserAction, SIGNAL(triggered(bool)), this, SLOT(instumentsAct(bool)));
    instruments->addAction(mEraserAction);

    mColorPickerAction = new QAction(tr("Color picker"), this);
    mColorPickerAction->setCheckable(true);
    mColorPickerAction->setIcon(QIcon(":/instruments/images/instruments-icons/pipette.png"));
    //connect(mColorPickerAction, SIGNAL(triggered(bool)), this, SLOT(instumentsAct(bool)));
    instruments->addAction(mColorPickerAction);

    mMagnifierAction = new QAction(tr("Magnifier"), this);
    mMagnifierAction->setCheckable(true);
    mMagnifierAction->setIcon(QIcon(":/instruments/images/instruments-icons/loupe.png"));
    //connect(mMagnifierAction, SIGNAL(triggered(bool)), this, SLOT(instumentsAct(bool)));
    instruments->addAction(mMagnifierAction);

    mPenAction = new QAction(tr("Pen"), this);
    mPenAction->setCheckable(true);
    mPenAction->setIcon(QIcon(":/instruments/images/instruments-icons/pencil.png"));
    //connect(mPenAction, SIGNAL(triggered(bool)), this, SLOT(instumentsAct(bool)));
    instruments->addAction(mPenAction);

    mLineAction = new QAction(tr("Line"), this);
    mLineAction->setCheckable(true);
    mLineAction->setIcon(QIcon(":/instruments/images/instruments-icons/line.png"));
    //connect(mLineAction, SIGNAL(triggered(bool)), this, SLOT(instumentsAct(bool)));
    instruments->addAction(mLineAction);

    mSprayAction = new QAction(tr("Spray"), this);
    mSprayAction->setCheckable(true);
    mSprayAction->setIcon(QIcon(":/instruments/images/instruments-icons/spray.png"));
    //connect(mSprayAction, SIGNAL(triggered(bool)), this, SLOT(instumentsAct(bool)));
    instruments->addAction(mSprayAction);

    mFillAction = new QAction(tr("Fill"), this);
    mFillAction->setCheckable(true);
    mFillAction->setIcon(QIcon(":/instruments/images/instruments-icons/fill.png"));
    //connect(mFillAction, SIGNAL(triggered(bool)), this, SLOT(instumentsAct(bool)));
    instruments->addAction(mFillAction);

    mRectangleAction = new QAction(tr("Rectangle"), this);
    mRectangleAction->setCheckable(true);
    mRectangleAction->setIcon(QIcon(":/instruments/images/instruments-icons/rectangle.png"));
    //connect(mRectangleAction, SIGNAL(triggered(bool)), this, SLOT(instumentsAct(bool)));
    instruments->addAction(mRectangleAction);

    mEllipseAction = new QAction(tr("Ellipse"), this);
    mEllipseAction->setCheckable(true);
    mEllipseAction->setIcon(QIcon(":/instruments/images/instruments-icons/ellipse.png"));
    //connect(mEllipseAction, SIGNAL(triggered(bool)), this, SLOT(instumentsAct(bool)));
    instruments->addAction(mEllipseAction);

    mCurveLineAction = new QAction(tr("Curve"), this);
    mCurveLineAction->setCheckable(true);
    mCurveLineAction->setIcon(QIcon(":/instruments/images/instruments-icons/curve.png"));
    //connect(curveLineAction, SIGNAL(triggered(bool)), this, SLOT(instumentsAct(bool)));
    instruments->addAction(mCurveLineAction);

    mTextAction = new QAction(tr("Text"), this);
    mTextAction->setCheckable(true);
    mTextAction->setIcon(QIcon(":/instruments/images/instruments-icons/text.png"));
    //connect(mTextAction, SIGNAL(triggered(bool)), this, SLOT(instumentsAct(bool)));
    instruments->addAction(mTextAction);
    menuInstruments->addAction(mCursorAction);
    menuInstruments->addAction(mEraserAction);
    menuInstruments->addAction(mColorPickerAction);
    menuInstruments->addAction(mMagnifierAction);
    menuInstruments->addAction(mPenAction);
    menuInstruments->addAction(mLineAction);
    menuInstruments->addAction(mSprayAction);
    menuInstruments->addAction(mFillAction);
    menuInstruments->addAction(mRectangleAction);
    menuInstruments->addAction(mEllipseAction);
    menuInstruments->addAction(mCurveLineAction);
    menuInstruments->addAction(mTextAction);

    mCursorButton = createToolButton(mCursorAction);
    mEraserButton = createToolButton(mEraserAction);
    mColorPickerButton = createToolButton(mColorPickerAction);
    mMagnifierButton = createToolButton(mMagnifierAction);
    mPenButton = createToolButton(mPenAction);
    mLineButton = createToolButton(mLineAction);
    mSprayButton = createToolButton(mSprayAction);
    mFillButton = createToolButton(mFillAction);
    mRectangleButton = createToolButton(mRectangleAction);
    mEllipseButton = createToolButton(mEllipseAction);
    mCurveButton = createToolButton(mCurveLineAction);
    mTextButton = createToolButton(mTextAction);

    QGridLayout *bLayout = new QGridLayout();
    bLayout->setMargin(3);
    bLayout->addWidget(mCursorButton, 0, 0);
    bLayout->addWidget(mEraserButton, 0, 1);
    bLayout->addWidget(mColorPickerButton, 1, 0);
    bLayout->addWidget(mMagnifierButton, 1, 1);
    bLayout->addWidget(mPenButton, 2, 0);
    bLayout->addWidget(mLineButton, 2, 1);
    bLayout->addWidget(mSprayButton, 3, 0);
    bLayout->addWidget(mFillButton, 3, 1);
    bLayout->addWidget(mRectangleButton, 4, 0);
    bLayout->addWidget(mEllipseButton, 4, 1);
    bLayout->addWidget(mCurveButton, 5, 0);
    bLayout->addWidget(mTextButton, 5, 1);

    QWidget *bWidget = new QWidget();
    bWidget->setLayout(bLayout);

    mPColorChooser = new ColorChooser(0, 0, 0, this);
    mPColorChooser->setStatusTip(tr("Primary color"));
    mPColorChooser->setToolTip(tr("Primary color"));
    //connect(mPColorChooser, SIGNAL(sendColor(QColor)), this, SLOT(primaryColorChanged(QColor)));

    mSColorChooser = new ColorChooser(255, 255, 255, this);
    mSColorChooser->setStatusTip(tr("Secondary color"));
    mSColorChooser->setToolTip(tr("Secondary color"));
    //connect(mSColorChooser, SIGNAL(sendColor(QColor)), this, SLOT(secondaryColorChanged(QColor)));

    QSpinBox *penSizeSpin = new QSpinBox();
    penSizeSpin->setRange(1, 20);
    penSizeSpin->setValue(1);
    penSizeSpin->setStatusTip(tr("Pen size"));
    penSizeSpin->setToolTip(tr("Pen size"));
    connect(penSizeSpin, SIGNAL(valueChanged(int)), this, SLOT(penValueChanged(int)));

    QGridLayout *tLayout = new QGridLayout();
    tLayout->setMargin(3);
    tLayout->addWidget(mPColorChooser, 0, 0);
    tLayout->addWidget(mSColorChooser, 0, 1);
    tLayout->addWidget(penSizeSpin, 1, 0, 1, 2);

    QWidget *tWidget = new QWidget();
    tWidget->setLayout(tLayout);

    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->setMargin(3);

    //mAnimate = createToolButton()
    mSaveFile = createToolButton(actionSave_File);
    mLoadFile = createToolButton(actionLoad_File);
    mSave = createToolButton(actionSave_to_Tape);
    mConnect = createToolButton(actionAutomatically_connect);
    //vLayout->addWidget(
    vLayout->addWidget(mSaveFile);
    vLayout->addWidget(mLoadFile);
    vLayout->addWidget(mConnect);
    QWidget *vWidget = new QWidget();
    vWidget->setLayout(vLayout);
/*
    toolBar->addWidget(bWidget);
    toolBar->addSeparator();
    toolBar->addWidget(tWidget);
    toolBar->addSeparator();
    //toolBar_2->addWidget(vWidget);
    */

    tools->addWidget(new QSpinBox(this));

    drawTimer = new QTimer(this);
    connectionScannerTimer = new QTimer(this);

    mode = Disconnected;

    patternEditor->init(DEFAULT_PATTERN_LENGTH, DEFAULT_PATTERN_HEIGHT);
    colorPicker->init();

    // Our pattern editor wants to get some notifications
    connect(colorPicker, SIGNAL(colorChanged(QColor)),
            patternEditor, SLOT(setToolColor(QColor)));
    connect(penSize, SIGNAL(valueChanged(int)),
            patternEditor, SLOT(setToolSize(int)));

    tape = new BlinkyTape(this);
    // Modify our UI when the tape connection status changes
    connect(tape, SIGNAL(connectionStatusChanged(bool)),
            this,SLOT(on_tapeConnectionStatusChanged(bool)));

    // TODO: Make this on demand by calling the blinkytape object?
    uploader = new AvrPatternUploader(this);

    // TODO: Should this be a separate view? it seems weird to have it chillin
    // all static like.
    connect(uploader, SIGNAL(maxProgressChanged(int)),
            this, SLOT(on_uploaderMaxProgressChanged(int)));
    connect(uploader, SIGNAL(progressChanged(int)),
            this, SLOT(on_uploaderProgressChanged(int)));
    connect(uploader, SIGNAL(finished(bool)),
            this, SLOT(on_uploaderFinished(bool)));

    // Set some default values for the painting interface
    penSize->setSliderPosition(2);
    patternSpeed->setSliderPosition(30);

    // Pre-set the upload progress dialog
    progressDialog = new QProgressDialog(this);
    progressDialog->setWindowTitle("BlinkyTape exporter");
    progressDialog->setLabelText("Saving pattern to BlinkyTape...");
    progressDialog->setMinimum(0);
    progressDialog->setMaximum(150);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setAutoClose(false);

    errorMessageDialog = new QMessageBox(this);
    errorMessageDialog->setWindowModality(Qt::WindowModal);


    // The draw timer tells the pattern to advance
    connect(drawTimer, SIGNAL(timeout()), this, SLOT(drawTimer_timeout()));
    drawTimer->setInterval(33);
    drawTimer->start();


    // Start a scanner to connect to a BlinkyTape automatically
    connect(connectionScannerTimer, SIGNAL(timeout()), this, SLOT(connectionScannerTimer_timeout()));
    connectionScannerTimer->setInterval(CONNECTION_SCANNER_INTERVAL);
    connectionScannerTimer->start();

    readSettings();
}

MainWindow::~MainWindow(){}

QToolButton* MainWindow::createToolButton(QAction *act) {
    QToolButton *toolButton = new QToolButton();
    toolButton->setMinimumSize(QSize(30, 30));
    toolButton->setMaximumSize(QSize(30, 30));
    toolButton->setDefaultAction(act);
    toolButton->setStatusTip(act->text());
    return toolButton;
}

void MainWindow::drawTimer_timeout() {

    // TODO: move this state to somewhere; the patternEditor class maybe?
    static int n = 0;

    // Ignore the timeout if it came to quickly, so that we don't overload the tape
    static qint64 lastTime = 0;
    qint64 newTime = QDateTime::currentMSecsSinceEpoch();
    if (newTime - lastTime < MIN_TIMER_INTERVAL) {
        qDebug() << "Dropping timer update due to rate limiting. Last update " << newTime - lastTime << "ms ago";
        return;
    }

    lastTime = newTime;


    // TODO: Get the width from elsewhere, so we don't need to load the image every frame
    QImage image = patternEditor->getPatternAsImage();

    if(tape->isConnected()) {
        QByteArray ledData;

        for(int i = 0; i < image.height(); i++) {
            QRgb color = ColorModel::correctBrightness(image.pixel(n, i));
            ledData.append(qRed(color));
            ledData.append(qGreen(color));
            ledData.append(qBlue(color));
        }
        tape->sendUpdate(ledData);

        n = (n+1)%image.width();
        patternEditor->setPlaybackRow(n);
    }
}


void MainWindow::connectionScannerTimer_timeout() {
    // If we are already connected, disregard.
    if(tape->isConnected() || mode==Uploading) {
        return;
    }

    // Check if our serial port is on the list
    QList<QSerialPortInfo> tapes = BlinkyTape::findBlinkyTapes();

    if(tapes.length() > 0) {
        on_tapeConnectDisconnect_clicked();
        return;
    }
}


void MainWindow::on_tapeConnectDisconnect_clicked()
{
    if(tape->isConnected()) {
        qDebug() << "Disconnecting from tape";
        tape->close();
    }
    else {
        QList<QSerialPortInfo> tapes = BlinkyTape::findBlinkyTapes();
        qDebug() << "Tapes found:" << tapes.length();

        if(tapes.length() > 0) {
            // TODO: Try another one if this one fails?
            qDebug() << "Attempting to connect to tape on:" << tapes[0].portName();
            tape->open(tapes[0]);
        }
    }
}

void MainWindow::on_patternSpeed_valueChanged(int value)
{
    drawTimer->setInterval(1000/value);
}

void MainWindow::on_patternPlayPause_clicked()
{
    if(drawTimer->isActive()) {
        drawTimer->stop();
        patternPlayPause->setText("Play");
    }
    else {
        drawTimer->start();
        patternPlayPause->setText("Pause");
    }
}

void MainWindow::on_actionLoad_File_triggered()
{
    QSettings settings;
    QString lastDirectory = settings.value("File/LoadDirectory").toString();

    QDir dir(lastDirectory);
    if(!dir.isReadable()) {
        lastDirectory = QDir::homePath();
    }

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Pattern"), lastDirectory, tr("Pattern Files (*.png *.jpg *.bmp)"));

    if(fileName.length() == 0) {
        return;
    }

    QFileInfo lastFile(fileName);
    settings.setValue("File/LoadDirectory", lastFile.absoluteFilePath());

    QImage pattern;

    // TODO: How to handle stuff that's not the right size?
    // Right now we always resize, could offer to crop, center, etc instead.
    if(!pattern.load(fileName)) {
        qDebug() << "Error loading pattern file " << fileName;
        return;
    }

    patternEditor->init(pattern);
}

void MainWindow::on_actionSave_File_triggered()
{
    //TODO: Track if we already had an open file to enable this, add save as?

    QSettings settings;
    QString lastDirectory = settings.value("File/SaveDirectory").toString();

    QDir dir(lastDirectory);
    if(!dir.isReadable()) {
        lastDirectory = QDir::homePath();
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Pattern"), "", tr("Pattern Files (*.png *.jpg *.bmp)"));

    if(fileName.length() == 0) {
        return;
    }

    QFileInfo lastFile(fileName);
    settings.setValue("File/SaveDirectory", lastFile.absoluteFilePath());

    // TODO: Alert the user if this failed.
    if(!patternEditor->getPatternAsImage().save(fileName)) {
        QMessageBox::warning(this, tr("Error"), tr("Error, cannot write file %1.")
                       .arg(fileName));
    }
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_saveToTape_clicked()
{
    on_actionSave_to_Tape_triggered();
}

void MainWindow::on_actionExport_pattern_for_Arduino_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Pattern for Arduino"), "pattern.h", tr("Header File (*.h)"));

    if(fileName.length() == 0) {
        return;
    }

    // Convert the current pattern into a Pattern
    QImage image =  patternEditor->getPatternAsImage();

    // Note: Converting frameRate to frame delay here.
    Pattern pattern(image,
                        1000/patternSpeed->value(),
                        Pattern::INDEXED_RLE);


    // Attempt to open the specified file
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("Error"), tr("Error, cannot write file %1.")
                       .arg(fileName));
        return;
    }

    QTextStream ts(&file);
    ts << pattern.header;
    file.close();
}


void MainWindow::on_tapeConnectionStatusChanged(bool connected)
{
    qDebug() << "status changed, connected=" << connected;
    actionSave_to_Tape->setEnabled(connected);
    if(connected) {
        mode = Connected;

        tapeConnectDisconnect->setText("Disconnect");
        saveToTape->setEnabled(true);
        actionConnect->setText(tr("Disconnect"));
        actionConnect->setIcon(QIcon(":/images/resources/disconnect.png"));
    }
    else {
        mode = Disconnected;

        tapeConnectDisconnect->setText("Connect");
        saveToTape->setEnabled(false);
        actionConnect->setText(tr("Connect"));
        actionConnect->setIcon(QIcon(":/images/resources/connect.png"));

        // TODO: Don't do this if we disconnected intentionally.
        connectionScannerTimer->start();
    }
}

void MainWindow::on_actionAbout_triggered()
{
    // TODO: store this somewhere, for later disposal.
    AboutPatternPaint* info = new AboutPatternPaint(this);
    info->show();
}

void MainWindow::on_actionSystem_Information_triggered()
{
    // TODO: store this somewhere, for later disposal.
    SystemInformation* info = new SystemInformation(this);
    info->show();
}

void MainWindow::on_uploaderMaxProgressChanged(int progressValue)
{
    if(progressDialog->isHidden()) {
        qDebug() << "Got a progress event while the progress dialog is hidden, event order problem?";
        return;
    }

    progressDialog->setMaximum(progressValue);
}

void MainWindow::on_uploaderProgressChanged(int progressValue)
{
    if(progressDialog->isHidden()) {
        qDebug() << "Got a progress event while the progress dialog is hidden, event order problem?";
        return;
    }

    // Clip the progress to maximum, until we work out a better way to estimate it.
    if(progressValue >= progressDialog->maximum()) {
        progressValue = progressDialog->maximum() - 1;
    }

    progressDialog->setValue(progressValue);
}

void MainWindow::on_uploaderFinished(bool result)
{
    mode = Disconnected;

    qDebug() << "Uploader finished! Result:" << result;
    progressDialog->hide();
}


void MainWindow::on_saveFile_clicked()
{
    on_actionSave_File_triggered();
}

void MainWindow::on_loadFile_clicked()
{
    on_actionLoad_File_triggered();
}

void MainWindow::on_actionVisit_the_BlinkyTape_forum_triggered()
{
    QDesktopServices::openUrl(QUrl("http://forums.blinkinlabs.com/", QUrl::TolerantMode));
}

void MainWindow::on_actionTroubleshooting_tips_triggered()
{
    QDesktopServices::openUrl(QUrl("http://blinkinlabs.com/blinkytape/docs/troubleshooting/", QUrl::TolerantMode));
}

void MainWindow::on_actionFlip_Horizontal_triggered()
{
    // TODO: This in a less hacky way?
    // TODO: Undo/redo
    QImage image =  patternEditor->getPatternAsImage();
    patternEditor->init(image.mirrored(true, false));
}

void MainWindow::on_actionFlip_Vertical_triggered()
{
    // TODO: This in a less hacky way?
    // TODO: Undo/redo
    QImage image =  patternEditor->getPatternAsImage();
    patternEditor->init(image.mirrored(false, true));
}

void MainWindow::on_actionClear_Pattern_triggered()
{
    // TODO: This in a less hacky way?
    // TODO: Undo/redo
    QImage image =  patternEditor->getPatternAsImage();
    image.fill(0);
    patternEditor->init(image);
}

void MainWindow::on_actionLoad_rainbow_sketch_triggered()
{
    if(!(tape->isConnected())) {
        return;
    }

    QByteArray sketch = QByteArray(reinterpret_cast<const char*>(ColorSwirlSketch),COLORSWIRL_LENGTH);

    if(!uploader->startUpload(*tape, sketch)) {
        errorMessageDialog->setText(uploader->getErrorString());
        errorMessageDialog->show();
        return;
    }
    mode = Uploading;

    progressDialog->setValue(progressDialog->minimum());
    progressDialog->show();
}

void MainWindow::on_actionSave_to_Tape_triggered()
{
    if(!(tape->isConnected())) {
        return;
    }

    // Convert the current pattern into a Pattern
    QImage image =  patternEditor->getPatternAsImage();

    // Note: Converting frameRate to frame delay here.
    Pattern pattern(image,
                        1000/patternSpeed->value(),
                        Pattern::RGB24);

    // TODO: Attempt different compressions till one works.

    qDebug() << "Color count: " << pattern.colorCount();

    std::vector<Pattern> patterns;
    patterns.push_back(pattern);

    if(!uploader->startUpload(*tape, patterns)) {
        errorMessageDialog->setText(uploader->getErrorString());
        errorMessageDialog->show();
        return;
    }
    mode = Uploading;

    progressDialog->setValue(progressDialog->minimum());
    progressDialog->show();
}


void MainWindow::on_actionResize_Pattern_triggered()
{
    int patternLength = patternEditor->getPatternAsImage().width();
    int ledCount = patternEditor->getPatternAsImage().height();

    // TODO: Dispose of this?
    ResizePattern* resizer = new ResizePattern(this);
    resizer->setWindowModality(Qt::WindowModal);
    resizer->setLength(patternLength);
    resizer->setLedCount(ledCount);
    resizer->exec();

    if(resizer->result() != QDialog::Accepted) {
        return;
    }

    // TODO: Data validation
    if(resizer->length() > 0) {

        qDebug() << "Resizing pattern, length:"
                 << resizer->length()
                 << "height:"
                 << resizer->ledCount();

        // Create a new pattern, filled with a black color
        QImage newImage(resizer->length(),
                            resizer->ledCount(),
                            QImage::Format_RGB32);
        newImage.fill(QColor(0,0,0,0));

        // Copy over whatever portion of the original pattern will fit
        QPainter painter(&newImage);
        QImage originalImage = patternEditor->getPatternAsImage();
        patternEditor->pushUndoCommand(new UndoCommand(originalImage, *(patternEditor)));
        painter.drawImage(0,0,originalImage);

        patternEditor->init(newImage, false);
    }
}

void MainWindow::on_actionAddress_programmer_triggered()
{
//    int patternLength = patternEditor->getPatternAsImage().width();
//    int ledCount = patternEditor->getPatternAsImage().height();

    // TODO: Dispose of this?
    AddressProgrammer* programmer = new AddressProgrammer(this);
    programmer->setWindowModality(Qt::WindowModal);
    programmer->exec();
}

void MainWindow::writeSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(880, 450)).toSize());
    move(settings.value("pos", QPoint(100, 100)).toPoint());
    settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
//    if (userReallyWantsToQuit()) {
    writeSettings();
    event->accept();
//    } else {
//        event->ignore();
//    }
}
