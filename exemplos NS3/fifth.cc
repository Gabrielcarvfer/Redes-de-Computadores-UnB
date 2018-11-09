#include <ns3/core-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/internet-module.h>
#include <ns3/applications-module.h>
#include <ns3/wifi-module.h>
#include <ns3/mobility-module.h>

//Para netanim
#include <ns3/netanim-module.h>
#include <ns3/bs-net-device.h>
#include <ns3/csma-module.h>
#include <ns3/uan-module.h>

using namespace ns3;

void setup_mobility2(NodeContainer * nodes, std::string mobilityModel, double x, double y, double z, double radius)
{
    MobilityHelper mobility;

    Ptr<RandomBoxPositionAllocator> randomPositionAlloc = CreateObject<RandomBoxPositionAllocator> ();
    Ptr<UniformRandomVariable> xVal = CreateObject<UniformRandomVariable> ();
    xVal->SetAttribute ("Min", DoubleValue (x-radius));
    xVal->SetAttribute ("Max", DoubleValue (x+radius));
    Ptr<UniformRandomVariable> yVal = CreateObject<UniformRandomVariable> ();
    yVal->SetAttribute ("Min", DoubleValue (y-radius));
    yVal->SetAttribute ("Max", DoubleValue (y+radius));
    Ptr<UniformRandomVariable> zVal = CreateObject<UniformRandomVariable> ();
    zVal->SetAttribute ("Min", DoubleValue (z));
    zVal->SetAttribute ("Max", DoubleValue (z));

    mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
                                  "X", PointerValue (xVal),
                                  "Y", PointerValue (yVal),
                                  "Z", PointerValue (zVal)
    );
    mobility.SetMobilityModel(mobilityModel);
    mobility.Install(*nodes);
}

//Simple network setup
int main()
{

    std::string outputFolder = "output";


    NodeContainer ethernet1, ethernet2;
    NodeContainer wifi1, wifi2;
    NodeContainer switch1, switch2;
    NodeContainer router1, router2;
    ethernet1.Create(125);
    ethernet2.Create(125);
    wifi1.Create(25);
    wifi2.Create(25);
    switch1.Create(2);
    switch2.Create(2);
    router1.Create(1);
    router2.Create(1);

    NodeContainer dominio1, dominio2;
    dominio1.Add(ethernet1);
    dominio1.Add(wifi1);
    dominio1.Add(switch1);
    dominio1.Add(router1);
    dominio2.Add(ethernet2);
    dominio2.Add(wifi2);
    dominio2.Add(switch2);
    dominio2.Add(router2);




    NodeContainer internet, todosOsNos;
    internet.Create(10);


    setup_mobility2(&dominio1, "ns3::ConstantPositionMobilityModel",100,100,0,20);
    setup_mobility2(&dominio2, "ns3::ConstantPositionMobilityModel",500,100,0,20);
    setup_mobility2(&internet, "ns3::ConstantPositionMobilityModel",250,100,0,20);

    todosOsNos.Add(dominio1);
    todosOsNos.Add(dominio2);
    todosOsNos.Add(internet);

    NodeContainer nosComCsma1, nosComCsma2;
    nosComCsma1.Add(ethernet1);
    nosComCsma2.Add(ethernet2);
    nosComCsma1.Add(switch1);
    nosComCsma2.Add(switch2);
    nosComCsma1.Add(router1);
    nosComCsma2.Add(router2);

    NodeContainer routers1,routers2;
    routers1.Add(nosComCsma1.Get(0));
    routers1.Add(internet.Get(0));
    routers2.Add(nosComCsma2.Get(0));
    routers2.Add(internet.Get(9));



    CsmaHelper csmaHelper;

    NetDeviceContainer csmaInterfaces1, csmaInterfaces2, csmaInterfacesInt, csmaInterfacesRouters;
    csmaInterfaces1 = csmaHelper.Install(nosComCsma1);
    csmaInterfaces2 = csmaHelper.Install(nosComCsma2);
    csmaInterfacesInt = csmaHelper.Install(internet);
    csmaInterfacesRouters.Add(csmaHelper.Install(routers1));
    csmaInterfacesRouters.Add(csmaHelper.Install(routers2));

    csmaHelper.EnablePcapAll("csma",true);


    InternetStackHelper internetHelper;
    internetHelper.Install(todosOsNos);

    Ipv4AddressHelper ipv4Helper;
    ipv4Helper.SetBase("192.168.1.0","255.255.255.0");
    ipv4Helper.Assign(csmaInterfaces1);

    ipv4Helper.NewNetwork();
    ipv4Helper.SetBase("192.168.2.0","255.255.255.0");
    ipv4Helper.Assign(csmaInterfaces2);

    ipv4Helper.NewNetwork();
    ipv4Helper.SetBase("192.168.3.0","255.255.255.0");
    ipv4Helper.Assign(csmaInterfacesInt);

    ipv4Helper.NewNetwork();
    ipv4Helper.SetBase("192.168.4.0","255.255.255.0");
    ipv4Helper.Assign(csmaInterfacesRouters);


    //Configurar aplicações
    ApplicationContainer clientApp, serverApp;

    UdpEchoServerHelper echoServerHelper(8080);
    serverApp = echoServerHelper.Install(ethernet1.Get(29)); //Instala servidor echo em 192.168.1.0 ou 1

    UdpEchoClientHelper echoClientHelper(Ipv4Address("192.168.1.30"),8080);
    echoClientHelper.SetAttribute("Interval",TimeValue(MilliSeconds(10)));
    clientApp = echoClientHelper.Install(ethernet2.Get(29)); //Instala cliente echo em 192.168.1.1 ou 2

    serverApp.Start(Seconds(1));
    serverApp.Stop(Seconds(10));
    clientApp.Start(Seconds(2));
    clientApp.Stop(Seconds(9));

    //A partir daqui, tem um servidor escutando na porta 8080 e um cliente mandando mensagens para ela
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


    //Exportar animação para o Netanim
    BaseStationNetDevice b;
    SubscriberStationNetDevice s;
    CsmaNetDevice c;
    UanNetDevice u;

    AnimationInterface anim(outputFolder+"anim2.xml");
    //anim.SetMaxPktsPerTraceFile(0xFFFFFFFF);
    //anim.EnablePacketMetadata(true);
    //anim.EnableIpv4RouteTracking (outputFolder+"routingtable-wireless.xml", Seconds (0), Seconds (9), Seconds (0.25));

    //Rodar o simulador
    Simulator::Stop(Minutes(2)); // Rodar simulação por 10 segundos
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}