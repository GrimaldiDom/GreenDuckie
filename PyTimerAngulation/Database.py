import sqlite3, os, time
from datetime import datetime
from sqlite3 import Error
from Logs import Logs
class Database:
    def __init__( self, database_filename=None ):
        self.log = Logs().getLog()

        if( database_filename != None ):
            self.database_filename = database_filename
        else:
            self.database_filename = "test.db"

        init_database = True if( not os.path.exists( self.database_filename ) or not os.path.getsize( self.database_filename) > 0  ) else False
        # path can equal either :memory: or a database file
        try:
            self.conn = sqlite3.connect( self.database_filename, isolation_level=None, check_same_thread=False )
            self.conn.row_factory = sqlite3.Row
        except Error as e:
            print( e )
            self.conn.close()
            exception_string = "Connection to database failed. Closing..."
            # self.log.exception( exception_string )
            raise Exception(exception_string)

    def commitChanges( self ):
        self.conn.commit()

    def sql_exec( self, sql, values=None ):
        if( self.conn != None ):
            c = self.conn.cursor()
        try:
            if( values != None ):
                c.execute( sql, values )
            else:
                c.execute( sql )

            return( c )

        except Error as e:
            print(e)
            print( "Executing sql command was unsuccessful..." )

    def getNewestPowerByMac( self, station, MAC ):
        ############################################################
        # Returns sql information for given MAC in dictionary form #
        ############################################################
        sql = "SELECT * FROM power WHERE mac=? AND station=? ORDER BY id DESC LIMIT 1"
        self.log.debug("[+] Started getNewestPowerByMac...")
        # self.log.debug("[?] Getting sql entry for mac='{}' and station='{}'...".format(MAC, station))

        MAX_RESPONSES = 1000
        response = self.sql_exec( sql, (MAC,station) ).fetchmany( MAX_RESPONSES )

        if( response == None or len(response) == 0 ):
            self.log.debug("[x] No sql entry found for mac='{}' from station='{}'...".format(MAC, station) )
            return None

        avg_power = 0
        for r in range(len(response)):
            avg_power += response[r]["power"]
        avg_power = avg_power/len(response)

        final_response = response[0]
        final = { "id":final_response["id"], "station":final_response["station"], "time":final_response["time"], "mac":final_response["mac"], "power":avg_power }

        # self.log.debug("Returning non-None value for for mac='{}' and station='{}' sql request...".format(MAC, station))
        # final["time"] = datetime.fromtimestamp( float( response["time"] ) )

        self.log.debug("[-] Completed getNewestPowerByMac...")
        return( final )

    def insertIntoLocations( self, MAC, x, y ):
        sql = "INSERT INTO location( mac, timestamp, x, y ) VALUES ( ?, ?, ?, ? )"
        self.sql_exec( sql, (MAC, time.time(), x, y) )

    def getNewestLocationByMac( self, MAC ):
        ############################################################
        # Returns sql information for given MAC in dictionary form #
        ############################################################
        self.log.debug("[+] Started getNewestLocationByMac...")
        sql = "SELECT * FROM location WHERE mac=? ORDER BY id DESC LIMIT 1"
        # self.log.debug("Getting sql entry for mac='{}'...".format(MAC))
        response = self.sql_exec( sql, (MAC,) )
        if( response == None ):
            self.log.debug("[x] No sql entry found for mac='{}'...".format(MAC, station) )
            return None

        self.log.debug("[-] Completed getNewestLocationByMac...")
        return( response.fetchone() )

    def getNewUnknownMacs( self, known_macs, last_id ):
        self.log.debug("[+] Started getNewUnknownMacs...")

        sql = "SELECT * FROM power WHERE id > {} ".format( last_id )

        response = self.sql_exec( sql ).fetchall()

        if( response == None or len(response) == 0 ):
            self.log.debug("[x] No new tracked object entries found..." )
            return( [], last_id )

        new_last_id = response[-1]["id"]

        new_macs = []
        for item in response:
            if( item["mac"] not in known_macs + new_macs ):
                new_macs.append( item["mac"] )

        self.log.debug("[?] Found {} new macs...".format( len(new_macs) ) )
        self.log.debug("[?] New last id={}".format(new_last_id) )

        self.log.debug("[+] Completed getNewUnknownMacs...")
        return( new_macs, new_last_id )

