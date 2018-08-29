import socket
import time
portaHost = 65000 #porta do servidor (fica escutando esperando conexões)
mensagem = "abobrinha" #mensagem a ser enviada

def processo1():
    # requisita API do SO uma conexão AF_INET (IPV4)
    #   com protocolo de transporte SOCK_STREAM (TCP)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # processo dorme por 10 segundos para evitar
    #   se conectar com servidor antes desse estar rodando
    time.sleep(10)

    # requisita estabelecimento de conexão
    sock.connect((socket.gethostname(),portaHost))

    while 1:
        # transforma mensagem em bytes e transmite
        sock.send(bytes(mensagem,"utf-8"))
        print("Cliente:", mensagem)
        time.sleep(5)

    return

def processo2():
    # requisita API do SO uma conexão AF_INET (IPV4)
    #   com protocolo de transporte SOCK_STREAM (TCP)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # requisita ao SO a posse de uma porta associada a um IP
    sock.bind((socket.gethostname(), portaHost))

    # requisita ao SO que a porta indicada de um dado IP seja
    #   reservado para escuta
    sock.listen(5)

    while 1:
        # aceita requisição de conexão do processo 1,
        #   e recebe um socket para o cliente e o
        #   endereço de IP dele
        (clientsocket, address) = sock.accept()

        while 1:
            # recebe do socket do cliente (processo 1) uma mensagem de 10 bytes
            mensagem_recebida = clientsocket.recv(10)

            print("Servidor:", mensagem_recebida)


    return

def main():
    import multiprocessing as mp
    processes = []

    # cria 2 processos, um servidor (processo2) e um cliente (processo1)
    processes += [mp.Process(target=processo2)]
    processes += [mp.Process(target=processo1)]

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