#!/bin/python3
# -*- coding: utf-8 -*-

from PyQt5.Qt import *
import sys, time
from datetime import datetime
from DuckieGraphicsView import DuckieGraphicsView
from Logs import Logs

class Ui_MainWindow( QMainWindow  ):
    # Initializes logging, useful for not crowding the terminal with output
    log = Logs().getLog()

    def __init__( self, *args ):
        QMainWindow.__init__(self, *args)
        self.setupUi()

    def setupUi(self):

        self.setObjectName("MainWindow")
        self.resize(960, 540)
        self.centralwidget = QWidget(self)
        self.centralwidget.setObjectName("centralwidget")
        self.horizontalLayout = QHBoxLayout(self.centralwidget)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.verticalLayout = QVBoxLayout()
        self.verticalLayout.setObjectName("verticalLayout")
        self.graphicsView = DuckieGraphicsView(self.centralwidget)
        self.graphicsView.setObjectName("graphicsView")
        self.verticalLayout.addWidget(self.graphicsView)
        self.horizontalLayout.addLayout(self.verticalLayout)
        self.setCentralWidget(self.centralwidget)
        self.menubar = QMenuBar(self)
        self.menubar.setGeometry(QRect(0, 0, 943, 24))
        self.menubar.setObjectName("menubar")
        self.setMenuBar(self.menubar)
        self.statusbar = QStatusBar(self)
        self.statusbar.setObjectName("statusbar")
        self.setStatusBar(self.statusbar)

        QMetaObject.connectSlotsByName(self)

        self.timer = QTimer()
        self.timer.timeout.connect( self.graphicsView.update )
        self.timer.start(10)


    def Run( self ):
        self.graphicsView.update()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    myapp = Ui_MainWindow()
    myapp.show()
    myapp.Run()
    sys.exit(app.exec_())
