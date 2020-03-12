#!/bin/python3
# -*- coding: utf-8 -*-

from PyQt5.Qt import *
import sys, logging
from datetime import datetime
from DuckieGraphicsView import DuckieGraphicsView
from Database import Database

class Ui_MainWindow( QMainWindow  ):
    # Initializes logging, useful for not crowding the terminal with output
    logging.basicConfig(filename='GreenDuckie.log', filemode='w', format='%(asctime)s - %(levelname)s - %(message)s', level=logging.DEBUG )
    log = logging.getLogger(__name__)

    def __init__( self, *args ):
        QMainWindow.__init__(self, *args)
        self.setupUi()

        # Initializing MAC table
        self.MACS = {}
        # 10 Second timeout
        self.timeout = 10

        self.DB = Database()

        # Running actual program
        self.initMacs()
        self.initTracking()

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
        self.graphicsView.setLog( self.log )
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

    def initMacs( self ):
        ##########################################################
        # This function gets the necessary MAC addresses for the #
        # required objects in the demonstration                  #
        ##########################################################
        with open('MACS.ini', 'r') as f:
            for line in f:
                obj, MAC = [ x.strip("\n") for x in line.split("=") ] # Splits line and puts them in correct variable while stripping newline
                self.MACS[ obj ] = MAC

        self.log.debug( "Macs Found: {}".format( self.MACS ) )

    def initTracking( self ):
        ########################################################
        # This function initializes the base station locations #
        # and draws them on the screen                         #
        ########################################################
        stations = {}

        # Test Values
        """
        "01:59:9B:E2:85:E7"
        "22:02:CA:3D:E3:71"
        "7F:06:41:BB:45:5C"
        """
        while( True ):
            for recv_Station in self.MACS.keys():        # Station that detects sample packet
                for emit_Station in self.MACS.keys():    # Station that emits sample packet
                    # One of these on each run should not find anything since it's trying to find its own blutooth packets
                    entry = self.DB.getNewestEntryByMac( recv_Station, self.MACS[ emit_Station ] )
                    if( entry == None ):
                        pwr = 0
                    else:
                        pwr = entry["power"]

                    if( recv_Station not in stations.keys() ):
                        stations[recv_Station] = {}

                    if( emit_Station not in stations[recv_Station].keys() ):
                        stations[recv_Station][emit_Station] = {}

                    stations[recv_Station][emit_Station] = { "power": pwr }


            # There should only ever be 4 elements with 0 as their power, and those should be the ones
            # trying to measure thier own power.
            # This if statement counts the number of 0s in the powers of the dictionary and breaks if the number of zeros is 4
            if( len( [ y for x in stations.keys() for y in stations[x].keys() if stations[x][y]["power"] != 0] ) == 4 ):
                print("Yes")
                break

    def PowerToDistance( self, power ):
        return( (power+42.183)/(-5.6743) ) # Estimated linear function


if __name__ == "__main__":
    app = QApplication(sys.argv)
    myapp = Ui_MainWindow()
    myapp.show()
    sys.exit(app.exec_())
