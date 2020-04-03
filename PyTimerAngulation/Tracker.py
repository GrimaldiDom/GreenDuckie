import math
import time
from Database import Database
from Logs import Logs

class Tracker:

    def __init__(self, DB):
        self.log = Logs().getLog()
        # Initializing MAC table
        self.STATION_MACS = {}
        self.TRACKING_MACS = []
        # 10 Second timeout
        self.timeout = 10

        # Dictionary of things to be tracked with MACs as key and coordinantes as values
        self.tracked_objects = {}

        self.DB = DB

        # Running actual program
        self.initMacs()

        self.last_id = 0


    def initMacs( self ):
        ##########################################################
        # This function gets the necessary MAC addresses for the #
        # required objects in the demonstration                  #
        ##########################################################
        self.log.debug("[+] Started initMACs...")

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
                        self.log.error("[x] Unknown ini header: ({})".format(line))
                        raise

                    continue

                if( stations == 1 ):
                    obj, MAC = [ x.strip("\n") for x in line.split("=") ] # Splits line and puts them in correct variable while stripping newline
                    self.STATION_MACS[ obj ] = MAC

                elif( tracking == 1 ):
                    self.TRACKING_MACS.append( line.strip("\n") )

        self.log.debug( "[?] Station Macs Found: {}".format( self.STATION_MACS ) )
        self.log.debug( "[?] Tracking Macs Found: {}".format( self.TRACKING_MACS ) )

        self.log.debug("[-] Completed initMACs...")

    def getStationObjectsPositions( self ):
        #############################################################################################
        # This function inserts the positional data of each station into the locations table in the #
        # shared SQL database. Also ensures that all stations have been found due to while loop not #
        # exiting until all are found.                                                              #
        #############################################################################################
        self.log.debug("[+] Started getStationLayout...")
        stations = {}

        while( True ):
            for recv_Station in self.STATION_MACS.keys():        # Station that detects sample packet
                for emit_Station in self.STATION_MACS.keys():    # Station that emits sample packet
                    if( recv_Station == emit_Station ):
                        continue

                    # One of these on each run should not find anything since it's trying to find its own blutooth packets
                    entry = self.DB.getNewestPowerByMac( recv_Station, self.STATION_MACS[ emit_Station ] )
                    if( entry == None ):
                        break

                    if( recv_Station not in stations.keys() ):
                        stations[recv_Station] = {}

                    if( emit_Station not in stations[recv_Station].keys() ):
                        stations[recv_Station][emit_Station] = {}

                    stations[recv_Station][emit_Station] = { "power": entry["power"] }

            done = True
            for recv_Station in self.STATION_MACS.keys():        # Station that detects sample packet

                if recv_Station not in stations.keys():
                    done = False
                    break

                for emit_Station in self.STATION_MACS.keys():    # Station that emits sample packet
                    if( recv_Station == emit_Station ):
                        continue

                    if emit_Station not in stations[recv_Station].keys():
                        done = False
                        break

            if( done == True ):
                break


        # Gets the keys in an array so we can manually pull out the 3 distances separately
        nodes = [ x for x in self.STATION_MACS.keys() ]
        self.log.debug( "[?] Nodes: {}".format( nodes ) )

        # Get the distances between the 3 stations
        ## I feel like it would be possible to average the 2 measurements (AB & BA) together to get a better reading
        distances = [
            self.PowerToDistance( stations[nodes[0]][nodes[1]]["power"] ), # A->B
            self.PowerToDistance( stations[nodes[0]][nodes[2]]["power"] ), # A->C
            self.PowerToDistance( stations[nodes[1]][nodes[2]]["power"] ), # B->C
        ]

        self.log.debug( "[?] Distances found: {}".format( distances ) )

        for i, station in enumerate( stations.keys() ):
            if( i == 0 ):
                pos = [ 0,0 ]
            elif( i == 1 ):
                pos = [ 0, distances[1] ]
            elif( i == 2 ):
                angles = self.getTriangle( distances )
                # Distances[1] is AC because C is third point
                # Angle[0] is angle A
                pos = self.getThirdPointCoordinate( distances[2], angles[0] )

            self.DB.insertIntoLocations( station, *pos  )

        self.log.debug("[-] Completed getStationLayout...")
        return

    def getTrackedObjectsPositions( self, track_unknown=False ):
        self.log.debug("[+] Started getTrackedObjectsPositions...")
        if( track_unknown ):
            new_macs, self.last_id = self.DB.getNewUnknownMacs( self.TRACKING_MACS + list( self.STATION_MACS.values() ), self.last_id )
            self.TRACKING_MACS += new_macs

        for tracking_mac in self.TRACKING_MACS:          # MAC we are tracking
            args = self.getTrilaterationArguments( tracking_mac )
            if( args == None ):
                continue

            tracked_position = self.Trilaterate( *args )
            self.DB.insertIntoLocations( tracking_mac, *tracked_position )

        self.log.debug("[-] Completed getTrackedObjectsPositions...")

    def getTrilaterationArguments( self, tracking_mac ):
        args = [] # Arguments to be passed into the trilateration method
        for recv_Station in self.STATION_MACS.keys():        # Station that detects sample packet
            # One of these on each run should not find anything since it's trying to find its own blutooth packets
            entry = self.DB.getNewestPowerByMac( recv_Station, tracking_mac )
            if( entry == None ):
                return( None )

            now = time.time()
            if( now - entry["time"] > 3 ): # If more than 3 seconds has passed since this measurement has been taken, meaning it hasn't been detected again in 3 seconds
                return( None )

            response = self.DB.getNewestLocationByMac( recv_Station )
            args.append( response["x"] )
            args.append( response["y"] )
            args.append( self.PowerToDistance( entry["power"] ) )

        return( args )

    @staticmethod
    def PowerToDistance( power ):
        if( power == None ):
            return None
        if power == 0:
            power = 1

        return( 2**( (255-power)/32 ) )

    @staticmethod
    def getTriangle( distances ):
        # Input 3 distances in an array and this function returns 3 angles
        a = distances[0]
        b = distances[1]
        c = distances[2]

        # Acos has bounds -1 to 1 so i mod it to keep it in bounds
        innerA = ( ( (b*b+c*c-a*a)/(2*b*c) + 1 ) % 2 ) - 1
        innerB = ( ( (a*a+c*c-b*b)/(2*a*c)/(2*b*c) + 1 ) % 2 ) - 1
        innerC = ( ( (a*a+b*b-c*c)/(2*a*b)/(2*b*c) + 1 ) % 2 ) - 1
        return( [
                    math.acos( innerA ), # Angle A
                    math.acos( innerB ), # Angle B
                    math.acos( innerC ), # Angle C
            ] )

    @staticmethod
    def getThirdPointCoordinate( AC, A ):
        # # Thank you sir
        # # https://math.stackexchange.com/questions/543961/determine-third-point-of-triangle-when-two-points-and-all-sides-are-known
        # coord = [0,0]

        # if( AB > 0 ):
            # coord[0] = (BC*BC - AC*AC + AB*AB)/(2*AB)

        # coord[1] = math.sqrt( abs( BC*BC - coord[0]*coord[0]  ) )

        # return( coord )

        # Trying this from user Shivam Agrawal
        # https://math.stackexchange.com/questions/143932/calculate-point-given-x-y-angle-and-distance
        # 0 or x1 but since calculating from A station at 0,0 can leave as 0
        x3 = 0 + ( AC * math.cos( A ) )
        y3 = 0 + ( AC * math.sin( A ) )
        return( [ x3, y3 ] )

    @staticmethod
    def Trilaterate( x1, y1, r1, x2, y2, r2, x3, y3, r3 ):
        A = 2*x2 - 2*x1
        B = 2*y2 - 2*y1
        C = r1**2 - r2**2 - x1**2 + x2**2 - y1**2 + y2**2
        D = 2*x3 - 2*x2
        E = 2*y3 - 2*y2
        F = r2**2 - r3**2 - x2**2 + x3**2 - y2**2 + y3**2
        x = (C*E - F*B) / (E*A - B*D)
        y = (C*D - A*F) / (B*D - A*E)
        return [x,y]

    @staticmethod
    def getAngle( a, b, c ):
        # The order of a and b dont matter so long as both a and b are the sides adjacent to the angle you are trying to measure
        # c needs to be on the side furthest from the angle.
        return( math.cos( (a*a+b*b-c*c)/(2*a*b) ) )

    def getNodeTriangle( self ):
        return( self.node_triangle )

