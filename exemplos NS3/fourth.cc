#include <ns3/core-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/internet-module.h>
#include <ns3/applications-module.h>

#include "ns3/olsr-helper.h"

#include <ns3/netanim-module.h>
#include <ns3/bs-net-device.h>
#include <ns3/csma-module.h>
#include <ns3/uan-module.h>
using namespace ns3;


//Static routing example
int main()
{
    //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_ALL);
    //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);
    //LogComponentEnable("LiIonEnergySource", LOG_LEVEL_DEBUG);
    //LogComponentEnableAll(LOG_LEVEL_DEBUG);


    std::string outputFolder = "output";

    
    NodeContainer nodes, nodesRede1, nodesRede2;
    nodes.Create(6);
    nodesRede1.Add(nodes.Get(0));
    nodesRede1.Add(nodes.Get(1));
    nodesRede1.Add(nodes.Get(2));

    nodesRede2.Add(nodes.Get(3));
    nodesRede2.Add(nodes.Get(4));
    nodesRede2.Add(nodes.Get(5));

    InternetStackHelper internetHelper;
    internetHelper.Install(nodes);
    internetHelper.EnablePcapIpv4All(outputFolder+"pcap");

    NetDeviceContainer netInterfaces, netInterfacesRede1, netInterfacesRede2, netInterfacesRede3;
    PointToPointHelper pointHelper;
    pointHelper.SetChannelAttribute("Delay",StringValue("5ms"));
    //Cria conexões físicas entre os dois nós do primeiro container ( roteador = nodes(1) = nodesRede1.Get(1) )
    netInterfacesRede1.Add(pointHelper.Install(nodesRede1.Get(0), nodesRede1.Get(1)));
    netInterfacesRede1.Add(pointHelper.Install(nodesRede1.Get(0), nodesRede1.Get(2)));

    //Cria conexões físicas entre os dois nós do segundo container ( roteador = nodes(1) = nodesRede2.Get(0) )
    netInterfacesRede2.Add(pointHelper.Install(nodesRede1.Get(0), nodesRede2.Get(0)));

    //Cria conexões físicas entre os dois nós do segundo container ( roteador = nodes(2) = nodesRede3.Get(0) )

    netInterfacesRede3.Add(pointHelper.Install(nodesRede2.Get(0), nodesRede2.Get(1)));
    netInterfacesRede3.Add(pointHelper.Install(nodesRede2.Get(0), nodesRede2.Get(2)));

    pointHelper.EnablePcapAll(outputFolder+"pcap", true);

    Ipv4InterfaceContainer interfaceRede1, interfaceRede2, interfaceRede3;
    Ipv4AddressHelper ipv4Helper;
    ipv4Helper.SetBase("192.168.1.0","255.255.255.0");
    interfaceRede1 = ipv4Helper.Assign(netInterfacesRede1);
    ipv4Helper.NewNetwork();

    ipv4Helper.SetBase("192.168.2.0","255.255.255.0");
    interfaceRede2 = ipv4Helper.Assign(netInterfacesRede2);
    ipv4Helper.NewNetwork();

    ipv4Helper.SetBase("192.168.3.0","255.255.255.0");
    interfaceRede3 = ipv4Helper.Assign(netInterfacesRede3);
    ipv4Helper.NewNetwork();

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    //Configurar aplicações
    ApplicationContainer clientApp, serverApp;

    UdpEchoServerHelper echoServerHelper(8080);
    serverApp = echoServerHelper.Install(nodesRede1.Get(2)); //Instala servidor echo em 192.168.1.0 ou 1

    UdpEchoClientHelper echoClientHelper(nodesRede1.Get(2)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),8080);
    clientApp = echoClientHelper.Install(nodesRede2.Get(2)); //Instala cliente echo em 192.168.1.1 ou 2

    serverApp.Start(Seconds(1));
    serverApp.Stop(Seconds(50));
    clientApp.Start(Seconds(2));
    clientApp.Stop(Seconds(49));

    //A partir daqui, tem um servidor escutando na porta 8080 e um cliente mandando mensagens para ela


    //Exportar simulação para netanim
    BaseStationNetDevice b;
    SubscriberStationNetDevice s;
    CsmaNetDevice c;
    UanNetDevice u;

    AnimationInterface anim(outputFolder+"anim2.xml");
    anim.SetMaxPktsPerTraceFile(0xFFFFFFFF);
    anim.EnablePacketMetadata(true);
    anim.EnableIpv4RouteTracking (outputFolder+"routingtable-wireless.xml", Seconds (0), Seconds (9), Seconds (0.25));

    //Executar simulação por 10 segundos e depois destruir
    Simulator::Stop(Seconds(50));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}