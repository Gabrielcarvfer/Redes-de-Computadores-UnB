#include <ns3/core-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/internet-module.h>
#include <ns3/applications-module.h>

//Para netanim
#include <ns3/netanim-module.h>
#include <ns3/bs-net-device.h>
#include <ns3/csma-module.h>
#include <ns3/uan-module.h>

using namespace ns3;


//Simple network setup
int main()
{

    std::string outputFolder = "output";

    
    //Cria nós (hosts/pcs/celulares/dispositivos quaisquer)
    NodeContainer nodes;
    nodes.Create(2);

    //Cria um auxiliar para criação de redes ponto a ponto e configura atributa das conexões criadas posteriormente. O auxiliar instala interfaces/placas de rede e as liga (cabo/canal/meio compartilhado) para determinados nós de rede
    PointToPointHelper pointHelper;
    pointHelper.SetChannelAttribute("Delay",StringValue("5ms"));

    //Instala interfaces de rede ponto a ponto ligando nós e devolve uma estrutura (basicamente uma lista) das interfaces
    NetDeviceContainer netInterfaces;
    netInterfaces = pointHelper.Install(nodes);

    //Habilita captura de tráfego na conexão ponto a ponto
    pointHelper.EnablePcapAll(outputFolder+"pcap", true);
    
    //Instala pilha TCP/IP nos nós
    InternetStackHelper internetHelper;
    internetHelper.Install(nodes);

    //Configura subrede 192.168.1.0-192.168.1.255
    Ipv4AddressHelper ipv4Helper;
    ipv4Helper.SetBase("192.168.1.0","255.255.255.0");
    ipv4Helper.Assign(netInterfaces);

    //Configurar aplicações
    ApplicationContainer clientApp, serverApp;

    UdpEchoServerHelper echoServerHelper(8080);
    serverApp = echoServerHelper.Install(nodes.Get(0)); //Instala servidor echo em 192.168.1.0 ou 1

    UdpEchoClientHelper echoClientHelper(Ipv4Address("192.168.1.0"),8080);
    clientApp = echoClientHelper.Install(nodes.Get(1)); //Instala cliente echo em 192.168.1.1 ou 2

    serverApp.Start(Seconds(1));
    serverApp.Stop(Seconds(10));
    clientApp.Start(Seconds(2));
    clientApp.Stop(Seconds(9));

    //A partir daqui, tem um servidor escutando na porta 8080 e um cliente mandando mensagens para ela


    //Exportar animação para o Netanim
    BaseStationNetDevice b;
    SubscriberStationNetDevice s;
    CsmaNetDevice c;
    UanNetDevice u;

    AnimationInterface anim(outputFolder+"anim2.xml");
    anim.SetMaxPktsPerTraceFile(0xFFFFFFFF);
    anim.EnablePacketMetadata(true);
    anim.EnableIpv4RouteTracking (outputFolder+"routingtable-wireless.xml", Seconds (0), Seconds (9), Seconds (0.25));

    //Rodar o simulador
    Simulator::Stop(Seconds(10)); // Rodar simulação por 10 segundos
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}