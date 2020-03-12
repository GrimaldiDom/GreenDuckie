from PyQt5.Qt import *
class DuckieTrackingScene( QGraphicsScene ):
    def __init__( self, *args ):
        super().__init__( *args )
        self.addText( "TEST" )


    # def paintEvent(self, event):
        # painter = QPainter(self)
        # painter.setPen(QPen(Qt.green,  8, Qt.DashLine))
        # painter.drawEllipse(40, 40, 40, 40)
