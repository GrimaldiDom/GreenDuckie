from PyQt5.Qt import *
import time
from Logs import Logs
from Database import Database
from Tracker import Tracker

class DuckieTrackingScene( QGraphicsScene ):
    def __init__( self, *args ):
        self.log = Logs().getLog()
        super().__init__()
        self.point_scale = 20
        self.distance_scale = 40
        # self.label_y_offset = 10

        self.DB = Database()
        self.tracker = Tracker( self.DB )

        self.track_unknown = True

    def update( self ):
        self.tracker.getStationObjectsPositions()
        self.tracker.getTrackedObjectsPositions( self.track_unknown )

        self.clear()

        self.updateStations()
        self.updateTrackedObjects()

    def updateStations( self ):
        self.log.debug("[+] Started updateStationLayout...")
        positions = []
        for mac in self.tracker.STATION_MACS:
            response = self.DB.getNewestLocationByMac( mac )
            if( response == None ):
                self.log.debug("[x] Failed updateStationLayout...")
                return

            positions.append( [ response["x"], response["y"] ] )

        self.drawTriangle( positions )
        self.log.debug("[-] Completed updateStationLayout...")

    def updateTrackedObjects( self ):
        self.log.debug("[+] Starting updateTrackedObjects...")
        for mac in self.tracker.TRACKING_MACS:
            response = self.DB.getNewestLocationByMac( mac )
            if( response == None ):
                continue

            now = time.time()
            if( now - response["timestamp"] > 3 ): # If more than 3 seconds has passed since this measurement has been taken, meaning it hasn't been detected again in 3 seconds
                continue

            self.log.debug("[*] Updating tracked objects...")

            pos = [ response["x"], response["y"] ]
            self.drawLabel( pos, response["mac"] )
            self.drawNode( pos )

        self.log.debug("[-] Completed updatingTrackedObjects...")

    def drawTriangle( self, positions ):
        self.log.debug("[+] Starting drawTriangle...")
        self.log.debug("[?] Positions -- A: {0}, B: {1}, C: {2}".format( positions[0], positions[1], positions[2] ) )

        # We assume the A node is 0,0 for simplicity
        self.drawLabel( positions[0], "A" )
        self.drawNode( positions[0] )

        # We assume the B node is north of the A node so the detection can be self organizing
        self.drawLabel( positions[1], "B" )
        self.drawNode( positions[1] )

        # Draw third node
        self.drawLabel( positions[2], "C" )
        self.drawNode( positions[2] )

        # Draw lines for all nodes
        [ self.drawLine( positions[x-1], positions[x] ) for x in range(len(positions)) ]
        self.log.debug("[-] Completed drawTriangle...")

    def drawNode( self, xy ):
        # xy = [x, y]
        pen = QPen(Qt.black, 2)
        brush = QBrush( Qt.black )
        radius = 0.25*self.point_scale

        xy = [ x*self.distance_scale for x in xy ]

        # x1, y1, x2, y2, pen, brush
        self.addEllipse( xy[0]-radius/2, xy[1]-radius/2, radius, radius, pen, brush )

    def drawLine( self, node1, node2 ):
        # node1 = [x, y], node2 = [x,y]
        pen = QPen(QPen(Qt.black, 2, Qt.DashLine))

        # x1, y1, x2, y2, pen
        self.addLine( node1[0]*self.distance_scale, node1[1]*self.distance_scale, node2[0]*self.distance_scale, node2[1]*self.distance_scale, pen )

    def drawLabel( self, pos, text ):
        pen = QPen(QPen(Qt.black, 1, Qt.SolidLine))

        t = self.addText( text )
        t.setPos( pos[0]*self.distance_scale, pos[1]*self.distance_scale )
