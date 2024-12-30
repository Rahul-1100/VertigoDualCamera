#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QCamera>
#include <QCameraDevice>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMainWindow>
#include <QMediaRecorder>
#include <QMutex>
#include <QRegExp>
#include <QSerialPort>
#include <QSharedPointer>
#include <QThread>
#include <QVector>
#include <QtMultimediaWidgets>
#include "./ui_mainwindow.h"
#include "modifiedWidgets.h"



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE



class ReceiverThread : public QThread
{
    Q_OBJECT

public:
    explicit ReceiverThread(QObject *parent = nullptr);
    ~ReceiverThread();

    void startReceiver(int buadrate, int waitTimeout);
    void stop();

signals:
    void error(const QString &s);
    void timeout(const QString &s);
    void ImuData(const QByteArray &s);

private:


    void run() override;
    static QString GetPortName();
    QString FindAndChangePortName();
    QString m_portName;
    QString m_response;
    bool currentPortNameChanged = false;
    int m_baudrate;
    int m_waitTimeout = 0;
    QMutex m_mutex;
    bool m_quit = false;
    
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private:
    
    const QString recordingFolder = "./Recorder";

    const QString recordedFolder = "./Recordings";

    Ui::MainWindow *ui;
    LogDialog *logDialog;
    QTimer *timer;
    ReceiverThread* ImuThread;
    QMutex calibration_locker;
    QMutex CameraDevice_locker; //locks the cameradevice vector when updating or setting the value as this might crash the code as multiple combobox can be updting it simultaneously 

    QRegularExpression gyro_pattern;// ("Gyro: (-?\\d+\\.\\d+), (-?\\d+\\.\\d+), (-?\\d+\\.\\d+)");
    QRegularExpression quat_pattern;//("Quat: (-?\\d+\\.\\d+), (-?\\d+\\.\\d+), (-?\\d+\\.\\d+), (-?\\d+\\.\\d+)");
    QRegularExpression cal_pattern;//("Cal: (\\d+)");
    std::unique_ptr<QVector<QString>> Gyro_values;
    std::unique_ptr<QVector<QString>> Quat_values;
    std::unique_ptr<int> Cal;
    QList<QRegularExpression> patterns;
    std::unique_ptr<QVector<QVideoWidget*>> videoWidgets;
    std::unique_ptr<QVector<ClickableComboBox*>> CameraBoxes;
    std::unique_ptr<QVector<QMediaCaptureSession*>> MediaCaptureSessions;
    std::unique_ptr<QVector<QMediaRecorder*>> MediaRecorders;
    std::unique_ptr<QVector<QCamera*>> Cameras;
    std::unique_ptr<QVector<QCameraDevice>> CameraDevices;
    std::unique_ptr<QFile> csvFile;
    std::unique_ptr<QTextStream> csvOut;
    std::unique_ptr<QFile> metadatafile;
    std::unique_ptr<QTextStream> metadataOut;
    std::unique_ptr<QDir> recordingDir;
    std::unique_ptr<QDir> recordedDir;
    std::unique_ptr<bool>gettingImuValues;//needed so that when we get the first values of imu to enable it otherwise it will always be printing "got other values"
    std::unique_ptr<QElapsedTimer> recordingTimer;
    std::unique_ptr<bool>recordingStarted;
    bool gotImu3threeTimes = false;
    int countToThree = 0;
    int previousCalibration = -1;
    QString folderName;
    
    QList<bool> gotAllImuValues;
    
    

    ClickableComboBox* changeToClickableComboBox(QComboBox*);
    QVideoWidget* changeLabelToVideoWidget(QLabel*);
    static QLayout *parentLayout(QWidget*);
    static QLayout *parentLayout(QWidget*,QLayout*);

    
    static void getPortDescription();
    static void showError(const QString&); 
    void getAvailableCameras(int who);
    void setCameraToWidget(int index,int WidgetIndex);
    void readSerialData(QByteArray);
    void startRecording();
    void stopRecording();
    void onCopyCompleted();
    void onPageChanged(int);
    void copyFiles(QString OrignalFilePath,QString DestinationPath);
    void setupCamera(QMediaCaptureSession* , QVideoWidget* , ClickableComboBox*,QCamera*,QMediaRecorder*, int index);
    void errors(QMediaRecorder::Error,QString);
    void setToRecordingPage(void);
    void setToStartingPage(void);
    void openLogDialog(void);
    void checkImuCalibration(void);
   

private slots:
    void StartStitcher();
    void updateElapsedTime(void);
    void OpenRecordingsFolder();

signals:
    void imuDataRecieved(QString&);

    
};



#endif // MAINWINDOW_H
