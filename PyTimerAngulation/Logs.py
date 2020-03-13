import logging

class Logs():
    def __init__(self):
        logging.basicConfig(filename='GreenDuckie.log', filemode='w', format='%(asctime)s - %(levelname)s - %(message)s', level=logging.DEBUG )
        self.log = logging.getLogger(__name__)

    def getLog( self ):
        return( self.log )
