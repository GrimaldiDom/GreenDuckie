from PyQt5.Qt import *
from DuckieTrackingScene import DuckieTrackingScene
from Logs import Logs
class DuckieGraphicsView( QGraphicsView ):
    def __init__(self, *args):
        self.log = Logs().getLog()
        super().__init__( *args )
        self.trackingScene = DuckieTrackingScene( self, self.log )
        self.setScene( self.trackingScene )
        self.show()

    def update(self):
        self.trackingScene.update()
