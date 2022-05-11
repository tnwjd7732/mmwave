/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008,2009 INRIA, UDCAST
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */


#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <cmath>


#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "packet-loss-counter.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"

#include "seq-ts-header.h"
#include "v2p-udp-server.h"

#include <boost/lexical_cast.hpp>

namespace ns3 {

//=====server DB================================
std::map <int32_t, std::vector<double>> mecDbPed;
std::map <int32_t, std::vector<double>> mecDbVeh;
std::map<int32_t, std::vector<double>>::iterator it;

uint32_t arr[26]={0};
uint32_t cnt = 0;
double sum=0, avg=0;

uint64_t packetcount=0;

//================================================

NS_LOG_COMPONENT_DEFINE ("V2PUdpServer");

NS_OBJECT_ENSURE_REGISTERED (V2PUdpServer);


TypeId
V2PUdpServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::V2PUdpServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<V2PUdpServer> ()
    .AddAttribute ("Port",
                   "Port on which we listen for incoming packets.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&V2PUdpServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketWindowSize",
                   "The size of the window used to compute the packet loss. This value should be a multiple of 8.",
                   UintegerValue (32),
                   MakeUintegerAccessor (&V2PUdpServer::GetPacketWindowSize,
                                         &V2PUdpServer::SetPacketWindowSize),
                   MakeUintegerChecker<uint16_t> (8,256))
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&V2PUdpServer::m_rxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&V2PUdpServer::m_rxTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
  ;
  return tid;
}

V2PUdpServer::V2PUdpServer ()
  : m_lossCounter (0)
{
  NS_LOG_FUNCTION (this);
  m_received=0;
}

V2PUdpServer::~V2PUdpServer ()
{
  NS_LOG_FUNCTION (this);
}

void V2PUdpServer::SetStartOrStop (uint16_t num)
{
  if(num==0){
    StopApplication();
  }
  else if(num==1){
    StartApplication();
  }


}

uint16_t
V2PUdpServer::GetPacketWindowSize () const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetBitMapSize ();
}

void
V2PUdpServer::SetPacketWindowSize (uint16_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_lossCounter.SetBitMapSize (size);
}

uint32_t
V2PUdpServer::GetLost (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetLost ();
}

uint64_t
V2PUdpServer::GetReceived (void) const
{
  NS_LOG_FUNCTION (this);
  return m_received;
}

void
V2PUdpServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
V2PUdpServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  std::cout<<"server start application"<<std::endl;
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),
                                                   m_port);
      if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&V2PUdpServer::HandleRead, this));

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local = Inet6SocketAddress (Ipv6Address::GetAny (),
                                                   m_port);
      if (m_socket6->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
    }

  m_socket6->SetRecvCallback (MakeCallback (&V2PUdpServer::HandleRead, this));

}

void
V2PUdpServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

//========================new=====================


double 
V2PUdpServer::CalculateAnglePoint(double x1, double x2, double cx, double y1, double y2, double cy)
{
  double angle;
  //angle=atan((y2-cy)/(x2-cx))-atan((y1-cy)-(x1-cx));
  double angle1=atan((y1-cy)/(x1-cx));
  double angle2=atan((y2-cy)/(x2-cx));
  angle=abs((angle1-angle2));
  if(angle1==0 && angle2==0){
    if(x1<cx && cx<x2) 
      angle=M_PI;
    else if(y1<cy && cy<y2)
      angle=M_PI;
    else if(x1>cx && cx>x2) 
      angle=M_PI;
    else if(y1>cy && cy>y2)
      angle=M_PI;

  }
 
  //angle은 x-c 선과 y-c선을 잇는 사이각
  //라디안 값이므로 출력할 땐 180/파이를 곱해야 함
  return angle;
}

double V2PUdpServer::cot(double i)
{
  return (1/tan(i));
}

void 
V2PUdpServer::CollisionPredictionAlgorithm (uint32_t pedNodeID, double pos_x, double pos_y, double velo_x, double velo_y, double angle)
{

/*
  Ptr<Node> veh = this->GetNode();
  Ptr<MobilityModel> vehmobility=veh->GetObject<MobilityModel>();  //get mobility model
  uint32_t ID=veh->GetId();

  //std::cout<<"[충돌예측알고리즘] "<<ID<<"vehicle과 "<<pedNodeID<<"번 보행자의 충돌 가능성 "<<std::endl;
  Vector vehvelocity = vehmobility -> GetVelocity(); //get Velocity vector
  Vector vehposition = vehmobility -> GetPosition(); //get Position vector
  double vehposx=vehposition.x;
  double vehposy=vehposition.y;
  double vehvelo=sqrt((vehvelocity.x*vehvelocity.x)+(vehvelocity.y*vehvelocity.y));
  double pedvelo=sqrt((velo_x*velo_x)+(velo_y*velo_y));
  
  double vehnextx, vehnexty, pednextx, pednexty;
  vehnextx=vehposx+vehvelocity.x*0.1;
  vehnexty=vehposy+vehvelocity.y*0.1;
  pednextx=pos_x+velo_x*0.1;
  pednexty=pos_y+velo_y*0.1;
  
  //std::cout<<"자동차 현재 위치("<<vehposx<<","<<vehposy<<" 보행자 현재 위치("<<pos_x<<","<<pos_y<<std::endl;
  
  //vehvelo*=3.6; // m/s to km/h
  //velo*=3.6;
  double vehangle;
  const double RadtoDeg = 57.2957951; //180/pi
  double Tbrake=1.65;
  const double Treact=0.85;
  double Vappr, Alpha, Beta;
  double Tcollcar, Tcollped, Scollcar, Scollped;
  double Tcollmin, D0;
  //std::cout<<"vehicle 속도벡터 x="<<vehvelocity.x<<" vehicle 속도벡터 y="<<vehvelocity.y<<std::endl;
  if(vehvelocity.y==0 && vehvelocity.x==0)
     vehangle=0;
  else
     vehangle = atan2(vehvelocity.y, vehvelocity.x); //degree
     
  
  if(angle ==0 || angle*RadtoDeg == 90 || angle*RadtoDeg == 180 ||vehangle*RadtoDeg==0 || vehangle*RadtoDeg==90||vehangle*RadtoDeg==180){
    Alpha=CalculateAnglePoint(vehnextx, pos_x, vehposx, vehnexty, pos_y, vehposy);
    Beta=CalculateAnglePoint(pednextx, vehposx, pos_x, pednexty, vehposy, pos_y);
    


    //std::cout<<"alpah(veh 중심):"<<Alpha<<"Beta(보행자 중심) :"<<Beta<<std::endl;
    Vappr=vehvelo * cos(Alpha) + pedvelo * cos(Beta);
    //std::cout<<"approaching V ="<<Vappr<<std::endl;
    
    if(Vappr<0) return;
    
  } 
  
  else{
    Alpha=CalculateAnglePoint(vehnextx, pos_x, vehposx, vehnexty, pos_y, vehposy);
    Beta=CalculateAnglePoint(pednextx, vehposx, pos_x, pednexty, vehposy, pos_y);

    //std::cout<<"alpah(veh 중심):"<<Alpha<<"Beta(보행자 중심) :"<<Beta<<std::endl;
    Vappr=vehvelo * cos(Alpha) + pedvelo * cos(Beta);
    //std::cout<<"approaching V ="<<Vappr<<std::endl;
  
    //std::cout<<"angle veh:"<<vehangle<<" angle ped:"<<angle<<std::endl;
    double collx, colly;
  
    if(Vappr<0){
      Vappr*=-1;
    }
    if(Vappr>=0) {
      collx=(vehposy-pos_y)-(vehposx*tan(vehangle)-pos_x*tan(angle))/(tan(angle)-tan(vehangle));
      colly=(vehposx-pos_x)-(vehposy*cot(vehangle)-pos_y*cot(angle))/(cot(angle)-cot(vehangle));    

      //std::cout<<"collx:"<<collx<<"colly:"<<colly<<std::endl;
    }
    else
      return;

    Scollcar=sqrt((collx-vehposx)*(collx-vehposx)+(colly-vehposy)*(colly-vehposy));
    Scollped=sqrt((collx-pos_x)*(collx-pos_x)+(colly-pos_y)*(colly-pos_y));
    Tcollcar=Scollcar/vehvelo;
    Tcollped=Scollped/pedvelo;


    if((Tcollcar - Tcollped < Treact+Tbrake) && (Tcollcar-Tcollped>0)) {
    //Step 5-1. 위험 상황! 경고 보내고 함수 종료
      //std::cout<<"남은 충돌시간(차량): "<<Tcollcar<<"남은 충돌시간(보행자): "<<Tcollped<<std::endl;    
     //std::cout<<"TTC(좌표) Warning! Warning Time: "<<Simulator::Now().GetSeconds()<<std::endl;
      return;
    }    
  }
   

  D0=sqrt((vehposx-pos_x)*(vehposx-pos_x)+(vehposy-pos_y)*(vehposy-pos_y));
  Tcollmin=D0/Vappr;
  
  if(Tcollmin<=Treact){ //Level 5. Imminent Collision Zone
   std::cout<<"Tcollmin Warning! Level 5 - Warning Time: "<<Simulator::Now().GetSeconds()<<"Tcollmin="<<Tcollmin<<std::endl;
    std::cout<<"(My)veh ID: "<<ID<<" ped ID: "<<pedNodeID<<std::endl;
  }
  else if(Treact < Tcollmin && Tcollmin <Tbrake+Treact){ //Level 4. Danger Zone
    std::cout<<"Tcollmin Warning! Level 4 - Warning Time: "<<Simulator::Now().GetSeconds()<<"Tcollmin="<<Tcollmin<<std::endl;
        std::cout<<"(My)veh ID: "<<ID<<" ped ID: "<<pedNodeID<<std::endl;
  }
  else if(Treact+Tbrake<=Tcollmin && Tcollmin<Treact+Tbrake+5){//Level 3. Immediately brake Zone
    
        if(arr[pedNodeID]==0 && pedNodeID!=0){
                std::cout<<"Tcollmin Warning! Level 3 - Warning Time: "<<Simulator::Now().GetSeconds()<<"Tcollmin="<<Tcollmin<<std::endl;
                std::cout<<"(My)veh ID: "<<ID<<" ped ID: "<<pedNodeID<<std::endl;
                std::cout<<pedNodeID<<"add done"<<std::endl;
                arr[pedNodeID]=1;
                cnt++;
                sum+=Simulator::Now().GetSeconds();
                if(cnt==25 || cnt==3){
                        //avg=sum/25;
                        avg=sum/3;
                        std::cout<<"avg first warning = "<<avg<<std::endl;
                        }
        }
  }
  else if(Treact+Tbrake+5 <Tcollmin && Tcollmin<=10){ //Level 2. Resolution advisory zone
    if(Vappr>0){//Send Warning message
     //std::cout<<"Tcollmin Warning! Level 2 - Warning Time: "<<Simulator::Now().GetSeconds()<<"Tcollmin="<<Tcollmin<<std::endl;
    }
  }
  else if(10<Tcollmin && Tcollmin<20){//Level 1. Traffic advisory zone
    //std::cout<<"No Warning! Level 1 "<<"Tcollmin="<<Tcollmin<<std::endl;
  }
  else { //No dangerous situation - No WARNING
   // std::cout<<"NO DANGEROUS SITUATION "<<"Tcollmin="<<Tcollmin<<std::endl;
  }
*/

}

void V2PUdpServer::HandleRead (Ptr<Socket> socket)
{

  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
         SeqTsHeader seqTs;
         packet->RemoveHeader (seqTs);
          
          int len=packet->GetSize();
         /* uint8_t *buffer;
          


         buffer= new uint8_t [len];*/
         
         uint8_t *buffer = new uint8_t[len];
          packet->CopyData(buffer, len);
          
          
         
          double recvTime=Simulator::Now ().GetSeconds ();
          NS_LOG_INFO ("At time " << recvTime << "s server received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort () << "Delay: " << Simulator::Now () - seqTs.GetTs ());
                       
        
   
        // if(InetSocketAddress::ConvertFrom (from).GetIpv4()==(Ipv4Address("7.0.0.23"))){
       //    std::cout<<"GPS 72byte 수신완료"<<std::endl;
       //    return;
         
         
         
        // }
       //   NS_LOG_INFO("split 시작 ... ");

          std::string str=(char*)buffer; //uint8_t to string
                    
          std::istringstream ss(str);
          std::string stringBuffer;
          std::vector <std::string> elems;
          elems.clear();
          
                //checking split result
          while(getline(ss, stringBuffer, '/')){
                elems.push_back(stringBuffer);
             //   NS_LOG_INFO("data split! "<< stringBuffer);
          }
               
          int32_t nodeType = boost::lexical_cast<int32_t>(elems[0]);
          int32_t nodeID = boost::lexical_cast<int32_t>(elems[1]);
     
          //we don't store vehicle node's datas. only storing pedestrian's data 
          //so, if nodeID is under 3 or over 3+vehicleNum, we don't store data

          //node ID 0:pgw, 1: remoteHost, 2:pgw, 3~3+vehicleNum-1: vehicleNode
          //3+vehicleNum~3+vehicleNum+pedestrianNum-1: pedestrianNode             
          
          
          //sender: MEC server -> receiver: vehicle
          //Performing Collision Prediction Algorithm with received data
          
          
          //if (InetSocketAddress::ConvertFrom (from).GetIpv4 ()==(Ipv4Address("20.1.1.1"))) {
          
           //     double pos_x, pos_y, velo_x, velo_y, angle;
            //    int32_t pedNodeID;
                
            /*    for(uint32_t i=0; i<elems.size(); i+=6){
                         
                          pedNodeID = boost::lexical_cast<int32_t>(elems[i]);
                          pos_x = boost::lexical_cast<double>(elems[i+1]);
                          pos_y = boost::lexical_cast<double>(elems[i+2]);
                          velo_x = boost::lexical_cast<double>(elems[i+3]);
                          velo_y = boost::lexical_cast<double>(elems[i+4]);
                          angle = boost::lexical_cast<double>(elems[i+5]);
                          
                          CollisionPredictionAlgorithm(pedNodeID, pos_x, pos_y, velo_x, velo_y, angle);

                }*/

         // }         
           
         
          
          
          
       //   else{   


                 packetcount++;
                 double pos_x= boost::lexical_cast<double>(elems[2]);
                 double pos_y= boost::lexical_cast<double>(elems[3]);
                 double velo_x= boost::lexical_cast<double>(elems[4]);
                 double velo_y= boost::lexical_cast<double>(elems[5]);
                 double angle=boost::lexical_cast<double>(elems[6]);
       
                  //엣지 컴퓨팅 실험할때 enable!!
                  std::vector<double> vec;
                  vec.push_back(pos_x);     vec.push_back(pos_y); vec.push_back(velo_x);
                  vec.push_back(velo_y);    vec.push_back(angle);
                  

                  if(nodeType == 0){ //store in pedestrian DB
                 // std::cout<<"ped data get!"<<std::endl;
                    if (mecDbPed.find(nodeID)!=mecDbPed.end()){ //등록된 보행자: data update -> erase & insert
                      mecDbPed.erase(nodeID); //첫번째 element는 nodeID(mecDb에서 key)
                      std::pair<int32_t, std::vector<double>> p1=make_pair(nodeID, vec);
                      mecDbPed.insert(p1); //update
                      std::cout<<Simulator::Now().GetSeconds()<<"sec ped 데이터 업데이트 완료!"<<std::endl;
                    }
          
                    else { //등록되지 않은 보행자: insert
                      std::pair<int32_t, std::vector<double>> p2=make_pair(nodeID, vec);
                      mecDbPed.insert(p2); //update
                     std::cout<<Simulator::Now().GetSeconds()<<"sec 새로운 ped 데이터 추가 완료!"<<std::endl;
                    }

                  }
                  else { //store in vehicle DB
                   // std::cout<<"veh data get!"<<std::endl;
                   if (mecDbVeh.find(nodeID)!=mecDbVeh.end()){ //등록된 보행자: data update -> erase & insert
                      mecDbVeh.erase(nodeID); //첫번째 element는 nodeID(mecDb에서 key)
                      std::pair<int32_t, std::vector<double>> p1=make_pair(nodeID, vec);
                      mecDbVeh.insert(p1); //update
                      std::cout<<Simulator::Now().GetSeconds()<<"sec veh 데이터 업데이트 완료!"<<std::endl;
                    }
          
                    else { //등록되지 않은 보행자: insert
                      std::pair<int32_t, std::vector<double>> p2=make_pair(nodeID, vec);
                      mecDbVeh.insert(p2); //update
                     std::cout<<Simulator::Now().GetSeconds()<<"sec 새로운 veh 데이터 추가 완료!"<<std::endl;
                    }
                  
                  
                  }
      }
      
      
      else if (Inet6SocketAddress::IsMatchingType (from)) //Ipv6 address -> we dont use
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }

      packet->RemoveAllPacketTags ();
      packet->RemoveAllByteTags ();

    }
}
/*

void
V2PUdpServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
      socket->GetSockName (localAddress);
      m_rxTrace (packet);
      m_rxTraceWithAddresses (packet, from, localAddress);
      if (packet->GetSize () > 0)
        {
          uint32_t receivedSize = packet->GetSize ();
          SeqTsHeader seqTs;
          packet->RemoveHeader (seqTs);
          uint32_t currentSequenceNumber = seqTs.GetSeq ();
          if (InetSocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO ("TraceDelay: RX " << receivedSize <<
                           " bytes from "<< InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
                           
              std::cout<<"V2P Server Application:: Handle Read function call"<<std::endl;
              std::cout<<"src ip: "<<InetSocketAddress::ConvertFrom(from).GetIpv4 ()<<std::endl;
            }
          else if (Inet6SocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO ("TraceDelay: RX " << receivedSize <<
                           " bytes from "<< Inet6SocketAddress::ConvertFrom (from).GetIpv6 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
            }

          m_lossCounter.NotifyReceived (currentSequenceNumber);
          m_received++;
        }
    }
}*/

} // Namespace ns3
