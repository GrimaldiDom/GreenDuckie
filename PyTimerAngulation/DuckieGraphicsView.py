from PyQt5.Qt import *
from DuckieTrackingScene import DuckieTrackingScene
class DuckieGraphicsView( QGraphicsView ):
    def __init__(self, *args):
        super().__init__( *args )
        self.resize(960, 540)
        self.trackingScene = DuckieTrackingScene( self )
        self.setScene( self.trackingScene )
        self.show()

    def setLog( self, log ):
        self.log = log
