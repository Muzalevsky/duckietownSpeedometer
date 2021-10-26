#include "mainwindow.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QString prev_img_path = a.applicationDirPath() + "/../data/frame0015.jpg";
    QString cur_img_path = a.applicationDirPath() + "/../data/frame0016.jpg";

    Point prev = w.getPointFromImage(prev_img_path);
    Point cur = w.getPointFromImage(cur_img_path);

    qDebug() <<"delta=" << cur.x - prev.x << cur.y - prev.y;

    return a.exec();
}
