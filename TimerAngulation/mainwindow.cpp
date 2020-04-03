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
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','macB',-71)"); //ab =
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','macC',-54)"); //ac =
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','macD',-77)"); //ad =

         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','macC',-73)"); //bc =
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','macD',-59)"); //bd =

         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'C','macD',-73)"); //bd =

         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','45-4A-AC-41-53-5F',-58)"); //object1 from a      ///// concurrent test (success)
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','45-4A-AC-41-53-5F',-62)"); //object1 from b
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'C','45-4A-AC-41-53-5F',-54)"); //object1 from c
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'D','45-4A-AC-41-53-5F',-62)"); //object1 from d


         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','12-3E-20-01-14-CF',-65)");         //            ////radmonized test (success)
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','29-79-2C-75-F1-D8',-81)");
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'C','12-3E-20-01-14-CF',-70)");
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'C','29-79-2C-75-F1-D8',-50)");
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','61-80-57-0F-36-5A',-75)");
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'C','61-80-57-0F-36-5A',-78)");
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','29-79-2C-75-F1-D8',-56)");
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','61-80-57-0F-36-5A',-78)");
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'D','61-80-57-0F-36-5A',-80)");
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'D','29-79-2C-75-F1-D8',-81)");
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'D','12-3E-20-01-14-CF',-66)");
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','12-3E-20-01-14-CF',-53)");

         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'D','x1',-62)"); //object from d                     //// incomplete Node test (Acknowledged but not drawn)
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'D','x2',-62)"); //object from d

         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'X','y3',-62)"); //object from d                     //// invalid Node test  (Ignored)
         q.exec("INSERT INTO power(time,station,mac,rssi) VALUES(0,'dhbfahfv','y4',-62)"); //object from d



    ///////////
    //set constants
         Ax=0;
         Ay=0;
         By=0;

         cntr=0;
         curID = 0;
         QString tempText;
         srand(1776);
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

    //Main Loop
         mainLoop(); //run once
         timer->start(1000); //millisecconds
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
            ////AB = c,AC = b,BC =a
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
        q.exec("SELECT mac,station,rssi,ROWID FROM power WHERE mac != 'macA' AND mac != 'macB' AND mac != 'macC' AND mac != 'macD'");

        q.first();
        /////////////////////////////////////TODO///////////////////////////////////////////
        //get only most recent A,B,C,D entries (USE IDS)

        //change the above exec to a prepare and add id>=
        ////////////////////////////////////////////////////////////////////////////////////
        do
        {
            if( q.isActive() && q.isValid())
            {
                qDebug()<< q.value(3).toInt() << "//////////////////////////////////////////";
                if(!objList.contains(q.value(0).toString())) ////////////new object
                {
                    //add to list
                    objList.append(q.value(0).toString());
                    //allocate mem
                    obj[cntr] = new object;
                    //name object
                    obj[cntr]->MAC = q.value(0).toString();

                    //get node and rssi
                    //calculate length from node
                    //set node x length as active
                    if(q.value(1).toString() == "A")
                    {
                        obj[cntr]->lenA = RSSItoLength(q.value(2).toInt());
                        obj[cntr]->isA=true;
                    }
                    else if(q.value(1).toString() == "B")
                    {
                        obj[cntr]->lenB = RSSItoLength(q.value(2).toInt());
                        obj[cntr]->isB=true;
                    }
                    else if(q.value(1).toString() == "C")
                    {
                        obj[cntr]->lenC = RSSItoLength(q.value(2).toInt());
                        obj[cntr]->isC=true;
                    }
                    else if(q.value(1).toString() == "D")
                    {
                        obj[cntr]->lenD = RSSItoLength(q.value(2).toInt());
                        obj[cntr]->isD=true;
                    }
                    else{qDebug()<<"OBJECT TRACKING ERROR";}

                    qDebug() <<"ADDED new object of mac: " << obj[cntr]->MAC;
                    cntr++;
                }
                else ///////////////////////////existing object
                {
                    index = objList.indexOf(q.value(0).toString());
                    //get node and rssi
                    //calculate length from node
                    //set node x length as active
                    if(q.value(1).toString() == "A")
                    {
                        obj[index]->lenA = RSSItoLength(q.value(2).toInt());
                        obj[index]->isA=true;
                    }
                    else if(q.value(1).toString() == "B")
                    {
                        obj[index]->lenB = RSSItoLength(q.value(2).toInt());
                        obj[index]->isB=true;
                    }
                    else if(q.value(1).toString() == "C")
                    {
                        obj[index]->lenC = RSSItoLength(q.value(2).toInt());
                        obj[index]->isC=true;
                    }
                    else if(q.value(1).toString() == "D")
                    {
                        obj[index]->lenD = RSSItoLength(q.value(2).toInt());
                        obj[index]->isD=true;
                    }
                    else{qDebug()<<"OBJECT TRACKING ERROR";}

                    qDebug() <<"UPDATED object of mac: " << obj[index]->MAC;
                }
                qDebug() << "Object: "<<q.value(0).toString()<<" Station: "<<q.value(1).toString() << " RSSI: " << q.value(2).toInt();
            }


        }while(q.next());
    //Locate Objects
        qDebug()<<"there are "<< cntr <<" known objects currently:";
        for(int i = 0;i<cntr;i++)
        {

          qDebug()<<"//////////Object "<< i +1 <<" with MAC address:"<<obj[i]->MAC << "//////////";

          if(obj[i]->isA && obj[i]->isB && obj[i]->isC)  //has A B C
          {
             //a=BO;b=AO;c=AB
              t3_a = getAngle(obj[i]->lenA, AB          , obj[i]->lenB);
              t3_b = getAngle(AB          , obj[i]->lenB, obj[i]->lenA); ///Solve A B O
              t3_c = getAngle(obj[i]->lenB, obj[i]->lenA, AB          );

              qDebug()<< "a: "<<obj[i]->lenA<< " b "<<obj[i]->lenB<< " c " << obj[i]->lenC<< " d " << obj[i]->lenD;

            //a=CO,AO,AC
              t4_a = getAngle(obj[i]->lenA, AC          , obj[i]->lenC);
              t4_b = getAngle(AC          , obj[i]->lenC, obj[i]->lenA            ); ///Solve A C Oz mz
              t4_c = getAngle(obj[i]->lenC, obj[i]->lenA, AC          ); ////Node C eliminates the mirror ambiguity

            //Calculate both (+)x(+)y  and mirriored (x)x(-)y using A B O
            //Calculate (+)x(+)y and mirriored (-)x and (+)y using A C O
            //find which set of x and y match thats the real cordniants
              qDebug()<< "T1:\n ang a: "<<t1_a * 360 /(2*M_PI)<< " ang b "<<t1_b * 360 /(2*M_PI)<< " ang c " << t1_c* 360 /(2*M_PI);
              qDebug()<< "T2:\n ang a: "<<t2_a* 360 /(2*M_PI)<< " ang b "<<t2_b* 360 /(2*M_PI) << " ang c " << t2_c* 360 /(2*M_PI);
              qDebug()<< "T3:\n ang a: "<<t3_a * 360 /(2*M_PI)<< " ang b "<<t3_b * 360 /(2*M_PI)<< " ang c " << t3_c* 360 /(2*M_PI);
              qDebug()<< "T4:\n ang a: "<<t4_a* 360 /(2*M_PI)<< " ang b "<<t4_b* 360 /(2*M_PI) << " ang c " << t4_c* 360 /(2*M_PI);
              qDebug()<< " has ABC";

              if( t1_a > t3_a && t1_a> t4_a)                 //// Below AB Right of AC
              {
                  obj[i]->x = getObjX(t3_a,obj[i]->lenA);
                  obj[i]->y = getObjY(t3_a,obj[i]->lenA);
                  obj[i]->isCords=true;
              }
              else if(t3_a > t1_a && t1_a > t4_a)             //// Below AB Left of AC
              {
                  obj[i]->x = getObjX(t3_a,obj[i]->lenA);
                  obj[i]->y = getObjY(t3_a,obj[i]->lenA);
                  obj[i]->isCords=true;
              }
              else if(t4_a > t1_a && t1_a > t3_a)             //// Above AB Right of AC
              {
                  obj[i]->x = getObjX(2*M_PI-t3_a,obj[i]->lenA);
                  obj[i]->y = getObjY(2*M_PI-t3_a,obj[i]->lenA);
                  obj[i]->isCords=true;
              }
              else if(t4_a + t3_a > t1_a)  //// Above AB Left of AC //Could happen if rounding screws it
              {
                  obj[i]->x = getObjX(2*M_PI-t3_a,obj[i]->lenA);
                  obj[i]->y = getObjY(2*M_PI-t3_a,obj[i]->lenA);
                  obj[i]->isCords=true;
              }
              else                                                 //// Shouldn't happen but who knows maybe its possible rounding causes this
              {
                  qDebug() <<"OBJECT ANGLE ERROR||OBJECT ANGLE ERROR||OBJECT ANGLE ERROR||OBJECT ANGLE ERROR||OBJECT ANGLE ERROR||OBJECT ANGLE ERROR||OBJECT ANGLE ERROR||OBJECT ANGLE ERROR ";
              }
              qDebug()<< "x1 "<<obj[i]->x << " y1 "<<obj[i]->y;
          }
          else if(obj[i]->isA && obj[i]->isB && obj[i]->isD) //or has A B D
          {
                ///////LOWER PRIORITY TODO/////////////////////
              qDebug()<< " lacking ABC but has ABD not yet implimented";
          }
          else
          {
            qDebug()<< " lacking node coverage";
          }

        }
    //Draw nodes
        //clear Scene?
            myScene->clear();

        //connect points
             int Scale = 40;
             int pointScale = 4;
            myScene->addLine(Ax*Scale,Ay*Scale,Bx*Scale,By*Scale,QPen(Qt::gray));
            myScene->addLine(Ax*Scale,Ay*Scale,Cx*Scale,Cy*Scale,QPen(Qt::gray));
            myScene->addLine(Bx*Scale,By*Scale,Cx*Scale,Cy*Scale,QPen(Qt::gray));
        //draw points
            myScene->addEllipse(Ax*Scale-.5*pointScale,Ay*Scale-.5*pointScale,pointScale,pointScale,QPen(Qt::red),QBrush(Qt::red));       //a    RED
            myScene->addEllipse(Bx*Scale-.5*pointScale,By*Scale-.5*pointScale,pointScale,pointScale,QPen(Qt::yellow),QBrush(Qt::yellow)); //b    YELLOW
            myScene->addEllipse(Cx*Scale-.5*pointScale,Cy*Scale-.5*pointScale,pointScale,pointScale,QPen(Qt::blue),QBrush(Qt::blue));     //c    BLUE
            ////myScene->addEllipse(Dx*Scale-.5*pointScale,Dy*Scale-.5*pointScale,pointScale,pointScale,QPen(Qt::green),QBrush(Qt::green));   //d    GREEN
        //draw length
            QGraphicsTextItem *textAB = new QGraphicsTextItem(QString::number(AB));
                textAB->setPos(Ax*Scale + Scale,Ay*Scale-Scale);
            QGraphicsTextItem *textAC = new QGraphicsTextItem(QString::number(AC));
                textAC->setPos(Ax*Scale-Scale,Ay*Scale+Scale);
            QGraphicsTextItem *textBD = new QGraphicsTextItem(QString::number(BC));
                textBD->setPos(Bx*Scale+Scale,Ay*Scale+Scale);
            //QGraphicsTextItem *textCD = new QGraphicsTextItem(QString::number(CD));
                //textCD->setPos(Cx*Scale+Scale,Cy*Scale+Scale);

            myScene->addItem(textAB);
            myScene->addItem(textAC);
            myScene->addItem(textBD);
            //myScene->addItem(textCD);

       //draw objects
            for(int i = 0;i<cntr;i++)
            {
                if(obj[i]->isCords)
                {
                    myScene->addRect(obj[i]->x*Scale-.5*pointScale,obj[i]->y*Scale-.5*pointScale,pointScale,pointScale,QPen(Qt::cyan),QBrush(Qt::cyan)); ////TODO Randomize color
                    QGraphicsTextItem *textObj = new QGraphicsTextItem(obj[i]->MAC);
                        textObj->setPos(obj[i]->x*Scale -1.25*Scale  -.5*pointScale,obj[i]->y*Scale-.5*pointScale);
                    myScene->addItem(textObj);
                }

            }
       ////////MEMORY MANAGMENT???
       // for(int i = 0;i<cntr;i++)
       // {
       //   delete obj[cntr];
       // }

          q.prepare("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','macB',:ssi)"); //ab
            q.bindValue(":ssi",-71 + ((rand()%2) -1));
            q.exec();
          q.prepare("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','macC',:ssi)"); //ac
           q.bindValue(":ssi",-54 + ((rand()%2) -1));
           q.exec();
          q.prepare("INSERT INTO power(time,station,mac,rssi) VALUES(0,'A','macD',:ssi)"); //ad
            q.bindValue(":ssi",-77 + ((rand()%2) -1));
            q.exec();

         q.prepare("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','macC',:ssi)"); //bc
            q.bindValue(":ssi",-73 + ((rand()%2) -1));
            q.exec();
         q.prepare("INSERT INTO power(time,station,mac,rssi) VALUES(0,'B','macD',:ssi)"); //bd
            q.bindValue(":ssi",-59 + ((rand()%2) -1));
            q.exec();

         q.prepare("INSERT INTO power(time,station,mac,rssi) VALUES(0,'C','macD',:ssi)"); //cd
         q.bindValue(":ssi",-73 + ((rand()%2) -1));
         q.exec();

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
qreal MainWindow::getX(qreal ang,qreal hypot)
{
    qreal alpha;
    if(ang > M_PI_2)
        {alpha = ang - M_PI_2 +M_PI;} //makes x negative
    else
        {alpha = M_PI_2 - ang;} // x is positive

    return (qSin(alpha)*hypot);

}
qreal MainWindow::getY(qreal ang,qreal hypot)
{
    qreal alpha;
    if(ang > M_PI_2)
        {alpha = ang - M_PI_2;} //y is always negative
    else
        {alpha = M_PI_2 - ang;} // y is always negative

    return (qCos(alpha)*hypot);
}
qreal MainWindow::getObjX(qreal abs_ang,qreal lengA) //could be combined with node cords but it makes assumptions that this does not
{
    return qCos(abs_ang)*lengA;
}
qreal MainWindow::getObjY(qreal abs_ang,qreal lengA)
{
    return qSin(abs_ang)*lengA;
}


MainWindow::~MainWindow()
{
    delete ui;
}
