#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsItem>
#include <QtSql>
#include <stdlib.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // include rssi's later
    int rsAB,rsAC,rsAD,rsBC,rsBD,rsCD;
    qreal AB,AC,AD,BC,BD,CD; //Length
    qreal t1_a,t1_b,t1_c; //triangle1 angles  //ANGLES ARE IN RADIANS
    qreal t2_a,t2_b,t2_c; //triangle2 angles
    qreal Ax,Ay,Bx,By,Cx,Cy,Dx,Dy; //Position

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    qreal getAngle(qreal a,qreal b,qreal c);
    qreal getX(qreal ang,qreal hypot);
    qreal getY(qreal ang,qreal hypot);
    qreal getRSSI(QString node, QString mac);
    qreal RSSItoLength(int RSSI);
    void mainLoop();

    void drawNodes();

    QGraphicsScene* myScene;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
