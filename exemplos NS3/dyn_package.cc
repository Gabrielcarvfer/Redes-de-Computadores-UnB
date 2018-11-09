// Created by Gabriel Ferreira (@gabrielcarvfer) in 13/03/2018

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/on-off-helper.h"

using namespace ns3;

/***************************************************************
Comece pela linha 156 (função main), e depois volte para cá.
****************************************************************/

//Início da aplicação customizada simples
class CustomApp : public Application
{
public:

    CustomApp ();
    virtual ~CustomApp();

    void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize,
                uint32_t nPackets, DataRate dataRate);

    void SetPacketSize(uint32_t packetSize);

private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);

    void ScheduleTx (void);
    void SendPacket (void);

    Ptr<Socket>     m_socket;
    Address         m_peer;
    uint32_t        m_packetSize;
    uint32_t        m_nPackets;
    DataRate        m_dataRate;
    EventId         m_sendEvent;
    bool            m_running;
    uint32_t        m_packetsSent;
};


CustomApp::CustomApp ()
        : m_socket (0),
          m_peer (),
          m_packetSize (0),
          m_nPackets (0),
          m_dataRate (0),
          m_sendEvent (),
          m_running (false),
          m_packetsSent (0)
{
}

CustomApp::~CustomApp()
{
    m_socket = 0;
}
void CustomApp::SetPacketSize(uint32_t packetSize)
{
    this->m_packetSize = packetSize;
}


void
CustomApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize,
              uint32_t nPackets, DataRate dataRate)
{
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    m_nPackets = nPackets;
    m_dataRate = dataRate;
}

void
CustomApp::StartApplication (void)
{
    m_running = true;
    m_packetsSent = 0;
    m_socket->Bind ();
    m_socket->Connect (m_peer);
    SendPacket ();
}

void
CustomApp::StopApplication (void)
{
    m_running = false;

    if (m_sendEvent.IsRunning ())
    {
        Simulator::Cancel (m_sendEvent);
    }

    if (m_socket)
    {
        m_socket->Close ();
    }
}

void
CustomApp::SendPacket (void)
{
    Ptr<Packet> packet = Create<Packet> (m_packetSize);
    m_socket->Send (packet);

    if (++m_packetsSent < m_nPackets)
    {
        ScheduleTx ();
    }
}

void
CustomApp::ScheduleTx (void)
{
    if (m_running)
    {
        Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
        m_sendEvent = Simulator::Schedule (tNext, &CustomApp::SendPacket, this);
    }
}
//Fim da aplicação customizada simplificada

//Evento a ser agendado, recebe como parâmetro no qual roda a aplicação que deve ter o tráfego modificado
static void sendPacket(Ptr<Node> node)
{
    //Recupera aplicação rodando no nó da rede
    ApplicationContainer app = node->GetApplication(0);

    //Variáveis a serem utilizadas/modificadas
    unsigned packetSize, changeTrafficInterval;

    //Geração dos valores 
    packetSize = rand() % 1024;
    changeTrafficInterval = rand() % 10;
    
    //Acesso ao método que altera as propriedades da conexão, que são serão completamente efetivadas ou na próxima
    // conexão, ou pode cancelar o m_sendEvent da aplicação, e reagendar para um intervalo diferente de transmissão
    app.Get(0)->GetObject<CustomApp>()->SetPacketSize(packetSize);

    //Reagenda esse procedimento maroto para modificar o tráfego novamente
    Simulator::Schedule(MilliSeconds(changeTrafficInterval), &sendPacket, node);
    return;
}

int
main (int argc, char *argv[])
{
    //Imprime no console logs de todos os componentes em nível de debug
    LogComponentEnableAll(LOG_LEVEL_DEBUG);

    //Cria estrutura para manipular nós da rede
    NodeContainer nodes;

    //Cria 2 nós na rede
    nodes.Create(2);

    //Cria estrutura para manipular interfaces de rede
    NetDeviceContainer netDevices;

    //Cria ajudante para configurar e criar interface de rede ponto a ponto
    PointToPointHelper p2pHelper;
    
    //Configure dispositivos da rede ponto a ponto para transmitirem a 50Mbps
    p2pHelper.SetDeviceAttribute("DataRate", StringValue("50Mbps"));
    
    //Instala interfaces de rede ponto a ponto nos nós da rede e os guarda na estrutura para futura manipulação
    netDevices = p2pHelper.Install(nodes);

    //Exporta tráfego de rede para arquivos output-#nó.pcap
    p2pHelper.EnablePcapAll("output", true);

    //Cria ajudante para instalar pilha de rede
    InternetStackHelper internetHelper;

    //Instala pilha de rede IP nos nós
    internetHelper.Install(nodes);
    
    //Cria ajudante para dar endereços IP
    Ipv4AddressHelper ipv4h;
    
    //Configure endereço de base e máscara de subrede
    ipv4h.SetBase ("192.168.1.0", "255.255.255.0");
    
    //Dá IPs às interfaces de rede
    ipv4h.Assign (netDevices);
    
    //Cria estruturas para manipular cliente e servidor
    ApplicationContainer client, server;

    //Cria aplicação packet sinker (recebe pacotes e dá ack) para o servidor, usando TCP, escutando qualquer endereço na porta 80
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 80));
    
    //Instala aplicação no servidor
    server = packetSinkHelper.Install(nodes.Get(1));
    
    //Inicia a aplicação após 100ms de simulação
    server.Start(Seconds(0.1));

    //Cria socket Tcp para cliente
    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId());
    
    //Cria instância da aplicação customizada
    Ptr<CustomApp> app = CreateObject<CustomApp> ();
    
    //Cria instância com endereço e porta do servidor
    Address serverAddress (InetSocketAddress(Ipv4Address("192.168.1.2"),80));
    
    //Configura aplicação customizada com o endereço, pacotes de tamanho 10B, com 100000 pacotes a serem enviados a 50000bps
    app->Setup(ns3TcpSocket, serverAddress, 10, 100000, 50000);//nodes.Get(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal()
    
    //Instala aplicação no nó cliente
    nodes.Get(0)->AddApplication(app);
    
    //Inicia a aplicação após 100ms de simulação
    client.Start(Seconds(0.1));

    //Agenda evento sendPacket (que muda o tamanho dos pacotes, e talvez outras propriedades do tráfego)
    // após 100ms do início da simulação
    Simulator::Schedule(Seconds(0.1), &sendPacket, nodes.Get(0));

    //Roda simulação por 10 segundos
    Simulator::Stop(Seconds(10));
    Simulator::Run ();
    Simulator::Destroy ();
}