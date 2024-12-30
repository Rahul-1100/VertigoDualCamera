#ifndef VIDEOEXPORTER_H
#define VIDEOEXPORTER_H

#include <QWidget>
#include "modifiedWidgets.h"
#include <QStandardItemModel>

namespace Ui {
class VideoExporter;
}
/*
thsis class has a QtTable class and 2 buttons named convert and close use videoStitcher class from modifiedWidget.h and make this class open a directory which will have names of folder, make a table from the named folders and */
class VideoExporter : public QWidget
{
    Q_OBJECT

public:
    explicit VideoExporter(QWidget *parent = nullptr);
    ~VideoExporter();
    void openInNewWindow();
    void Convert();

private:
    Ui::VideoExporter *ui;
    QStandardItemModel *model;
    private slots:
    void PrintStatus(const QString &status);
    void deleteFile();

};

#endif // VIDEOEXPORTER_H
