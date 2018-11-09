//
// Created by gabriel on 1/30/18.
//

#include <ns3/core-module.h>
#include <ns3/lte-module.h>
#include <ns3/internet-module.h>
#include <ns3/mobility-module.h>

//Para netanim
#include <ns3/netanim-module.h>
#include <ns3/bs-net-device.h>
#include <ns3/csma-module.h>
#include <ns3/uan-module.h>

using namespace ns3;

//Simple network setup
int main()
{
    //Create network nodes and containers
    NodeContainer UEnodes, eNBnodes;
    UEnodes.Create(2);
    eNBnodes.Create(1);

    //Set mobility models of nodes
    MobilityHelper mobilityHelper;
    mobilityHelper.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityHelper.Install(eNBnodes);

    mobilityHelper.SetMobilityModel("ns3::RandomWalk2dMobilityModel");
    mobilityHelper.Install(UEnodes);

    //Setup LTE
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();

    //Install configured LTE
    NetDeviceContainer UEnetInterfaces, eNBnetInterfaces;
    eNBnetInterfaces = lteHelper->InstallEnbDevice(eNBnodes);
    UEnetInterfaces  = lteHelper->InstallUeDevice(UEnodes);

    //Attach UE to eNB
    lteHelper->Attach(UEnetInterfaces, eNBnetInterfaces.Get(0));

    //Activate a data radio bearer
    enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer(q);
    lteHelper->ActivateDataRadioBearer(UEnetInterfaces, bearer);
    //lteHelper->EnableTraces(); //Needs fix

    //Exportar animação para o Netanim
    BaseStationNetDevice b;
    SubscriberStationNetDevice s;
    CsmaNetDevice c;
    UanNetDevice u;

    AnimationInterface anim("anim.xml");
    anim.SetMaxPktsPerTraceFile(0xFFFFFFFF);
    anim.EnablePacketMetadata(true);

    //Rodar o simulador
    Simulator::Stop(Seconds(1)); // Rodar simulação por 10 segundos
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}