import math
from Database import Database
from Logs import Logs

class Tracker:

    def __init__(self):
        self.log = Logs().getLog()
        # Initializing MAC table
        self.STATION_MACS = {}
        self.TRACKING_MACS = []
        # 10 Second timeout
        self.timeout = 10

        # Dictionary of things to be tracked with MACs as key and coordinantes as values
        self.tracked_objects = {}

        self.DB = Database()

        # Running actual program
        self.initMacs()


    def initMacs( self ):
        ##########################################################
        # This function gets the necessary MAC addresses for the #
        # required objects in the demonstration                  #
        ##########################################################
        current_dict = {}
        stations = 0
        tracking = 0
        with open('MACS.ini', 'r') as f:
            for line in f:
                if( line[0] == "\n" or line[0] == "\n" ):
                    continue

                if( line[0] == "[" ):
                    if( "Stations" in line ):
                        stations = 1
                        tracking = 0
                        if( current_dict ): # Checks if not empty
                            self.TRACKING_MACS = current_dict

                    elif( "Tracking" in line ):
                        stations = 0
                        tracking = 1
                        if( current_dict ): # Checks if not empty
                            self.stations = current_dict

                    else:
                        self.log.error("Unknow ini header: ({})".format(line))
                        raise

                    continue

                if( stations == 1 ):
                    obj, MAC = [ x.strip("\n") for x in line.split("=") ] # Splits line and puts them in correct variable while stripping newline
                    self.STATION_MACS[ obj ] = MAC

                elif( tracking == 1 ):
                    self.TRACKING_MACS.append( line.strip("\n") )

        self.log.debug( "Station Macs Found: {}".format( self.STATION_MACS ) )
        self.log.debug( "Tracking Macs Found: {}".format( self.TRACKING_MACS ) )

    def getStationLayout( self ):
        #############################################################################################
        # This function return a list of 2 lists consisting of distances and angles respectively.   #
        # These are determined from the database and MACs of the stations1                          #
        #############################################################################################
        stations = {}

        while( True ):
            for recv_Station in self.STATION_MACS.keys():        # Station that detects sample packet
                for emit_Station in self.STATION_MACS.keys():    # Station that emits sample packet
                    # One of these on each run should not find anything since it's trying to find its own blutooth packets
                    entry = self.DB.getNewestEntryByMac( recv_Station, self.STATION_MACS[ emit_Station ] )
                    if( entry == None ):
                        pwr = 0
                    else:
                        pwr = entry["power"]

                    if( recv_Station not in stations.keys() ):
                        stations[recv_Station] = {}

                    if( emit_Station not in stations[recv_Station].keys() ):
                        stations[recv_Station][emit_Station] = {}

                    stations[recv_Station][emit_Station] = { "power": pwr, "distance": self.PowerToDistance(pwr) }


            # There should only ever be 4 elements with 0 as their power, and those should be the ones
            # trying to measure thier own power.
            # This if statement counts the number of 0s in the powers of the dictionary and breaks if the number of zeros is 4
            if( len( [ y for x in stations.keys() for y in stations[x].keys() if stations[x][y]["power"] == 0] ) == 3 ):
                self.log.debug( stations )
                break

        # Gets the keys in an array so we can manually pull out the 3 distances separately
        nodes = [ x for x in self.STATION_MACS.keys() ]

        # Get the distances between the 3 stations
        ## I feel like it would be possible to average the 2 measurements (AB & BA) together to get a better reading
        distances = [
            stations[nodes[0]][nodes[1]]["distance"],
            stations[nodes[0]][nodes[2]]["distance"],
            stations[nodes[1]][nodes[2]]["distance"],
        ]

        # Contains array of 3 angles to create triangle
        angles = self.getTriangle( distances )

        self.node_triangle = [ distances, angles ]
        return( self.node_triangle )

    def getTrackedObjectsLayout( self ):
        l_tracked_objects = {}

        for tracking_mac in self.TRACKING_MACS:          # MAC we are tracking
            for recv_Station in self.STATION_MACS.keys():        # Station that detects sample packet
                # One of these on each run should not find anything since it's trying to find its own blutooth packets
                entry = self.DB.getNewestEntryByMac( recv_Station, tracking_mac )
                if( entry == None ):
                    pwr = None
                else:
                    pwr = entry["power"]

                if( tracking_mac not in l_tracked_objects.keys() ):
                    l_tracked_objects[tracking_mac] = {}

                if( recv_Station not in l_tracked_objects[tracking_mac].keys() ):
                    l_tracked_objects[tracking_mac][recv_Station] = {}


                l_tracked_objects[tracking_mac][recv_Station] = { "power": pwr, "distance": PowerToDistance(pwr) }

        macs_to_remove = []
        for tracking_mac in l_tracked_objects.keys():
            for recv_Station in l_tracked_objects[tracking_mac].keys():
                # if any of the recieving stations don't have a power for an object to track, it can't be tracked
                if l_tracked_objects[ tracking_mac ][recv_Station]["power"] == None:
                    macs_to_remove.append( tracking_mac )
                    macs_to_remove = list( set( macs_to_remove ) )

        for mac in macs_to_remove:
            del l_tracked_objects[mac]

        for tracking_mac in l_tracked_objects.keys():
            # Gets the keys in an array so we can manually pull out the 3 distances separately
            nodes = [ x for x in self.STATION_MACS.keys() ]

            position = self.getTrackedLocation( l_tracked_objects[tracking_mac], self.node_triangle )

            self.tracked_objects[tracking_mac] = position

        return( self.tracked_objects )

    def getTrackedLocation( self, object_pwrs ):
        for recv_Station in object_pwrs.keys():        # Station that detects sample packet
            pass

    @staticmethod
    def PowerToDistance( power ):
        if( power == None ):
            return None
        return( (power)/(40) ) # Estimated linear function in feet

    @staticmethod
    def getTriangle( distances ):
        # Input 3 distances in an array and this function returns 3 angles
        a = distances[0]
        b = distances[1]
        c = distances[2]
        return( [
                    math.cos( (a*a+b*b-c*c)/(2*a*b) ),
                    math.cos( (a*a+c*c-b*b)/(2*a*c) ),
                    math.cos( (b*b+c*c-a*a)/(2*b*c) ),
            ] )

    @staticmethod
    def getAngle( a, b, c ):
        # The order of a and b dont matter so long as both a and b are the sides adjacent to the angle you are trying to measure
        # c needs to be on the side furthest from the angle.
        return( math.cos( (a*a+b*b-c*c)/(2*a*b) ) )

    def getNodeTriangle( self ):
        return( self.node_triangle )

