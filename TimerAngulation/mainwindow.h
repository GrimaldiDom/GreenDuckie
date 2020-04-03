#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsItem>
#include <QtSql>
#include <stdlib.h>

#include "objects.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int rsAB,rsAC,rsAD,rsBC,rsBD,rsCD;  //Node RSSI
    qreal AB,AC,AD,BC,BD,CD;            //Node lengths
    qreal t1_a,t1_b,t1_c;               //Node triangle1 angles  //ANGLES ARE IN RADIANS
    qreal t2_a,t2_b,t2_c;               //Node triangle2 angles
    qreal Ax,Ay,Bx,By,Cx,Cy,Dx,Dy;      //Node positions

    int cntr,index; //counters
    qreal t3_a,t3_b,t3_c;               //Object triangle1 angles (Temporary)
    qreal t4_a,t4_b,t4_c;               //Object triangle2 angles (Temporary)

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    qreal getAngle(qreal a,qreal b,qreal c);
    qreal getX(qreal ang,qreal hypot); //WORKS DIFFERENT FROM OBJECT CORDS// makes assumptions and uses triangulation angle
    qreal getY(qreal ang,qreal hypot);
    qreal getRSSI(QString node, QString mac);
    qreal RSSItoLength(int RSSI);
    qreal getObjX(qreal abs_ang,qreal lengA); //WORKS DIFFERENT FROM NODE CORDS// this uses absolute angle
    qreal getObjY(qreal abs_ang,qreal lengA);

    void mainLoop();


    object* obj[128];
    QStringList objList;
    int curID;
    QGraphicsScene* myScene;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
