import sqlite3, os
from datetime import datetime
from sqlite3 import Error
from Logs import Logs
class Database:
    def __init__( self, database_filename=None ):
        self.log = Logs().getLog()

        if( database_filename != None ):
            self.database_filename = database_filename
        else:
            self.database_filename = "demo.db"

        init_database = True if( not os.path.exists( self.database_filename ) or not os.path.getsize( self.database_filename) > 0  ) else False
        # path can equal either :memory: or a database file
        try:
            self.conn = sqlite3.connect( self.database_filename )
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

    def getNewestEntryByMac( self, station, MAC ):
        ############################################################
        # Returns sql information for given MAC in dictionary form #
        ############################################################
        sql = "SELECT * FROM power WHERE mac=? AND station=?"
        response = self.sql_exec( sql, (MAC,station) ).fetchone()
        if( response == None ):
            return None
        final = { "id":response["id"], "station":response["station"], "mac":response["mac"], "power":response["power"] }
        final["time"] = datetime.fromtimestamp( float( response["time"] ) )
        return( final )
