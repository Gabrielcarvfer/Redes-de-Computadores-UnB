# Testing whether broadcasted packets arrive in bound sockets to a specific IP instead of any address/0.0.0.0
# Works just fine on Windows, the ignores it apparently, but fails on linux
# 
# Related zulip discussion https://ns-3.zulipchat.com/#narrow/channel/103658-general/topic/Socket.20questions
#
# Expected output (on Windows)
#
#   Localhost hostname: Minisforum
#   Available addresses: ['172.29.224.1', '192.168.172.245', '10.10.10.131', '169.254.83.107']
#   Picking whatever starts with 192. : 192.168.172.245
#   Local broadcast address: 192.168.172.255
#   Client sending 'b'banana'' to ('192.168.172.255', 3213)
#   Server received b'banana' from ('192.168.172.245', 62815)
#   Server sent back b'banana' to ('192.168.172.245', 62815)
#   Client received 'b'banana'' from ('192.168.172.245', 3213)
#   Client sending 'b'banana'' to ('192.168.172.255', 3213)
#   Server received b'banana' from ('192.168.172.245', 62816)
#   Server sent back b'banana' to ('192.168.172.245', 62816)
#   Client received 'b'banana'' from ('192.168.172.245', 3213)
#   Client sending 'b"That's all folks"' to ('192.168.172.245', 3213)
#   Server received b"That's all folks" from ('192.168.172.245', 62817)
#
#   Process finished with exit code 0

import socket
import time
from concurrent.futures import ThreadPoolExecutor


class PingServer:
    def __init__(self, bound_address="0.0.0.0", port=3213):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind((bound_address, port))

        while 1:
            data, source = self.socket.recvfrom(1024)
            print(f"Server received {data} from {source}")
            if data == b"That's all folks":
                break
            self.socket.sendto(data, source)
            print(f"Server sent back {data} to {source}")
        self.socket.close()

def PingClient(server_address="127.0.0.1", server_port=3213, the_message=b"banana"):
    while True:
        socket_cliente_thread = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        address = (server_address, server_port)
        print(f"Client sending '{the_message}' to {address}")
        socket_cliente_thread.sendto(the_message, address)
        if the_message == b"That's all folks":
            break
        msg, address = socket_cliente_thread.recvfrom(512)
        print(f"Client received '{msg}' from {address}")
        break
    socket_cliente_thread.close()

threadPool = ThreadPoolExecutor()

localhost_name = socket.gethostname()
available_addresses = socket.gethostbyname_ex(localhost_name)[2]
local_network_address = list(filter(lambda x: "192.168." in x, available_addresses))[0]
local_network_broadcast_address = local_network_address.split(".")
local_network_broadcast_address[-1] = "255"
local_network_broadcast_address = ".".join(local_network_broadcast_address)

print(f"Localhost hostname: {localhost_name}")
print(f"Available addresses: {available_addresses}")
print(f"Picking whatever starts with 192. : {local_network_address}")
print(f"Local broadcast address: {local_network_broadcast_address}")
server_port = 3213

threadPool.submit(PingServer, local_network_address, server_port)

time.sleep(2)
for i in range(2):
    PingClient(local_network_broadcast_address, server_port)
PingClient(local_network_address, server_port, b"That's all folks")
