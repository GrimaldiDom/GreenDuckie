from PyQt5.Qt import *
from Logs import Logs
from Tracker import Tracker

class DuckieTrackingScene( QGraphicsScene ):
    def __init__( self, *args ):
        self.log = Logs().getLog()
        super().__init__()
        self.scale = 30

        tracker = Tracker()
        station_layout = tracker.getStationLayout()
        self.drawTriangle( station_layout )

        tracker.getTrackedObjectsLayout()
        tracked_objects_layout = tracker.getTrackedObjectsLayout()
        self.drawTrackedObjects( tracked_objects_layout )


    def drawTriangle( self, triangle ):
        # distance = [ AB, AC, BC ]
        distances = triangle[0]
        # distance = [ A, B, C ]
        angles = triangle[1]

        nodes = [ [0, 0], [0, distances[1]], [ distances[1], distances[2] ] ]
        # We assume the A node is 0,0 for simplicity
        self.drawNode( nodes[0] )

        # We assume the B node is north of the A node so the detection can be self organizing
        self.drawNode( nodes[1] )

        # Draw third node
        self.drawNode( nodes[2] )

        # Draw lines for all nodes
        [ self.drawLine( nodes[x-1], nodes[x] ) for x in range(len(nodes)) ]

    def drawNode( self, xy ):
        # xy = [x, y]
        pen = QPen(Qt.black, 2)
        brush = QBrush( Qt.black )
        scale = self.scale
        radius = 0.25*scale

        xy = [ x*scale for x in xy ]

        # x1, y1, x2, y2, pen, brush
        self.addEllipse( xy[0]-radius/2, xy[1]-radius/2, radius, radius, pen, brush )

    def drawLine( self, node1, node2 ):
        # node1 = [x, y], node2 = [x,y]
        pen = QPen(QPen(Qt.black, 2, Qt.DashLine))

        # x1, y1, x2, y2, pen
        self.addLine( node1[0]*self.scale, node1[1]*self.scale, node2[0]*self.scale, node2[1]*self.scale, pen )

    def drawTrackedObjects( self, tracked_objects ):
        pass
