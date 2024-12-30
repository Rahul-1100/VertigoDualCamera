#include "videoexporter.h"
#include "ui_videoexporter.h"
#include <QStandardItemModel>
#include <QDir>
#include <QCheckBox>
#include <QProgressBar>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

VideoExporter::VideoExporter(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoExporter)
    , model(new QStandardItemModel(0, 3, this))
{
    ui->setupUi(this);
    this->setWindowTitle("Convert Files");
    this->setWindowIcon(QIcon(":/assets/icon.ico"));
    model->setHorizontalHeaderLabels(QStringList() << "Select" << "Folder Name" << "Progress");
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);

    QDir directory("./Recordings");

    QStringList folders = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &folderName : folders) {
        QList<QStandardItem *> rowItems;

        // Checkbox item
        QStandardItem *checkBoxItem = new QStandardItem();
        checkBoxItem->setCheckable(true);
        checkBoxItem->setCheckState(Qt::Checked);
        rowItems.append(checkBoxItem);

        // Folder name item
        QStandardItem *folderNameItem = new QStandardItem(folderName);
        // Store the full path as user data
        folderNameItem->setData(directory.absoluteFilePath(folderName), Qt::UserRole);
        rowItems.append(folderNameItem);

        // Progress bar item
        QStandardItem *progressBarItem = new QStandardItem(" ");
        rowItems.append(progressBarItem);

        model->appendRow(rowItems);
    }

    // Connect the button's clicked signal to the slot
    connect(ui->pushButton_2, &QPushButton::clicked, this, &VideoExporter::Convert);
    connect(ui->pushButton, &QPushButton::clicked, this, &VideoExporter::deleteFile);
}

VideoExporter::~VideoExporter()
{
    delete ui;
}

void VideoExporter::Convert()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(this, "Select where to put converted videos", QDir::homePath());
    QList<QString> commands;
    
    VideoStitcherThread *videoStitching = new VideoStitcherThread(this);
    for (int row = 0; row < model->rowCount(); ++row) {
        QStandardItem *folderNameItem = model->item(row, 1); // Column 1 is the folder name
        if (folderNameItem) {
            // Retrieve the full path from user data
            QString fullPath = folderNameItem->data(Qt::UserRole).toString();
            // fullPath = fullPath.replace("\\", "\\\\");
            // fullPath = fullPath.replace("/", "\\"); // Convert to Windows path
            // selectedDirectory = selectedDirectory.replace("/", "\\"); // Convert to Windows path
            // Escape backslashes
            // qDebug()<< "directory Path:" << selectedDirectory;
            // qDebug() << "Folder Path:" << fullPath;
            QString command = QString("VideoStitcherPy.exe --dir \"%1\" --output_dir \"%2\" --output_filename \"%3.mp4\"")
                                .arg(fullPath)
                                .arg(selectedDirectory)
                                .arg(folderNameItem->text());
            QStandardItem *checkBoxItem = model->item(row, 0); // Column 0 is the checkbox
            if (checkBoxItem && checkBoxItem->checkState() == Qt::Checked) {
                commands.append(command);
            }
            
        }
        for(const QString &command:commands){
            // qDebug()<<command;
        }
        videoStitching->commands = commands;

        connect(videoStitching, &VideoStitcherThread::StitcherOutputs, this, &VideoExporter::PrintStatus);
        videoStitching->start();

    }
}
void VideoExporter::PrintStatus(const QString &status)
{
    // qDebug() << status;
    if (status.contains("Extracted string:")) {
        QDir dir;
        // Extract the folder name from the status message
        QString folderName = status.section(' ', -1);
        // qDebug()<<folderName;
        dir.setPath(folderName);
        if (dir.exists(folderName)) {
                    dir.removeRecursively();
                    qDebug() << "Deleted folder:" << folderName;
                }
                QString lastFolderName = folderName.section('/', -1, -1);
                for (int row = 0; row < model->rowCount(); ++row) {
                    QStandardItem *folderNameItem = model->item(row, 1); // Column 1 is the folder name
                    if (folderNameItem && folderNameItem->text() == lastFolderName) {
                    QStandardItem *progressBarItem = model->item(row, 2); // Column 2 is the progress bar
                    if (progressBarItem) {
                        progressBarItem->setText("Completed");
                    }
                    break; // Exit the loop once the folder is found and updated
                    }
                }
        
    }
}
void VideoExporter::deleteFile()
{
    QDir dir;
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "Delete Folders", "Are you sure that you will be deleting these folders?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (int row = model->rowCount() - 1; row >= 0; --row) {
            QStandardItem *checkBoxItem = model->item(row, 0); // Column 0 is the checkbox
            if (checkBoxItem && checkBoxItem->checkState() == Qt::Checked) {
                QStandardItem *folderNameItem = model->item(row, 1); // Column 1 is the folder name
                if (folderNameItem) {
                    QString fullPath = folderNameItem->data(Qt::UserRole).toString();
                    QDir dir(fullPath);
                    if (dir.exists()) {
                        dir.removeRecursively();
                        // qDebug() << "Deleted folder:" << fullPath;
                    }
                    model->removeRow(row);
                }
            }
        }
    }
    QString folderName = "Recordings";
    dir.setPath(folderName);
    if (dir.exists(folderName)) {
        dir.removeRecursively();
        // qDebug() << "Deleted folder:" << folderName;
    }
}

void VideoExporter::openInNewWindow()
{
    VideoExporter *newWindow = new VideoExporter();
    newWindow->setWindowFlags(Qt::Dialog);
    newWindow->setAttribute(Qt::WA_DeleteOnClose);
    newWindow->show();
}
