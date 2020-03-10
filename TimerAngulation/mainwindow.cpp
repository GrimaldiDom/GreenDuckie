#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ///////////
    db.setDatabaseName(":memory:");

    if( !db.open() )
    {
        qDebug() << db.lastError();
        qDebug() << "Error: Unable to connect due to above error!";
    }

    QSqlQuery q(db); ////power(int id,int time,text station,text mac,int power)
    q.exec("CREATE TABLE power(id INT PRIMARY_KEY AUTO_INCREMENT,time INT, station TEXT,mac TEXT,rssi INT)");
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','macB',-60)"); //ab
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','macC',-54)"); //ac
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','macD',-69)"); //ad

         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','macC',-66)"); //bc
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','macD',-60)"); //bd

         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'C','macD',-60)"); //bd

         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','obj',-50)"); //object from a
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','obj',-55)"); //object from b
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'C','obj',-57)"); //object from c
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'D','obj',-54)"); //object from d


    ///////////
    //set constants
         Ax=0;
         Ay=0;

         By=0;
    //initilize scene
        myScene = new QGraphicsScene(this);
          myScene->setItemIndexMethod(QGraphicsScene::NoIndex); //speeds up?
          myScene->setSceneRect(-200,-150,400,300);
          myScene->setBackgroundBrush(Qt::white);

        ui->graphicsView->setScene(myScene);
          ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    //initilize timer
        QTimer *timer = new QTimer(this);
         connect(timer,&QTimer::timeout,this,&MainWindow::mainLoop);
         timer->start(333); //millisecconds

         srand(1776);
         mainLoop(); //run once
}



void MainWindow::mainLoop()
{
    //Get RSSI into Length
       QSqlQuery q(db);

       q.exec("SELECT rssi FROM power WHERE station = 'A' AND mac = 'macB'");
       q.last();
       rsAB = q.value(0).toInt();
       q.exec("SELECT rssi FROM power WHERE station = 'A' AND mac = 'macC'");
       q.last();
       rsAC = q.value(0).toInt();
       q.exec("SELECT rssi FROM power WHERE station = 'A' AND mac = 'macD'");
       q.last();
       rsAD = q.value(0).toInt();

       q.exec("SELECT rssi FROM power WHERE station = 'B' AND mac = 'macC'");
       q.last();
       rsBC = q.value(0).toInt();
       q.exec("SELECT rssi FROM power WHERE station = 'B' AND mac = 'macD'");
       q.last();
       rsBD = q.value(0).toInt();

       q.exec("SELECT rssi FROM power WHERE station = 'C' AND mac = 'macD'");
       q.last();
       rsCD = q.value(0).toInt();

       AB = RSSItoLength(rsAB);
       AC = RSSItoLength(rsAC);
       AD = RSSItoLength(rsAD);

       BC = RSSItoLength(rsBC);
       BD = RSSItoLength(rsBD);

       CD = RSSItoLength(rsCD);

       qDebug() << "AB: " << AB;
       qDebug() << "AC: " << AC;
       qDebug() << "AC: " << BC;

    //Solve triangles
        //Solve triangle 1
            ////AB = c,AC = b,CB =a
            t1_a = getAngle(AC,AB,BC);
            t1_b = getAngle(AB,BC,AC);
            t1_c = getAngle(BC,AC,AB);
                //qDebug()<<"A1"<< t1_a<<"B1"<< t1_b<<"C1"<< t1_c<<endl;
        //Solve triangle 2
            ////BD = c, CD = b,BC = a
            t2_a = getAngle(CD,BD,BC);
            t2_b = getAngle(BD,BC,CD); //CHECKED FOR ACC
            t2_c = getAngle(BC,CD,BD);
                //qDebug()<<"A2"<< t2_a<<"B2"<< t2_b<<"C2"<< t2_c<<endl;
    //Get node cords
    //// DUE TO 2D NATURE OF AB, CD COULD BE ABOVE AB. Below AB is always assumed if CD is above map will be upside down but still functional!
        Bx=AB;

        Cx=getX(t1_a,AC) + 0; // can be either + or - (Depends on if ang A is acute(+) or obtuse(-))
        Cy=getY(t1_a,AC) + 0; //will be + but + is down

        Dx=getX(t2_b,BD) + AB;// can be either + or - (- if ang B is acute and Dx > AB)
        Dy=getY(t2_b,BD) + 0; // will be + but + is down
    //Detect Objects
        q.exec("SELECT mac,station,rssi FROM power WHERE mac != 'macA' AND mac != 'macB' AND mac != 'macC' AND mac != 'macD'");

        q.first();
        do
        {
            qDebug() << q.isActive();
            qDebug() << q.isValid();

            qDebug() << "Object: "<<q.value(0).toString()<<" Station: "<<q.value(1).toString() << " RSSI: " << q.value(2).toInt();
        }while(q.next());
    //Locate Objects

    //Draw nodes
        //clear Scene?
            myScene->clear();
        //connect points
             int Scale = 30;
             int pointScale = 4;
            myScene->addLine(Ax*Scale,Ay*Scale,Bx*Scale,By*Scale,QPen(Qt::gray));
            myScene->addLine(Ax*Scale,Ay*Scale,Cx*Scale,Cy*Scale,QPen(Qt::gray));
            myScene->addLine(Bx*Scale,By*Scale,Dx*Scale,Dy*Scale,QPen(Qt::gray));
            myScene->addLine(Cx*Scale,Cy*Scale,Dx*Scale,Dy*Scale,QPen(Qt::gray));
        //draw points
            myScene->addEllipse(Ax*Scale-.5*pointScale,Ay*Scale-.5*pointScale,pointScale,pointScale,QPen(Qt::red),QBrush(Qt::red));       //a    RED
            myScene->addEllipse(Bx*Scale-.5*pointScale,By*Scale-.5*pointScale,pointScale,pointScale,QPen(Qt::yellow),QBrush(Qt::yellow)); //b    YELLOW
            myScene->addEllipse(Cx*Scale-.5*pointScale,Cy*Scale-.5*pointScale,pointScale,pointScale,QPen(Qt::blue),QBrush(Qt::blue));     //c    BLUE
            myScene->addEllipse(Dx*Scale-.5*pointScale,Dy*Scale-.5*pointScale,pointScale,pointScale,QPen(Qt::green),QBrush(Qt::green));   //d    GREEN
        //draw length
            QGraphicsTextItem *textAB = new QGraphicsTextItem(QString::number(AB));
                textAB->setPos(Ax*Scale + Scale,Ay*Scale-Scale);
            QGraphicsTextItem *textAC = new QGraphicsTextItem(QString::number(AC));
                textAC->setPos(Ax*Scale-Scale,Ay*Scale+Scale);
            QGraphicsTextItem *textBD = new QGraphicsTextItem(QString::number(BD));
                textBD->setPos(Bx*Scale+Scale,Ay*Scale+Scale);
            QGraphicsTextItem *textCD = new QGraphicsTextItem(QString::number(CD));
                textCD->setPos(Cx*Scale+Scale,Cy*Scale+Scale);

            myScene->addItem(textAB);
            myScene->addItem(textAC);
            myScene->addItem(textBD);
            myScene->addItem(textCD);


          q.prepare("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','macB',:ssi)"); //ab
            q.bindValue(":ssi",-75 + ((rand()%2) -1));
            q.exec();
          q.prepare("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','macC',:ssi)"); //ac
            q.bindValue(":ssi",-60 + ((rand()%2) -1));
            q.exec();
          q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','macD',:ssi)"); //ad
            q.bindValue(":ssi",-87 + ((rand()%2) -1));
            q.exec();

          q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','macC',-85)"); //bc
          q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','macD',-64)"); //bd

          q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'C','macD',-89)"); //cd

            return;
}
qreal MainWindow::RSSItoLength(int RSSI)
{
    return (RSSI+42.183)/(-5.6743);   //estimated liner function
}

qreal MainWindow::getAngle(qreal a,qreal b,qreal c)
{
    return qAcos((a*a+b*b-c*c)/(2*a*b));
}
qreal MainWindow::getX(qreal ang,qreal hypot) //uses absolute angle from AB
{
    qreal alpha;
    if(ang > M_PI_2)
        {alpha = ang - M_PI_2 +M_PI;} //makes x negative
    else
        {alpha = M_PI_2 - ang;} // x is positive

    return (qSin(alpha)*hypot);

}
qreal MainWindow::getY(qreal ang,qreal hypot) //uses absolute angle from AB
{
    qreal alpha;
    if(ang > M_PI_2)
        {alpha = ang - M_PI_2;} //y is always negative
    else
        {alpha = M_PI_2 - ang;} // y is always negative

    return (qCos(alpha)*hypot);
}

MainWindow::~MainWindow()
{
    delete ui;
}
