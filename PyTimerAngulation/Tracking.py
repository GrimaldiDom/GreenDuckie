import math
from Database import Database
from Logs import Logs

class Tracker:
    log = Logs().getLog()

    def __init__(self):
        # Initializing MAC table
        self.MACS = {}
        # 10 Second timeout
        self.timeout = 10

        self.DB = Database()

        # Running actual program
        self.initMacs()
        self.initTracking()


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

                    stations[recv_Station][emit_Station] = { "power": pwr, "distance": self.PowerToDistance(pwr) }


            print( stations )
            # There should only ever be 4 elements with 0 as their power, and those should be the ones
            # trying to measure thier own power.
            # This if statement counts the number of 0s in the powers of the dictionary and breaks if the number of zeros is 4
            if( len( [ y for x in stations.keys() for y in stations[x].keys() if stations[x][y]["power"] == 0] ) == 3 ):
                break

        # Gets the keys in an array so we can manually pull out the 3 distances separately
        nodes = [ x for x in self.MACS.keys() ]

        # Get the distances between the 3 stations
        ## I feel like it would be possible to average the 2 measurements (AB & BA) together to get a better reading
        AB = stations[nodes[0]][nodes[1]]["distance"]
        AC = stations[nodes[0]][nodes[2]]["distance"]
        BC = stations[nodes[1]][nodes[2]]["distance"]

        # Contains array of 3 angles to create triangle
        angles = self.getTriangle( AB, AC, BC )

    @staticmethod
    def PowerToDistance( power ):
        return( (power+42.183)/(-5.6743) ) # Estimated linear function

    @staticmethod
    def getTriangle( a, b, c ):
        # Input 3 distances and this function returns 3 angles
        return( [
                    math.cos( (a*a+b*b-c*c)/(2*a*b) ),
                    math.cos( (a*a+c*c-b*b)/(2*a*c) ),
                    math.cos( (b*b+c*c-a*a)/(2*b*c) )
            ] )

    @staticmethod
    def getAngle( a, b, c ):
        # The order of a and b dont matter so long as both a and b are the sides adjacent to the angle you are trying to measure
        # c needs to be on the side furthest from the angle.
        return( math.cos( (a*a+b*b-c*c)/(2*a*b) ) )

