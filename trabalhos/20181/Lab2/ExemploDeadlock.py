import socket
import time
portaHost = 65000 #porta do servidor (fica escutando esperando conexões)
mensagem = "abobrinha" #mensagem a ser enviada

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
        sock.send(bytes(mensagem,"utf-8"))
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
            mensagem_recebida = client[0].recv(10)
            duration = time.time() - start

            print("Servidor recebe de cliente %s:%d após %.2f segundos" % (client[1][0],client[1][1], duration) )




    pass

def main():
    import multiprocessing as mp
    processes = []

    processes += [mp.Process(target=processo2)]

    for id in range(1):
        processes += [mp.Process(target=processo1, args=[id])]

    # inicia os dois processos (pode olhar no gerenciador de tarefas,
    #    que lá estarão
    for process in processes:
        process.start()

    # espera pela finalização dos processos filhos
    #   (em Sistemas operacionais verão o que isso significa)
    for process in processes:
        process.join()

    return

# Para evitar dar pau com multiprocessos em python,
#   sempre colocar essa guarda, que evita processos filhos
#   de executarem a o conteúdo da função
if __name__ == '__main__':
    main()