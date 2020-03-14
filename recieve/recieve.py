#!/usr/bin/python

import sys
sys.path.append("../PyTimerAngulation")

import sqlite3
import socket               # Import socket module
import _thread as thread

from Database import Database

MSG_SIZE=40
DB = Database("test.db")

def on_new_client(clientsocket,addr):
    while True:
        msg = clientsocket.recv(MSG_SIZE)

        if not msg:
            print( "{} has disconnected".format( addr ) )
            break

        #do some checks and if msg == someWeirdSignal: break:
        ascii_msg = msg.decode("ascii").strip("\x00")
        print( ascii_msg )
        s_ascii_msg = ascii_msg.split("/")

        station = s_ascii_msg[0]
        time = s_ascii_msg[1]
        mac = s_ascii_msg[2]
        power = s_ascii_msg[3]

        sql = "INSERT INTO power VALUES( NULL, ?, ?, ?, ? )"
        DB.sql_exec( sql, (float(time),station,mac,int(power) ) )

def main():
    sql_list = ["DROP TABLE IF EXISTS power;",
          "DROP TABLE IF EXISTS locations;",
          "CREATE TABLE power( id INTEGER PRIMARY KEY AUTOINCREMENT, time INT, station TEXT, mac TEXT, power INT );",
          "CREATE TABLE locations( id INT, mac TEXT, x INT, y INT );" ]

    [ DB.sql_exec( sql ) for sql in sql_list ]

    s = socket.socket()         # Create a socket object
    host = ""                   # Accept connection from any IP
    port = 55123                # Reserve a port for your service.

    print( 'Server started!')
    print( 'Waiting for clients...')

    s.bind((host, port))        # Bind to the port
    s.listen(5)                 # Now wait for client connection.

    while True:
       c, addr = s.accept()     # Establish connection with client.
       print( 'Got connection from', addr )
       thread.start_new_thread(on_new_client,(c,addr))
       #Note it's (addr,) not (addr) because second parameter is a tuple
       #Edit: (c,addr)
       #that's how you pass arguments to functions when creating new threads using thread module.
    s.close()

if __name__ == "__main__":
    main()
