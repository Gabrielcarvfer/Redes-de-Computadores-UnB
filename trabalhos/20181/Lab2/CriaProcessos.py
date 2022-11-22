def main(processo1, processo2, num_clientes=1):
    import multiprocessing as mp
    mp.freeze_support()
    processes = []
    # Cria processo servidor
    processes += [mp.Process(target=processo2)]

    # Cria processos clientes
    for id in range(num_clientes):
        processes += [mp.Process(target=processo1, args=(id,))]

    # inicia os dois processos (pode olhar no gerenciador de tarefas,
    #    que lá estarão)
    for process in processes:
        process.start()

    # espera pela finalização dos processos filhos
    #   (em Sistemas operacionais, aprenderão mais sobre o assunto)
    for process in processes:
        process.join()

    return