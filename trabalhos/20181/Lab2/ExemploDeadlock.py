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
    time.sleep(5)

    # requisita estabelecimento de conexão
    sock.connect((socket.gethostname(), portaHost))

    while 1:
        # transforma mensagem em bytes e transmite
        sock.send(bytes(mensagem, "utf-8"))
        print("Cliente id %d: enviou mensagem" % id)
        time.sleep(1)

        # recebe confirmação
        answer = sock.recv(10)

    pass


def processo2():
    # requisita API do SO uma conexão AF_INET (IPV4)
    #   com protocolo de transporte SOCK_STREAM (TCP)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # requisita ao SO a posse de uma porta associada a um IP
    sock.bind((socket.gethostname(), portaHost))

    # requisita ao SO que a porta indicada de um dado IP seja
    #   reservado para escuta
    sock.listen(10)

    clients = {}

    while 1:
        # aceita requisição de conexão do processo 1,
        #   e recebe um socket para o cliente e o
        #   endereço de IP dele
        (clientsocket, address) = sock.accept()
        clients[address] = (clientsocket, address)

        for client in clients.values():
            # envia mensagens previamente enviadas (por exemplo)
            client[0].send(bytes(mensagem, "utf-8"))

            # recebe do socket do cliente (processo 1) uma mensagem de 10 bytes
            start = time.time()
            mensagem_recebida = client[0].recv(9)
            duration = time.time() - start

            print("Servidor recebe de cliente %s:%d após %.2f segundos" % (client[1][0], client[1][1], duration))

    pass


# Para evitar dar pau com multi processos em python,
#   sempre colocar essa guarda, que evita processos filhos
#   de executarem o conteúdo da função
if __name__ == '__main__':
    main(processo1, processo2)
