#include "modifiedWidgets.h"
#include <QComboBox>
#include <QMouseEvent>
#include <windows.h>
#include <iostream>
#include <QThread>
#include <QEventLoop>
#include <QRegExp>

ClickableComboBox::ClickableComboBox(QWidget* parent):QComboBox(parent){}

ClickableComboBox::~ClickableComboBox(){

}

void ClickableComboBox::mousePressEvent(QMouseEvent* Event) {
    emit clicked();
    QComboBox::mousePressEvent(Event);
}

FileCopyWorker::FileCopyWorker(){}
FileCopyWorker::~FileCopyWorker(){}

void FileCopyWorker::copyFile(const QString &sourcePath, const QString &destinationPath) {
    // Perform the file copy operation
    // qDebug() << "Starting file copy from " << sourcePath << " to " << destinationPath<<"with size of"<<QFile(sourcePath).size();
    // QThread::msleep(1000);

    QFile sourceFile(sourcePath);
    // QFile destinationFile(destinationPath);

    // Check if the source file exists
    if (!sourceFile.exists()) {
        // qDebug() << "Source file does not exist:" << sourcePath;
        emit copyCompleted(false);
        return;
    }

    // If the destination file exists, remove it
    // if (destinationFile.exists()) {
    //     if (!QFile::remove(destinationPath)) {
    //         qDebug() << "Failed to remove existing destination file:" << destinationPath;
    //         emit copyCompleted(false);
    //         return;
    //     }
    // }
    
    

    // Copy the source file to the destination
    if (QFile::copy(sourcePath,destinationPath)) {
        // qDebug() << "File copied successfully from " << sourcePath << " to " << destinationPath;
        emit copyCompleted(true);
    } else {
        // qDebug() << "File copy failed: " << sourceFile.errorString();
        emit copyCompleted(false);
    }
}

LogDialog::LogDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("UART Logs");
    resize(600, 400);

    logArea = new QPlainTextEdit(this);
    logArea->setReadOnly(true);
    logArea->resize(size());
}

void LogDialog::setLogData(const QString &data) {
    logArea->setPlainText(data); // Display existing data
}

void LogDialog::appendLogData(const QString &data) {
    logArea->appendPlainText(data); // Dynamically append new data
}

VideoStitcherThread::VideoStitcherThread(QObject *parent):QThread(parent){}
VideoStitcherThread::~VideoStitcherThread(){}

void VideoStitcherThread::run() {
    for(const QString &command:commands){
        executeVideoThread(command.toStdString());
        // qDebug()<<"executed command"<<command;
        QRegExp regex("--dir\\s+\"([^\"]+)\"\\s+--output_dir");
        // QRegExp regex("--dir\\s+([^\\s]+)\\s+--output_dir");
        if (regex.indexIn(command) != -1) {
            QString extractedString = regex.cap(1);
            emit StitcherOutputs("Extracted string: " + extractedString);
        } else {
            emit StitcherOutputs("No match found in command: " + command);
        }
        // emit StitcherOutputs("done"+command);
        // QEventLoop loop;
        // connect(this, &VideoStitcherThread::StitcherOutputs, [&loop](const QString &output) {
        //     qDebug() << output;
        //     loop.quit();
        // });
        // loop.exec();
    }

    
}

void VideoStitcherThread::executeVideoThread(const std::string &command) {
    // Execute the command and get the output
    HANDLE hStdOutRead = nullptr;
    HANDLE hStdOutWrite = nullptr;

    // Create a pipe for the child process's STDOUT
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
        // emit StitcherOutputs("Failed to create pipe.");
        return;
    }

    // Ensure the read handle to the pipe is not inherited
    if (!SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(hStdOutRead);
        CloseHandle(hStdOutWrite);
        // emit StitcherOutputs("Failed to set handle information.");
        return;
    }

    // Create the child process
    PROCESS_INFORMATION pi = {};
    STARTUPINFO si = {};
    si.cb = sizeof(STARTUPINFO);
    si.hStdOutput = hStdOutWrite;
    si.hStdError = hStdOutWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    // Convert command to wide string
    std::wstring wCommand(command.begin(), command.end());

    if (!CreateProcess(
            nullptr,               // Application name
            &wCommand[0],          // Command line
            nullptr,               // Process security attributes
            nullptr,               // Thread security attributes
            TRUE,                  // Inherit handles
            0,                     // Creation flags
            nullptr,               // Environment
            nullptr,               // Current directory
            &si,                   // STARTUPINFO
            &pi                    // PROCESS_INFORMATION
            )) {
        CloseHandle(hStdOutRead);
        CloseHandle(hStdOutWrite);
        // emit StitcherOutputs("Failed to create process.");
        return;
    }

    // Close the write end of the pipe in the parent process
    CloseHandle(hStdOutWrite);

    // Read the output from the child process
    char buffer[128];
    DWORD bytesRead;
    while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        // emit StitcherOutputs(QString::fromStdString(buffer));
    }

    // Clean up handles
    CloseHandle(hStdOutRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
