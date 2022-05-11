/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation;
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   Author: Marco Miozzo <marco.miozzo@cttc.es>
*           Nicola Baldo  <nbaldo@cttc.es>
*
*   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
*                         Sourjya Dutta <sdutta@nyu.edu>
*                         Russell Ford <russell.ford@nyu.edu>
*                         Menglei Zhang <menglei@nyu.edu>
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include "ns3/mmwave-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store-module.h"
#include "ns3/command-line.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/epc-helper.h"
#include "ns3/node-list.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/buildings-helper.h"
#include "ns3/buildings-module.h"
#include "ns3/global-value.h"
#include <ns3/random-variable-stream.h>
#include <ns3/lte-ue-net-device.h>
#include <ctime>
#include <stdlib.h>
#include <list>

using namespace ns3;
using namespace mmwave;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */
 static double
CalculateAngle(Vector myposition)
{
  double angle;
  const double RadtoDeg = 57.2957951; //180/pi
  
  if(myposition.y==0 && myposition.x==0){ //velocity=0
    angle=0;
    }
  
  else{
    angle = atan2(myposition.y, myposition.x) * RadtoDeg; //degree
    }
    
    
  return angle;

}
 
/*MEC - gNB Interface */
void
AddMECgNBInterface (Ptr<Node> gnb, Ptr<Node> mec, PointToPointHelper p2pmgb, Ipv4AddressHelper mgbipv4h)
{

  NetDeviceContainer mgbDevices = p2pmgb.Install (gnb, mec);
  Ipv4InterfaceContainer mgbIpIfaces = mgbipv4h.Assign (mgbDevices);

/*
  Ipv4StaticRoutingHelper mgbipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = mgbipv4RoutingHelper.GetStaticRouting (mec->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

 */
  
}
static void
ModifyPacketData_UE (Ptr<Node> node, UdpClientHelper udpclient, Ptr <Application> app, uint8_t type)
{
std::cout<<"modify call"<<std::endl;
   
   Ptr<MobilityModel> mymobility=node->GetObject<MobilityModel>();  //get mobility model

   uint32_t ID=node->GetId();

   std::string nodeID=std::to_string(ID);
   
   Vector myvelocity = mymobility -> GetVelocity(); //get Velocity vector
   Vector myposition = mymobility -> GetPosition(); //get Position vector
   
   double angle = CalculateAngle(myposition);
   
   //Literally, sending_data is the data to be snet in a packet
   std::string sending_data; 
   std::string pos_x, pos_y, velo_x, velo_y;
   std::string angle_str, nodeType;
   nodeType=std::to_string(type);
   pos_x=std::to_string(myposition.x);
   pos_y=std::to_string(myposition.y);
   velo_x=std::to_string(myvelocity.x);
   velo_y=std::to_string(myvelocity.y);
   angle_str=std::to_string(angle);
   
   
   //Each data element in the packet is separated by "/"
   sending_data.append(nodeType); sending_data.append("/");//ped:0, veh:1
   sending_data.append(nodeID); sending_data.append("/");
   
   sending_data.append(pos_x); sending_data.append("/");
   sending_data.append(pos_y); sending_data.append("/");
 
   sending_data.append(velo_x); sending_data.append("/");
   sending_data.append(velo_y); sending_data.append("/");
   sending_data.append(angle_str); 

   udpclient.SetFill(app, sending_data);

   Simulator::Schedule(Seconds(0.1), &ModifyPacketData_UE, node, udpclient, app, type);
}


static ns3::GlobalValue g_bufferSize ("bufferSize", "RLC tx buffer size (MB)",
                                      ns3::UintegerValue (20), ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_x2Latency ("x2Latency", "Latency on X2 interface (us)",
                                     ns3::DoubleValue (500), ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_mmeLatency ("mmeLatency", "Latency on MME interface (us)",
                                      ns3::DoubleValue (10000), ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_mobileUeSpeed ("mobileSpeed", "The speed of the UE (m/s)",
                                         ns3::DoubleValue (2), ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_rlcAmEnabled ("rlcAmEnabled", "If true, use RLC AM, else use RLC UM",
                                        ns3::BooleanValue (true), ns3::MakeBooleanChecker ());
static ns3::GlobalValue g_noiseAndFilter ("noiseAndFilter", "If true, use noisy SINR samples, filtered. If false, just use the SINR measure",
                                          ns3::BooleanValue (false), ns3::MakeBooleanChecker ());
static ns3::GlobalValue g_handoverMode ("handoverMode",
                                        "Handover mode",
                                        ns3::UintegerValue (3), ns3::MakeUintegerChecker<uint8_t> ());
static ns3::GlobalValue g_reportTablePeriodicity ("reportTablePeriodicity", "Periodicity of RTs",
                                                  ns3::UintegerValue (1600), ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_outageThreshold ("outageTh", "Outage threshold",
                                           ns3::DoubleValue (-5), ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_lteUplink ("lteUplink", "If true, always use LTE for uplink signalling",
                                     ns3::BooleanValue (false), ns3::MakeBooleanChecker ());


 
NS_LOG_COMPONENT_DEFINE ("EpcFirstExample");
int
main (int argc, char *argv[])
{
  uint16_t numEnb = 126; //126
  uint16_t numUe = 27; //132
  double simTime = 3.0;
  double interPacketInterval = 100;
  //double minDistance = 10.0; // eNB-UE distance in meters
  //doufble maxDistance = 150.0; // eNB-UE distance in meters
  bool harqEnabled = false;
  bool fixedTti = false;
  //bool rlcAmEnabled = false;

  UintegerValue uintegerValue;
  BooleanValue booleanValue;
  StringValue stringValue;
  DoubleValue doubleValue;
  std::string traceFile;

  //uint16_t mecLevel;

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue ("numEnb", "Number of eNBs", numEnb);
  cmd.AddValue ("numUe", "Number of UEs per eNB", numUe);
  cmd.AddValue ("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue ("interPacketInterval", "Inter-packet interval [us])", interPacketInterval);
  cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
  cmd.AddValue ("harq", "Enable Hybrid ARQ", harqEnabled);
  //cmd.AddValue ("rlcAm", "Enable RLC-AM", rlcAmEnabled);
  cmd.Parse (argc, argv);

   //Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue (rlcAmEnabled));
  Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue (harqEnabled));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue (harqEnabled));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::TbDecodeLatency", UintegerValue (200.0));
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (MilliSeconds (100000.0)));
  Config::SetDefault ("ns3::LteEnbRrc::SystemInformationPeriodicity", TimeValue (MilliSeconds (5.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue (MicroSeconds (100.0)));
  Config::SetDefault ("ns3::LteRlcUmLowLat::ReportBufferStatusTimer", TimeValue (MicroSeconds (100.0)));
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));
  Config::SetDefault ("ns3::LteEnbRrc::FirstSibTime", UintegerValue (2));
Config::SetDefault ("ns3::MmWaveHelper::UseIdealRrc", BooleanValue (true));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1apLinkDelay", TimeValue (Seconds (0)));

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse (argc, argv);
 
  
   if (traceFile.empty () || numUe <= 0 || simTime <= 0)
    {
      std::cout << "Usage of " << argv[0] << " :\n\n"
      "./waf --run \"ns2-mobility-trace"
      " --traceFile=src/mobility/examples/default.ns_movements"
      " --nodeNum=2 --duration=100.0 --logFile=ns2-mob.log\" \n\n"
      "NOTE: ns2-traces-file could be an absolute or relative path. You could use the file default.ns_movements\n"
      "      included in the same directory of this example file.\n\n"
      "NOTE 2: Number of nodes present in the trace file must match with the command line argument and must\n"
      "        be a positive number. Note that you must know it before to be able to load it.\n\n"
      "NOTE 3: Duration must be a positive number. Note that you must know it before to be able to load it.\n\n";

      return 0;
  }  
  
  int windowForTransient = 150; // number of samples for the vector to use in the
  GlobalValue::GetValueByName ("reportTablePeriodicity", uintegerValue);
  int ReportTablePeriodicity = (int)uintegerValue.Get (); // in microseconds
  if (ReportTablePeriodicity == 1600)
    {
      windowForTransient = 150;
    }
  else if (ReportTablePeriodicity == 25600)
    {
      windowForTransient = 50;
    }
  else if (ReportTablePeriodicity == 12800)
    {
      windowForTransient = 100;
    }
  else
    {
      NS_ASSERT_MSG (false, "Unrecognized");
    }

  
  

  int vectorTransient = windowForTransient * ReportTablePeriodicity;
   // params for RT, filter, HO mode
  GlobalValue::GetValueByName ("noiseAndFilter", booleanValue);
  bool noiseAndFilter = booleanValue.Get ();
  GlobalValue::GetValueByName ("handoverMode", uintegerValue);
  uint8_t hoMode = uintegerValue.Get ();
  GlobalValue::GetValueByName ("outageTh", doubleValue);
  double outageTh = doubleValue.Get ();

  GlobalValue::GetValueByName ("rlcAmEnabled", booleanValue);
  bool rlcAmEnabled = booleanValue.Get ();
  GlobalValue::GetValueByName ("bufferSize", uintegerValue);
  uint32_t bufferSize = uintegerValue.Get ();
  //GlobalValue::GetValueByName ("interPckInterval", uintegerValue);
  //uint32_t interPacketInterval = uintegerValue.Get ();
  GlobalValue::GetValueByName ("x2Latency", doubleValue);
  double x2Latency = doubleValue.Get ();
  GlobalValue::GetValueByName ("mmeLatency", doubleValue);
  double mmeLatency = doubleValue.Get ();
  GlobalValue::GetValueByName ("mobileSpeed", doubleValue);
  double ueSpeed = doubleValue.Get ();

  //double transientDuration = double(vectorTransient) / 1000000;
  //double simTime = transientDuration + ((double)ueFinalPosition - (double)ueInitialPosition) / ueSpeed + 1;

  NS_LOG_UNCOND ("rlcAmEnabled " << rlcAmEnabled << " bufferSize " << bufferSize << " interPacketInterval " <<
                 interPacketInterval << " x2Latency " << x2Latency << " mmeLatency " << mmeLatency << " mobileSpeed " << ueSpeed);

  //GlobalValue::GetValueByName ("outPath", stringValue);
  std::string path = stringValue.Get ();
  std::string mmWaveOutName = "MmWaveSwitchStats";
  std::string lteOutName = "LteSwitchStats";
  std::string dlRlcOutName = "DlRlcStats";
  std::string dlPdcpOutName = "DlPdcpStats";
  std::string ulRlcOutName = "UlRlcStats";
  std::string ulPdcpOutName = "UlPdcpStats";
  std::string  ueHandoverStartOutName =  "UeHandoverStartStats";
  std::string enbHandoverStartOutName = "EnbHandoverStartStats";
  std::string  ueHandoverEndOutName =  "UeHandoverEndStats";
  std::string enbHandoverEndOutName = "EnbHandoverEndStats";
  std::string cellIdInTimeOutName = "CellIdStats";
  std::string cellIdInTimeHandoverOutName = "CellIdStatsHandover";
  std::string mmWaveSinrOutputFilename = "MmWaveSinrTime";
  std::string x2statOutputFilename = "X2Stats";
  std::string udpSentFilename = "UdpSent";
  std::string udpReceivedFilename = "UdpReceived";
  std::string extension = ".txt";
  std::string version;
  version = "mc";
  Config::SetDefault ("ns3::MmWaveUeMac::UpdateUeSinrEstimatePeriod", DoubleValue (0));

  //get current time
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  strftime (buffer,80,"%d_%m_%Y_%I_%M_%S",timeinfo);
  std::string time_str (buffer);

  Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue (rlcAmEnabled));
  Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue (harqEnabled));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue (harqEnabled));
  Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::HarqEnabled", BooleanValue (harqEnabled));
  Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::FixedTti", BooleanValue (fixedTti));
  Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::SymPerSlot", UintegerValue (6));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::TbDecodeLatency", UintegerValue (200.0));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::NumHarqProcess", UintegerValue (100));
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (MilliSeconds (100.0)));
  Config::SetDefault ("ns3::LteEnbRrc::SystemInformationPeriodicity", TimeValue (MilliSeconds (5.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue (MicroSeconds (100.0)));
  Config::SetDefault ("ns3::LteRlcUmLowLat::ReportBufferStatusTimer", TimeValue (MicroSeconds (100.0)));
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));
  Config::SetDefault ("ns3::LteEnbRrc::FirstSibTime", UintegerValue (2));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkDelay", TimeValue (MicroSeconds (x2Latency)));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkDataRate", DataRateValue (DataRate ("1000Gb/s")));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkMtu",  UintegerValue (10000));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1uLinkDelay", TimeValue (MicroSeconds (1000)));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1apLinkDelay", TimeValue (MicroSeconds (mmeLatency)));
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (bufferSize * 1024 * 1024));
  Config::SetDefault ("ns3::LteRlcUmLowLat::MaxTxBufferSize", UintegerValue (bufferSize * 1024 * 1024));
  Config::SetDefault ("ns3::LteRlcAm::StatusProhibitTimer", TimeValue (MilliSeconds (10.0)));
  Config::SetDefault ("ns3::LteRlcAm::MaxTxBufferSize", UintegerValue (bufferSize * 1024 * 1024));

  // handover and RT related params
  switch (hoMode)
    {
    case 1:
      Config::SetDefault ("ns3::LteEnbRrc::SecondaryCellHandoverMode", EnumValue (LteEnbRrc::THRESHOLD));
      break;
    case 2:
      Config::SetDefault ("ns3::LteEnbRrc::SecondaryCellHandoverMode", EnumValue (LteEnbRrc::FIXED_TTT));
      break;
    case 3:
      Config::SetDefault ("ns3::LteEnbRrc::SecondaryCellHandoverMode", EnumValue (LteEnbRrc::DYNAMIC_TTT));
      break;
    }

  Config::SetDefault ("ns3::LteEnbRrc::FixedTttValue", UintegerValue (150));
  Config::SetDefault ("ns3::LteEnbRrc::CrtPeriod", IntegerValue (ReportTablePeriodicity));
  Config::SetDefault ("ns3::LteEnbRrc::OutageThreshold", DoubleValue (outageTh));
  Config::SetDefault ("ns3::MmWaveEnbPhy::UpdateSinrEstimatePeriod", IntegerValue (ReportTablePeriodicity));
  Config::SetDefault ("ns3::MmWaveEnbPhy::Transient", IntegerValue (vectorTransient));
  Config::SetDefault ("ns3::MmWaveEnbPhy::NoiseAndFilter", BooleanValue (noiseAndFilter));
  
  // set the type of RRC to use, i.e., ideal or real
  // by setting the following two attributes to true, the simulation will use 
  // the ideal paradigm, meaning no packets are sent. in fact, only the callbacks are triggered
  Config::SetDefault ("ns3::MmWaveHelper::UseIdealRrc", BooleanValue(true));

  GlobalValue::GetValueByName ("lteUplink", booleanValue);
  bool lteUplink = booleanValue.Get ();

  Config::SetDefault ("ns3::McUePdcp::LteUplink", BooleanValue (lteUplink));
  std::cout << "Lte uplink " << lteUplink << "\n";

  // settings for the 3GPP the channel
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (MilliSeconds (100))); // interval after which the channel for a moving user is updated,
  Config::SetDefault ("ns3::ThreeGppChannelModel::Blockage", BooleanValue (true)); // use blockage or not
  Config::SetDefault ("ns3::ThreeGppChannelModel::PortraitMode", BooleanValue (true)); // use blockage model with UT in portrait mode
  Config::SetDefault ("ns3::ThreeGppChannelModel::NumNonselfBlocking", IntegerValue (4)); // number of non-self blocking obstacles

  // set the number of antennas in the devices
  Config::SetDefault ("ns3::McUeNetDevice::AntennaNum", UintegerValue(16));
  Config::SetDefault ("ns3::MmWaveNetDevice::AntennaNum", UintegerValue(64));
  
  // set to false to use the 3GPP radiation pattern (proper configuration of the bearing and downtilt angles is needed) 
  Config::SetDefault ("ns3::ThreeGppAntennaArrayModel::IsotropicElements", BooleanValue (true));   
    
    
    
  //LogComponentEnable ("UdpClient", LOG_LEVEL_ALL);
  LogComponentEnable ("PacketSink", LOG_LEVEL_ALL);
  LogComponentEnable ("LteHelper", LOG_LEVEL_ALL);
  
  NodeContainer ueNodes;  
  ueNodes.Create (numUe);
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);
  ns2.Install ();  
  
  Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper> ();
  mmwaveHelper->SetPathlossModelType ("ns3::ThreeGppUmiStreetCanyonPropagationLossModel");


  
  Ptr<MmWavePointToPointEpcHelper>  epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  mmwaveHelper->SetEpcHelper (epcHelper);
  mmwaveHelper->SetHarqEnabled (harqEnabled);
  mmwaveHelper->Initialize ();
  
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  Ptr<Node> mme = epcHelper->GetMmeNode ();
  
  
  
  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  //EPC node position allocator (S/PGW, MME, remote host)
  NodeContainer EPC;
  EPC.Add(pgw);
  EPC.Add(mme);
  EPC.Add(remoteHost);
  
  MobilityHelper EPCmobility;
  EPCmobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (6280.0),
                                 "MinY", DoubleValue (2500.0),
                                 "DeltaX", DoubleValue (20.0),
                                 "DeltaY", DoubleValue (30.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
  EPCmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  EPCmobility.Install (EPC);
  
  
  

  NodeContainer enbNodes;
  enbNodes.Create (numEnb);

  // Install Mobility Model
  
  MobilityHelper enbmobility;
  enbmobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (4200.0),
                                 "MinY", DoubleValue (3300.0),
                                 "DeltaX", DoubleValue (300.0),
                                 "DeltaY", DoubleValue (300.0),
                                 "GridWidth", UintegerValue (14),
                                 "LayoutType", StringValue ("RowFirst"));
  enbmobility.AssignStreams(ueNodes, 132);
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.Install (enbNodes);

  // Making MEC nodecontainers
  NodeContainer MECnodes;
  MECnodes.Create(4);
  
  MobilityHelper MECmobility;
  MECmobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (5250.0),
                                 "MinY", DoubleValue (3750.0),
                                 "DeltaX", DoubleValue (1950.0),
                                 "DeltaY", DoubleValue (1500.0),
                                 "GridWidth", UintegerValue (2),
                                 "LayoutType", StringValue ("RowFirst"));  
  MECmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  MECmobility.Install (MECnodes);
  internet.Install (MECnodes);
  
 







  
  // Install mmWave Devices to the nodes
  NetDeviceContainer enbmmWaveDevs = mmwaveHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer uemmWaveDevs = mmwaveHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (uemmWaveDevs));




  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
     
    }
    
      mmwaveHelper->AddX2Interface (enbNodes);
      std::cout<<"x2 interface done"<<std::endl;  
      mmwaveHelper->AttachToClosestEnb (uemmWaveDevs, enbmmWaveDevs);
 /*
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  mmwaveHelper->ActivateDataRadioBearer (uemmWaveDevs, bearer);
  */


  // find the closest MEC
  double minDistance = std::numeric_limits<double>::infinity ();
  int closestMECIndex = -1;
  
  PointToPointHelper p2pmgb;
  
  p2pmgb.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2pmgb.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2pmgb.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  
  
  Ipv4AddressHelper mgbipv4h;
  mgbipv4h.SetBase ("199.0.0.0", "255.0.0.0");
  NetDeviceContainer mgbDevices;
  Ipv4InterfaceContainer mgbIpIfaces; 
  for (uint32_t i = 0; i < enbmmWaveDevs.GetN (); ++i)
    {
    minDistance = std::numeric_limits<double>::infinity ();
    closestMECIndex = -1;
    for (NodeContainer::Iterator j = MECnodes.Begin(); j != MECnodes.End (); ++j){
      Vector mecPos = (*j)->GetObject<MobilityModel> ()->GetPosition ();
      Vector enbPos = enbmmWaveDevs.Get (i)->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
      
      double distance = CalculateDistance (mecPos, enbPos);

      if (distance < minDistance)
        {
          minDistance = distance;
          closestMECIndex = (*j)->GetId();
        }
     }
     mgbDevices = p2pmgb.Install (enbmmWaveDevs.Get(i)->GetNode(), MECnodes.Get(closestMECIndex-numUe-numEnb-3));
     mgbIpIfaces = mgbipv4h.Assign (mgbDevices);

  }
  std::cout<<"find the MEC neighbor"<<std::endl;
  //find MEC neighbor (for Inter MEC communication)
  // find the closest MEC

  PointToPointHelper p2pImc;
  
  p2pImc.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2pImc.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2pImc.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  
  
  Ipv4AddressHelper Imcipv4h;
  Imcipv4h.SetBase ("50.0.0.0", "255.0.0.0");
  NetDeviceContainer ImcDevices;
  Ipv4InterfaceContainer ImcIpIfaces; 
  for (NodeContainer::Iterator i = MECnodes.Begin(); i != MECnodes.End (); ++i) {

    for (NodeContainer::Iterator j = i+1; j != MECnodes.End (); ++j) {
      ImcDevices = p2pImc.Install ((*i),*(j));
      ImcIpIfaces = Imcipv4h.Assign (ImcDevices);
      
     }
     

  }
  
  // Install and start applications on UEs and remote host

  uint16_t ulPort = 2000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;


  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
     
          ++ulPort;
          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          //ulPacketSinkHelper.SetAttribute ("PacketWindowSize", UintegerValue (256));
          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
          UdpClientHelper ulClient (remoteHostAddr, ulPort);
          ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds (interPacketInterval)));
          ulClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
          ulClient.SetAttribute ("PacketSize", UintegerValue (25));
          clientApps.Add (ulClient.Install (ueNodes.Get (u)));
        
    }

  // Start applications
  //NS_LOG_UNCOND ("transientDuration " << transientDuration << " simTime " << simTime);
  serverApps.Start (Seconds (1));
  clientApps.Start (Seconds (1));
  //clientApps.Stop (Seconds (simTime - 1));
  
  
  
  // Install and start applications on UEs and remote host
  

  
  //mmwaveHelper->EnableTraces ();
  // Uncomment to enable PCAP tracing
  //p2ph.EnablePcapAll ("mmwave-epc-simple");
    std::cout<<"anim"<<std::endl;
    
    
  AnimationInterface anim("mmwave_0509.xml");
  for(uint32_t i=0; i<numUe; i++){
    anim.UpdateNodeColor(i, 0,255,0);
  }
  
  for(uint32_t i=0; i<numEnb; i++){
    Ptr<Node> enb = enbNodes.Get(i);
    anim.UpdateNodeColor(enb, 0,0,255);
  }

  Simulator::Stop (Seconds (simTime));
  
  
  Simulator::Run ();

  /*GtkConfigStore config;
  config.ConfigureAttributes();*/

  Simulator::Destroy ();
  return 0;

}
