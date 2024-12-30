#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include <QtMultimediaWidgets>
#include <QtSerialPort/QSerialPortInfo>
#include <QSerialPort>
#include <qdebug.h>
#include <QRegExp>
#include <QSharedPointer>
#include <QCameraDevice>
#include <QCamera>
#include <QMediaRecorder>
#include <QtMultimedia/QMediaCaptureSession>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QDateTime>
#include <QFile>
#include <QThread>
#include <QDesktopServices>
#include "modifiedWidgets.h"
#include "videoexporter.h"


// #include <QtWidgets>

// pointers from imus needs to be destroyed at the end
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow),
                                          gyro_pattern("Gyro: (-?\\d+\\.\\d+), (-?\\d+\\.\\d+), (-?\\d+\\.\\d+)"),
                                          quat_pattern("Quat: (-?\\d+\\.\\d+), (-?\\d+\\.\\d+), (-?\\d+\\.\\d+), (-?\\d+\\.\\d+);"),
                                          cal_pattern("Cal: (\\d+)"),
    recordingDir(std::make_unique<QDir>()),
    recordedDir(std::make_unique<QDir>()),
    logDialog(nullptr),
    csvFile(nullptr),
    csvOut(nullptr),
    metadatafile(nullptr),
    metadataOut(nullptr),
    MediaRecorders(std::make_unique<QVector<QMediaRecorder*>>()),
    CameraDevices(std::make_unique<QVector<QCameraDevice>>()),
    Cameras(std::make_unique<QVector<QCamera*>>()),
    CameraBoxes(std::make_unique<QVector<ClickableComboBox*>>()),
    videoWidgets(std::make_unique<QVector<QVideoWidget*>>()),
    MediaCaptureSessions(std::make_unique<QVector<QMediaCaptureSession*>>()),
    Gyro_values(std::make_unique<QVector<QString>>(3)),
    Quat_values(std::make_unique<QVector<QString>>(4)),
    Cal(std::make_unique<int>(-1)),
    recordingStarted(std::make_unique<bool>(false)),
    gettingImuValues(std::make_unique<bool>(false)),
    gotAllImuValues({false,false,false}),
    recordingTimer(std::make_unique<QElapsedTimer>()),
    timer(new QTimer(this))
    

{

    if(!recordedDir->exists(recordedFolder)){
        recordedDir->mkpath(recordedFolder);
    }
    recordedDir->setPath(recordedFolder);
    if(!recordingDir->exists(recordingFolder)){
        recordingDir->mkdir(recordingFolder);
    }
    recordingDir->setPath(recordingFolder);
    csvFile=std::make_unique<QFile>(recordingDir->filePath("imudata.csv"));
     if (csvFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        csvOut = std::make_unique<QTextStream>(csvFile.get());
    } else {
        showError("Failed to open file: " + csvFile->fileName());
    }
    metadatafile=std::make_unique<QFile>(recordingDir->filePath("metadata.txt"));
    if(metadatafile->open(QIODevice::WriteOnly|QIODevice::Text)){
        metadataOut = std::make_unique<QTextStream>(metadatafile.get());
    } else {
        showError("failed to open file"+ metadatafile->fileName());
    }
    folderName = QString::number(QDateTime::currentSecsSinceEpoch()) ;
    
    patterns.append(gyro_pattern);
    patterns.append(quat_pattern);
    patterns.append(cal_pattern);
    const QMutexLocker locker(&calibration_locker);
    
    ui->setupUi(this);
    this->setWindowTitle("StayFit Vertigo");
    this->setWindowIcon(QIcon(":/assets/icon.ico"));
    this->ImuThread = new ReceiverThread(this);
    ui->progressBar->setMaximum(3);
    ui->progressBar->setValue(0);
    setupCamera(new QMediaCaptureSession(this),changeLabelToVideoWidget(ui->camera1), changeToClickableComboBox(ui->listDevices1),new QCamera(this),new QMediaRecorder(this), 0);
    setupCamera(new QMediaCaptureSession(this), changeLabelToVideoWidget(ui->camera2), changeToClickableComboBox(ui->listdevices2),new QCamera(this),new QMediaRecorder(this), 1);
    setupCamera(new QMediaCaptureSession(this), changeLabelToVideoWidget(ui->camera3), changeToClickableComboBox(ui->listdevices3),new QCamera(this),new QMediaRecorder(this) , 2);


    ui->stackedWidget->setCurrentIndex(0);
    ui->pushButton_2->setEnabled(false);
    ui->label_7->setText("No Recordings");
    // ui->pushButton_3->setEnabled(false);
    connect(ui->pushButton_5,&QPushButton::clicked,this,&MainWindow::OpenRecordingsFolder);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateElapsedTime);
    connect(ui->pushButton_4,&QPushButton::clicked,this,&MainWindow::setToStartingPage);
    connect(ui->pushButton_3,&QPushButton::clicked,this,&MainWindow::StartStitcher);
    connect(ui->Start,&QPushButton::clicked,this,&MainWindow::checkImuCalibration);
    connect(ui->actionstart,&QAction::triggered,this,&MainWindow::setToRecordingPage);
    connect(ui->action1st_page,&QAction::triggered,this,&MainWindow::setToStartingPage);
    connect(ui->actionImu_Data,&QAction::triggered,this,&MainWindow::openLogDialog);
    connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::startRecording);
    connect(ui->pushButton_2,&QPushButton::clicked,this,&MainWindow::stopRecording);
    connect(ui->stackedWidget,&QStackedWidget::currentChanged,this,&MainWindow::onPageChanged);
    connect(ImuThread, &ReceiverThread::ImuData, this, &MainWindow::readSerialData);
    ImuThread->startReceiver(921600, 10);//buadrate and timeout
}

MainWindow::~MainWindow()
{
    this->ImuThread->stop();
    delete ImuThread;
    delete ui;
}

void MainWindow::StartStitcher()
{

    // qDebug()<<"started Stitching";
    VideoExporter *videoExporter = new VideoExporter();
    videoExporter->openInNewWindow();
}

void MainWindow::onPageChanged(int index){
    if(index == 1){
    ui->page->hide();
    ui->page_2->setVisible(true);
    
    // qDebug()<<"name: "<<ui->lineEdit_2->text();
    // qDebug()<<"sex: "<<ui->lineEdit->text();
    // qDebug()<<"age: "<<ui->lineEdit_3->text();
    // qDebug()<<"medical history: "<<ui->lineEdit_4->text();
    // qDebug()<<"recent accident: "<<ui->lineEdit_5->text();
    *metadataOut<<"Name: "<<ui->lineEdit_2->text()<<"\n";
    *metadataOut<<"sex: "<<ui->lineEdit->text()<<"\n";
    *metadataOut<<"age: "<<ui->lineEdit_3->text()<<"\n";
    *metadataOut<<"medical history: "<<ui->lineEdit_4->text()<<"\n";
    *metadataOut<<"recent accident: "<<ui->lineEdit_5->text()<<"\n";
    folderName = ui->lineEdit_2->text()+"_"+QString::number(QDateTime::currentSecsSinceEpoch());
    }
    else if(index ==0){
        metadatafile->close();
        metadatafile=std::make_unique<QFile>(recordingDir->filePath("metadata.txt"));
        if(metadatafile->open(QIODevice::WriteOnly|QIODevice::Text)){
            metadataOut = std::make_unique<QTextStream>(metadatafile.get());
        } else {
            showError("failed to open file"+ metadatafile->fileName());
        }
        (*metadataOut)<<""; //clear metadata
        // qDebug()<<"txt file cleared";
        ui->page->setVisible(true);
        ui->page_2->hide();
    }
}

void MainWindow::setupCamera(QMediaCaptureSession* session, QVideoWidget* videoWidget, ClickableComboBox* cameraBox,QCamera* camera,QMediaRecorder* mediaRecorder, int index) {
    MediaCaptureSessions->append(session);
    videoWidgets->append(videoWidget);
    CameraBoxes->append(cameraBox);
    Cameras->append(camera);
    camera->setActive(false);
    session->setVideoOutput(videoWidget);
    session->setCamera(camera);
    session->setRecorder(mediaRecorder);
    MediaRecorders->append(mediaRecorder);
    cameraBox->addItem("Choose a Camera Device");
    cameraBox->setCurrentIndex(0);
    QString filepath = recordingDir->absoluteFilePath("output"+QString::number(index)+".mp4");
    QFile videoPath(filepath) ;
    // qDebug()<<filepath;
    if(videoPath.open(QIODevice::WriteOnly)) videoPath.close();


    mediaRecorder->setOutputLocation(QUrl::fromLocalFile(filepath));
    mediaRecorder->setVideoResolution(640,480);
    mediaRecorder->setQuality(QMediaRecorder::Quality::HighQuality);
    connect(mediaRecorder,&QMediaRecorder::errorOccurred,this,&MainWindow::errors);
    // mediaRecorder->setOutputLocation();

    connect(cameraBox, &ClickableComboBox::clicked, [this, index](const QString& cameraName = "DefaultCamera") {
        getAvailableCameras(index);
    });
    connect(cameraBox, &ClickableComboBox::activated, [this, index](int idx) {
        setCameraToWidget(idx, index);
    });
}

 void MainWindow::errors(QMediaRecorder::Error err,QString error){
    // qDebug()<<error;
}

void MainWindow::getAvailableCameras(int ComboboxNumber)
{
    int count_usb_cameras = 1;
    auto combobox = (*CameraBoxes)[ComboboxNumber];
    
    combobox->clear();
    CameraDevice_locker.lock();
    CameraDevices->clear();
    auto videoInputs = QMediaDevices().videoInputs();
    for(auto &camera : videoInputs){
        CameraDevices->append(camera);
        auto camera_discription = camera.description();
        if(camera_discription == QString("USB Camera")){
            camera_discription += QString::number(count_usb_cameras);
            count_usb_cameras++;
        }
         // Add to ComboBox if it's not already listed
        combobox->addItem(camera_discription);
    }
    CameraDevice_locker.unlock();
}

void MainWindow::setCameraToWidget(int index,int widgetIndex)
{
    CameraDevice_locker.lock();
    auto availableCamera = (*CameraDevices)[index];
    CameraDevice_locker.unlock();
    auto videoInputs = QMediaDevices().videoInputs();
    bool deviceAvailable = false;
    for(size_t i = 0;i<videoInputs.size();i++)
    {
        if(availableCamera.id() == videoInputs[i].id()) {deviceAvailable=true;break;}
    }
    if(deviceAvailable)
    {

        (*Cameras)[widgetIndex]->setActive(false);

        (*Cameras)[widgetIndex]->setCameraDevice(availableCamera);
        
        (*Cameras)[widgetIndex]->setActive(true);

    }
    // qDebug()<<(*CameraBoxes)[widgetIndex]->itemText(index);
}

void MainWindow::updateElapsedTime()
{
    if((*recordingStarted)){
       qint64 elapsed = recordingTimer->elapsed();
       QTime time = QTime(0,0).addMSecs(elapsed);
       QString timeString = time.toString("hh:mm:ss");
       ui->label_7->setText("Recording Started: " + timeString);
    }else{
        qint64 elapsed = recordingTimer->elapsed();
        QTime time = QTime(0,0).addMSecs(elapsed);
        QString timeString =  time.toString("hh:mm:ss");
        timer->stop();
        ui->label_7->setText("Recording Stopped. Last Recording Time: " + timeString);
        // qDebug()<<"timer stopped";
    }
}

void MainWindow::OpenRecordingsFolder(){
    QDesktopServices::openUrl(QUrl::fromLocalFile(recordedDir->absolutePath()));
}

ClickableComboBox *MainWindow::changeToClickableComboBox(QComboBox *box)
{
    QWidget *parentWidget = box->parentWidget();
    QLayout *layout = parentLayout(box);
    ClickableComboBox *clickableBox = new ClickableComboBox(parentWidget);
    layout->replaceWidget(box, clickableBox);
    return clickableBox;
}


QVideoWidget *MainWindow::changeLabelToVideoWidget(QLabel *label)
{
    QWidget *parentWidget = label->parentWidget();
    QLayout *layout = parentLayout(label);
    QVideoWidget *video = new QVideoWidget(parentWidget);
    layout->replaceWidget(label, video);
    delete label;
    return video;
}

void MainWindow::startRecording()
{
    // qDebug()<<"started";
    ui->pushButton_2->setEnabled(true);
    ui->pushButton->setEnabled(false);
    // ui->pushButton_3->setEnabled(false);
    recordedDir->mkdir(folderName);
    
    for(int i = 0;i<3;i++){
        QString fname = folderName+"/output"+QString::number(i)+".mp4";
        QFile videoPath(fname) ;
        if (videoPath.open(QIODevice::WriteOnly)) videoPath.close();
    }

    for(auto &recorder:(*MediaRecorders)){
        
        
        QString fname =folderName+"/"+ recorder->outputLocation().fileName();
        QFile videoPath(fname) ;
        if (videoPath.open(QIODevice::WriteOnly)) videoPath.close();
        QString final_pt = recordedDir->absoluteFilePath(fname);
        // qDebug()<<final_pt;

        recorder->setOutputLocation(QUrl::fromLocalFile(final_pt));
        copyFiles(recordingDir->absoluteFilePath(recorder->outputLocation().fileName()),recordedDir->absoluteFilePath(fname));
    }
    (*csvOut)<<"Quaternion W,Quaternion X, Quaternion Y, Quaternion z,Rotation X, Rotation Y, Rotation Z, calibration, TimeStamp"<<"\n";
    for(auto &videorecorders:(*MediaRecorders)){
        videorecorders->record();
    }
    (*recordingStarted) = true;
    recordingTimer->start();
    timer->start(1000);

}
void MainWindow::stopRecording()
{
    // qDebug()<<"stopped";
    (*recordingStarted) = false;
    
    
    
    QString csvName = folderName+QString("/imudata.csv");
    QString metadataName = folderName+QString("/metadata.txt");
    // qDebug()<<folderName;
    csvFile->copy(recordedDir->absoluteFilePath(csvName));
    csvFile->close();
    metadatafile->copy(recordedDir->absoluteFilePath(metadataName));
    

    for(auto &recorder:(*MediaRecorders)){
        recorder->stop();}
        QThread::msleep(1000);
    
    csvFile=std::make_unique<QFile>(recordingDir->absoluteFilePath("imudata.csv"));
    if (csvFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        csvOut = std::make_unique<QTextStream>(csvFile.get());
    } else {
        showError("Failed to open file: " + csvFile->fileName());
    }
    (*csvOut)<<"";
    
    
    folderName = ui->lineEdit_2->text()+"_"+QString::number(QDateTime::currentSecsSinceEpoch());
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(false);
    // ui->pushButton_3->setEnabled(true);
    
}

void MainWindow::setToRecordingPage()
{
    ui->stackedWidget->setCurrentIndex(1);
}
void MainWindow::setToStartingPage()
{
    ui->stackedWidget->setCurrentIndex(0);
}

QLayout *MainWindow::parentLayout(QWidget *widget)
{
    QLayout *layout = widget->parentWidget() ? widget->parentWidget()->layout() : nullptr;
    return parentLayout(widget, layout);
}

QLayout *MainWindow::parentLayout(QWidget *widget, QLayout *layout)
{
    int itemCount = layout ? layout->count() : 0;
    for (int i = 0; i < itemCount; i++)
    {
        if (layout->itemAt(i)->widget() == widget)
        {
            return layout;
        }
        else if (QLayout *itemLayout = parentLayout(widget, layout->itemAt(i)->layout()))
        {
            return itemLayout;
        }
    }
    return nullptr;
}


void MainWindow::readSerialData(QByteArray data)
{
   QStringList theData = QString(data).split("\r\n");

for (const auto &ImuData : theData)
{   
    QString data = ImuData;
    emit imuDataRecieved(data);
    if (data == "None")
    {
        if (ui->label_6->text() != " Imu Device Not Plugged in") {
            ui->label_6->setText(" Imu Device Not Plugged in");
        }
        calibration_locker.lock();
        *Cal = -1;
        // countToThree = 0;
        calibration_locker.unlock();
        ui->progressBar->setValue(0);
        continue;
    }

    for (const auto &pattern : patterns)
    {
        QRegularExpressionMatch match = pattern.match(data);
        if (match.hasMatch())
        {
            if(!*(gettingImuValues)) (*gettingImuValues) = true; 
            if (ui->label_6->text() != "Imu Connected") {
            ui->label_6->setText("Imu Connected");
        }
            if (match.lastCapturedIndex() == 4) // Quaternion values
            {
                (*Quat_values)[0] = match.captured(1);
                (*Quat_values)[1] = match.captured(2);
                (*Quat_values)[2] = match.captured(3);
                (*Quat_values)[3] = match.captured(4);
                gotAllImuValues[0]=true;
                
            }
            else if (match.lastCapturedIndex() == 3) // Gyroscope values
            {
                (*Gyro_values)[0] = match.captured(1);
                (*Gyro_values)[1] = match.captured(2);
                (*Gyro_values)[2] = match.captured(3);
                gotAllImuValues[1]=true;
                
            }
            else if (match.lastCapturedIndex() == 1) // Calibration value
            {
                gotAllImuValues[2]=true;
                calibration_locker.lock();
                *Cal = match.captured(1).toInt();
                calibration_locker.unlock();
                auto cal = *Cal;
                if(cal==3){
                    if(previousCalibration!=3) countToThree +=1;
                    if(countToThree==3 && !gotImu3threeTimes) gotImu3threeTimes = true;
                }
                if(gotImu3threeTimes || cal ==3) ui->progressBar->setValue(3);
                else ui->progressBar->setValue(cal);
                previousCalibration=cal;
            }
            break; // Exit the pattern loop once a match is found
        }
    }

    // Update UI only when no match is found and IMU values are not being processed
    if (!(*gettingImuValues))
    {
        if (ui->label_6->text() != "got other values") {
            ui->label_6->setText("got other values");
        }
    }
    if(gotAllImuValues[0]==true && gotAllImuValues[1]==true && gotAllImuValues[2]==true and (*recordingStarted)==true){
        // out<<writeData of type QVector

        gotAllImuValues = {false,false,false};
        (*csvOut)<<(*Quat_values)[0]<<","<<(*Quat_values)[1]<<","<<(*Quat_values)[2]<<","<<(*Quat_values)[3]<<","<<(*Gyro_values)[0]<<","<<(*Gyro_values)[1]<<","<<(*Gyro_values)[2]<<","<<QString::number(*Cal)<<QDateTime::currentMSecsSinceEpoch()<<"\n";
        
    }
}

}

void MainWindow::checkImuCalibration(){
    calibration_locker.lock();
    int cal = *(Cal);
    calibration_locker.unlock();
    switch (cal) {
    case -1:
        QMessageBox::warning(this,"Imu not Found","Check wether IMU is connected properly");
        break;
    case 0:
        QMessageBox::information(this,"Imu needs Calibration","Move your device around to calibrate it");
        break;
    case 1:
        QMessageBox::information(this,"Imu needs Calibration","Move your device around to calibrate it");
        break;
    case 2:
        QMessageBox::information(this,"Imu needs Calibration","Move your device around to calibrate it");
        break;
    case 3:
        setToRecordingPage();
        break;
    default:
        break;
    }
}

void MainWindow::openLogDialog() {
    if (!logDialog) {
        logDialog = new LogDialog(this); // Pass parent to manage lifetime
        logDialog->setModal(false);
        connect(this, &MainWindow::imuDataRecieved, logDialog, &LogDialog::appendLogData); // Connect updates
    }

    logDialog->setLogData(""); // Initialize with existing data
    logDialog->show();
}
void MainWindow::copyFiles(QString originalFilePath,QString destinationPath){
     // QThread *copyThread = new QThread(this);
        FileCopyWorker *worker = new FileCopyWorker();
        
        // Move the worker to the new thread
        // worker->moveToThread(copyThread);

        // // Connect signals and slots
        // connect(copyThread, &QThread::started, [worker, originalFilePath, destinationPath]() {
        //     worker->copyFile(originalFilePath, destinationPath);
        // });
        // connect(worker, &FileCopyWorker::copyCompleted, this, &MainWindow::onCopyCompleted);
        // connect(worker, &FileCopyWorker::copyCompleted, copyThread, &QThread::quit);
        // connect(copyThread, &QThread::finished, worker, &QObject::deleteLater);
        // connect(copyThread, &QThread::finished, copyThread, &QObject::deleteLater);

        // Start the file copy thread

        worker->copyFile(originalFilePath, destinationPath);
}

void MainWindow::onCopyCompleted(){
    // qDebug()<<"copied";
}

void MainWindow::showError(const QString& err){
    static QErrorMessage errorMessage; // Static instance of QErrorMessage
    errorMessage.showMessage(err);
    errorMessage.exec(); // Wait for user interaction
}

void MainWindow::getPortDescription()
{
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &portInfo : serialPortInfos)
    {
        qDebug() << "\n"
                 << "Port:" << portInfo.portName() << "\n"
                 << "Location:" << portInfo.systemLocation() << "\n"
                 << "Description:" << portInfo.description() << "\n"
                 << "Manufacturer:" << portInfo.manufacturer() << "\n"
                 << "Serial number:" << portInfo.serialNumber() << "\n"
                 << "Vendor Identifier:"
                 << (portInfo.hasVendorIdentifier()
                         ? QByteArray::number(portInfo.vendorIdentifier(), 16)
                         : QByteArray())
                 << "\n"
                 << "Product Identifier:"
                 << (portInfo.hasProductIdentifier()
                         ? QByteArray::number(portInfo.productIdentifier(), 16)
                         : QByteArray());
    }
}

ReceiverThread::ReceiverThread(QObject *parent)
    : QThread(parent)
{
    // Constructor logic if needed
    // qDebug() << "ReceiverThread created";
}

ReceiverThread::~ReceiverThread()
{
    stop();
    // Destructor logic if needed
    // qDebug() << "ReceiverThread destroyed";
}

void ReceiverThread::startReceiver(int baudrate, int waitTimeout)
{
    const QMutexLocker locker(&m_mutex);

    m_waitTimeout = waitTimeout;
    m_baudrate = baudrate;
    if (!isRunning())
        start();
}


QString ReceiverThread::GetPortName()
{
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    // QString portname = nullptr;
    for (const QSerialPortInfo &portInfo : serialPortInfos)
    {
        if (portInfo.description().indexOf("CP210x") != -1)
            return portInfo.portName();
    }
    return "";
}

QString ReceiverThread::FindAndChangePortName(){
    while(!m_quit){
        QThread::msleep(100);
        if(m_portName==GetPortName() and GetPortName().length()!=0);
        else if(m_portName!=GetPortName() and GetPortName().length()!=0) {

            m_portName =  GetPortName();
            return m_portName;
        }
    }
    return "";
}
void ReceiverThread::run()
{


    m_mutex.lock();
    while (!m_quit)
    {
        m_portName = GetPortName();
        if (m_portName == "")
        {
            emit ImuData(QByteArray("None"));
        }
        else
        {

            break;
        }
        QThread::msleep(100);
    }
    QString currentPortName;
    if (currentPortName != m_portName)
    {
        currentPortName = m_portName;
        currentPortNameChanged = true;
    }

    int currentWaitTimeout = m_waitTimeout;

    m_mutex.unlock();
    QSerialPort serial;

    while (!m_quit)
    {
        if (currentPortName.isEmpty())
        {
            emit error(tr("Port not set"));
            // emit ImuData("port not set");
            return;
        }
        else if (currentPortNameChanged)
        {
            serial.close();

            serial.setPortName(currentPortName);
            serial.setBaudRate(m_baudrate);
            currentPortNameChanged = false;
            QThread::msleep(100);
            if (!serial.open(QIODevice::ReadWrite))
            {
                // qDebug()<<"somehow got here!";
                // currentPortName = FindAndChangePortName();
                emit ImuData("None");
            }
        }

        if (serial.waitForReadyRead(currentWaitTimeout))
        { // read request
            QByteArray requestData = serial.readAll();
            while (serial.waitForReadyRead(5))
                requestData += serial.readAll();
            emit ImuData(requestData);
        }
        else
        {
            currentPortNameChanged=true;
            emit timeout(tr("Wait read request timeout %1")
                             .arg(QTime::currentTime().toString()));
        }
    }
}

void ReceiverThread::stop()
{
    this->m_quit = true;
}
