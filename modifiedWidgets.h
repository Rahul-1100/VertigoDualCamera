#ifndef CLICKABLECOMBOBOX_H
#define CLICKABLECOMBOBOX_H

#include <QComboBox>
#include <QMouseEvent>
#include <QDialog>
#include <QFile>
#include <QPlainTextEdit>
#include <QThread>

class ClickableComboBox : public QComboBox
{
    Q_OBJECT
public:
    ClickableComboBox(QWidget* parent = nullptr);
    ~ClickableComboBox();
    void mousePressEvent(QMouseEvent* Event) override;


signals:
    void clicked();
};

class FileCopyWorker : public QObject {
    Q_OBJECT

public:
    FileCopyWorker();
    ~FileCopyWorker();

public slots:
    void copyFile(const QString &, const QString &);

signals:
    void copyCompleted(bool success);
};

class LogDialog : public QDialog {
    Q_OBJECT

public:
    explicit LogDialog(QWidget *parent = nullptr);
    void setLogData(const QString &data); // Initialize with existing data
    void appendLogData(const QString &data); // Append new data dynamically

private:
    QPlainTextEdit *logArea;
};


class VideoStitcherThread : public QThread
{
    Q_OBJECT
public:
    explicit VideoStitcherThread(QObject *parent = nullptr);
    ~VideoStitcherThread();
    QList<QString> commands;
private:
    

    void run() override;
    void executeVideoThread(const std::string &command);
signals:
    void StitcherOutputs(const QString &s);
};



#endif
