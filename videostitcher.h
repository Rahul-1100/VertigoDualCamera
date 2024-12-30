#ifndef VIDEOSTITCHER_H
#define VIDEOSTITCHER_H

#include <QMainWindow>

namespace Ui {
class VideoStitcher;
}

class VideoStitcher : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoStitcher(QWidget *parent = nullptr);
    ~VideoStitcher();

private:
    Ui::VideoStitcher *ui;
};

#endif // VIDEOSTITCHER_H
