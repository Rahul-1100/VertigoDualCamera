#include "videostitcher.h"
#include "ui_videostitcher.h"

VideoStitcher::VideoStitcher(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VideoStitcher)
{
    ui->setupUi(this);
}

VideoStitcher::~VideoStitcher()
{
    delete ui;
}
