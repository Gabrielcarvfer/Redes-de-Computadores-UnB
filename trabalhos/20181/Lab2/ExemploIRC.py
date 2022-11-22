#! /usr/bin/env python3
# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
#
# Copyright (c) 2018-2022 Gabriel Ferreira <gabrielcarvfer@gmail.com>
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

from CriaProcessos import main
import socket
import time

portaHost = 65000  # porta do servidor (fica escutando esperando conexões)
mensagem = "abobrinha"  # mensagem a ser enviada


def processo1(id):
    # requisita API do SO uma conexão AF_INET (IPV4)
    #   com protocolo de transporte SOCK_STREAM (TCP)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # processo dorme por 10 segundos para evitar
    #   se conectar com servidor antes desse estar rodando
    time.sleep(10)

    # requisita estabelecimento de conexão
    sock.connect((socket.gethostname(), portaHost))

    while 1:
        # transforma mensagem em bytes e transmite
        sock.send(mensagem.encode(encoding="utf-8"))
        print("Cliente:", mensagem)
        time.sleep(5)

    return


class ServerClient:
    numClients = 0

    def __init__(self, ipv4, sock, nickname="USR" + str(numClients), hostname="", channel=""):
        self.ipv4 = ipv4
        self.sock = sock
        self.nickname = nickname
        self.hostname = hostname
        self.channel = channel

        ServerClient.numClients += 1

    def sendMsg(self, msg):
        self.sock.send(msg.encode("utf-8"))


class ServerChannel:
    def __init__(self, name):
        self.name = name
        self.clients = {}


class ServerApp:

    def __init__(self, portaHost):
        # Cria estruturas para segurar clients e canais
        self.clients = {}
        self.canais = {"": ServerChannel("")}

        # registra handlers para comandos
        self.handlers = {"NICK": self.nickClientHandler,
                         "USUARIO": self.newClientHandler,
                         "SAIR": self.deleteClientHandler,
                         "ENTRAR": self.subscribeChannelHandler,
                         "SAIRC": self.unsubscribeChannelHandler,
                         "LISTAR": self.listChannelHandler,
                         }
        # requisita API do SO uma conexão AF_INET (IPV4)
        #   com protocolo de transporte SOCK_STREAM (TCP)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # requisita ao SO a posse de uma porta associada a um IP
        self.sock.bind((socket.gethostname(), portaHost))

        # requisita ao SO que a porta indicada de um dado IP seja
        #   reservado para escuta
        self.sock.listen(5)

        self.run()

    def run(self):
        while 1:
            # aceita requisição de conexão do processo 1,
            #   e recebe um socket para o cliente e o
            #   endereço de IP dele
            (clientsock, address) = self.sock.accept()

            while 1:
                # recebe do socket do cliente (processo 1) uma mensagem de 512 bytes
                mensagem_recebida = clientsock.recv(512).decode("utf-8")

                print(mensagem_recebida)

                # processa mensagem
                answer = self.parseCommands(clientsock, address, mensagem_recebida)
                if len(answer) > 0:
                    self.sendMessage(answer)
        pass

    def parseCommands(self, clientsock, clientAddr, mensagem_recebida):
        commands = mensagem_recebida.split('\n')  # comandos separados por nova linha
        unrecognized_commands = []
        invalid_parameters = []

        if clientAddr not in self.clients.keys():
            self.clients[clientAddr] = ServerClient(clientAddr, clientsock)
            self.canais[""].clients[clientAddr] = self.clients[clientAddr]

        client = self.clients[clientAddr]

        for command in commands:
            comm_n_args = command.split(' ')
            if comm_n_args[0][0] == '?':
                if comm_n_args[0][1:] in self.handlers.keys():
                    ans = self.handlers[comm_n_args[0][1:]](clientAddr, comm_n_args[1:])
                    if len(ans) > 0:
                        invalid_parameters += ans
                else:
                    unrecognized_commands += comm_n_args[0]
            else:
                self.sendMsgChannel(command, client.channel)

        answer = ""
        if len(unrecognized_commands) > 0:
            answer += "Unrecognized commands: %s" % unrecognized_commands
        if len(invalid_parameters) > 0:
            answer += "Invalid parameters: %s\n" % invalid_parameters

        return answer

    def sendMsgChannel(self, msg, channel):
        for client in self.canais[channel].clients:
            self.clients[client].sendMsg(msg)

    def nickClientHandler(self, clientAddr, args):
        pass

    def newClientHandler(self, clientAddr, args):
        pass

    def deleteClientHandler(self, clientAddr, args):
        pass

    def subscribeChannelHandler(self, clientAddr, args):
        pass

    def unsubscribeChannelHandler(self, clientAddr, args):
        pass

    def listChannelHandler(self, clientAddr, args):

        pass


def processo2():
    # Cria servidor e escuta clients
    serv = ServerApp(portaHost)
    pass


# Para evitar dar pau com multi processos em python,
#   sempre colocar essa guarda, que evita processos filhos
#   de executarem o conteúdo da função
if __name__ == '__main__':
    main(processo1, processo2)
