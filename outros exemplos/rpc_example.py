#! /usr/bin/env python3
# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
#
# Copyright (c) 2022 Gabriel Ferreira <gabrielcarvfer@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


import asyncio
import socket
import multiprocessing as mp
import time

rpc_calls = {}
RPC_PORT = 4329
BUFFSIZE = 4000


def rpc_server_init(port):
    asyncio.run(rpc_server(port))


async def rpc_server(port: int):
    print("RPC SERVER")
    # Request a IPv4 (AF_INET) TCP (SOCK_STREAM) socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    print("RPC SERVER - BINDING")

    # Request the OS to bind a IP+port combination, makit it exclusive to the application
    sock.bind((socket.gethostname(), port))

    print("RPC SERVER - LISTENING")

    # Request the OS to listen to the indicated port for 10 connections
    sock.listen(10)
    clients = {}
    while True:
        # Accept the connection and gets a socket to communicate with them and their IP
        (clientsocket, address) = sock.accept()
        clients[address] = (clientsocket, address)

        print("RPC SERVER - ACCEPTED CONNECTION: ", address)

        for client in clients.values():
            print("RPC SERVER - Listening to client requests: ", client[1])

            # Receive RPC call request from client and split it into request + arguments
            rpc_req, rpc_param = client[0].recvmsg(BUFFSIZE)[0].decode("utf-8").split(':')

            print("RPC SERVER - Request: "+ rpc_req +"("+ rpc_param+")")

            # Execute the request
            rpc_response = await rpc_calls[rpc_req](rpc_param)

            print("RPC SERVER - Response: " + rpc_response)

            # Send the response
            client[0].sendmsg([rpc_response.encode("utf-8")])
    print("RPC SERVER DIED")
    pass

rpc_cli_sock = None


def rpc(func):
    global rpc_calls
    # Register function as a rpc call
    if func.__name__ not in rpc_calls:
        rpc_calls[func.__name__] = func

    def inner(filename):
        global rpc_cli_sock

        print("RPC CLIENT")
        # Create a socket to the RPC server
        if not rpc_cli_sock:
            print("RPC CLIENT - Registering to the server")
            rpc_cli_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            rpc_cli_sock.connect((socket.gethostname(), RPC_PORT))

        print("RPC CLIENT - Send RPC request")
        # Now that we are connected, send the name of the call and the argument to the rpc server
        msg = func.__name__ + ":" + filename
        print(msg)
        rpc_cli_sock.sendmsg([msg.encode("utf-8")])

        print("RPC CLIENT - Wait for RPC response")
        # Then wait to receive its response
        resp = rpc_cli_sock.recvmsg(BUFFSIZE)[0].decode("utf-8")
        print("RPC CLIENT - Received RPC response: ", resp)
        return resp

    return inner

# Registers read_file as a RPC call
# In RPC-land, this results in two stubs
# a client stub:
# - that marshals (encodes) the data,
# - transmits to the server as an RPC request
# - wait for the RPC response
# - forward the RPC response back to its caller
# a server stub:
# - that unmarshals (decodes) the data,
# - calls the remote call with the arguments passed
# - marshals the RPC response
# - sends the RPC response to the client stub
@rpc
async def read_file(filename):
    return "boop"


print(rpc_calls)  # read_file should be registered


async def waiting_routine(filename):
    return read_file(filename)


async def main():

    # Prepare RPC server on a separate process
    mp.Process(target=rpc_server_init, kwargs={"port": RPC_PORT}, daemon=True).start()

    # Wait a few seconds for the RPC server to be ready
    time.sleep(30)

    # What was supposed to be trivially done locally, is much more complicated when remote
    contents = await waiting_routine("large_file.txt")
    print("Contents of \"large_file_test.txt\" is: ", contents)



# Async is different from normal programs
# There is a runner that runs a main routine
# and worker threads that wait for requests
# without ever blocking
if __name__ == "__main__":
    asyncio.run(main())
