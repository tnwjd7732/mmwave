/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "v2p-collision-client.h"
#include "ns3/v2p-collision-client-server-helper.h"
#include "seq-ts-header.h"
#include "ns3/timer.h"
#include "ns3/v2p-udp-server.h"
#include <string>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>


//==test
#include "ns3/epc-sgw-pgw-application.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("V2PCollisionClient");

NS_OBJECT_ENSURE_REGISTERED (V2PCollisionClient);

TypeId
V2PCollisionClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::V2PCollisionClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<V2PCollisionClient> ()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&V2PCollisionClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval",
                   "The time to wait between packets", TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&V2PCollisionClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress",
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&V2PCollisionClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&V2PCollisionClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize",
                   "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                   UintegerValue (1024),
                   MakeUintegerAccessor (&V2PCollisionClient::m_size),
                   MakeUintegerChecker<uint32_t> (12,65507))
  ;
  return tid;
}

V2PCollisionClient::V2PCollisionClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_totalTx = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0; //add
  m_dataSize = 0; //add
}

V2PCollisionClient::~V2PCollisionClient ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0; //add~
  
  //add
  delete [] m_data; 
  m_data = 0; 
  m_dataSize = 0;
}

void
V2PCollisionClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void
V2PCollisionClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
V2PCollisionClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
V2PCollisionClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }
    }

#ifdef NS3_LOG_ENABLE
  std::stringstream peerAddressStringStream;
  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv6Address::ConvertFrom (m_peerAddress);
    }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 ();
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 ();
    }
  m_peerAddressString = peerAddressStringStream.str();
#endif // NS3_LOG_ENABLE

  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_socket->SetAllowBroadcast (true);
  m_sendEvent = Simulator::Schedule (Seconds (0.1), &V2PCollisionClient::Send, this);
  CollisionPredict();
}

void
V2PCollisionClient::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}

//=================v2p client func. added=======================


void
V2PCollisionClient::CollisionPredict(void)
{
  Simulator::Cancel (m_sendEvent);
 uint32_t alert = 0;
    //access to MEC database for getting user's data

  std::map<int32_t, std::vector<double>>::iterator ped;
  std::map<int32_t, std::vector<double>>::iterator veh;
  std::string pedID, vehID;
  //if me == ped
  uint32_t gNbNum=2;
  uint32_t pedNum=4;
  uint32_t myID = m_peerPort - 1200 + gNbNum;
        
        if(myID < gNbNum + pedNum){
            for(veh=mecDbVeh.begin(); veh!=mecDbVeh.end(); veh++){
               if(veh->first == 6 && myID == 4){
                  pedID=std::to_string(myID);
                  vehID=std::to_string(veh->first);
                  alert = 1;
                  break;
               }
                
            
            }
        }
        else {
            for(ped=mecDbPed.begin(); ped!=mecDbPed.end(); ped++){
                if(ped->first == 4 && myID == 6){
                  vehID=std::to_string(myID);
                  pedID=std::to_string(ped->first);
                  alert = 1;
                  
                  break;
               }
           }
        }

        if(alert == 1){
            //Tx alert to each UE and Veh
            alert = 0;

            std::string sending;

            sending.append(pedID);
            sending.append("/");
            sending.append(vehID);
            //std::cout<<Simulator::Now().GetSeconds()<<"sending data (server->ue): "<<sending<<std::endl;

            SetFill(sending); 
            m_sendEvent = Simulator::Schedule (MilliSeconds(1), &V2PCollisionClient::Send, this);

     }
   Simulator::Schedule (Seconds(0.5), &V2PCollisionClient::CollisionPredict, this);
}





void 
V2PCollisionClient::SetDataSize (uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << dataSize);

  //
  // If the client is setting the echo packet data size this way, we infer
  // that she doesn't care about the contents of the packet at all, so 
  // neither will we.
  //
  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
  m_size = dataSize;
}

uint32_t 
V2PCollisionClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void 
V2PCollisionClient::SetFill (std::string fill)
{
  NS_LOG_FUNCTION (this << fill);

  uint32_t dataSize = fill.size () + 1;

  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memcpy (m_data, fill.c_str (), dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}



void 
V2PCollisionClient::SetFill (uint8_t fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memset (m_data, fill, dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}



void 
V2PCollisionClient::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << fillSize << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  if (fillSize >= dataSize)
    {
      memcpy (m_data, fill, dataSize);
      m_size = dataSize;
      return;
    }

  //
  // Do all but the final fill.
  //
  uint32_t filled = 0;
  while (filled + fillSize < dataSize)
    {
      memcpy (&m_data[filled], fill, fillSize);
      filled += fillSize;
    }

  //
  // Last fill may be partial
  //
  memcpy (&m_data[filled], fill, dataSize - filled);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}


void 
V2PCollisionClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &V2PCollisionClient::Send, this);
}


//==============================================================


void
V2PCollisionClient::Send (void)
{
  //std::cout<<"v2p collision client :: send() call"<<std::endl;
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());
 
  Ptr<Packet> p = Create<Packet> (m_data, m_dataSize); // 8+4 : the size of the seqTs header
   SeqTsHeader seqTs;
  seqTs.SetSeq (m_sent);
  p->AddHeader (seqTs);

  if ((m_socket->Send (p)) >= 0)
    {
      ++m_sent;
      m_totalTx += p->GetSize ();
#ifdef NS3_LOG_ENABLE
    NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
                                    << m_peerAddressString << " Uid: "
                                    << p->GetUid () << " Time: "
                                    << (Simulator::Now ()).As (Time::S));
#endif // NS3_LOG_ENABLE
    }
#ifdef NS3_LOG_ENABLE
  else
    {
      NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
                                          << m_peerAddressString);
    }
#endif // NS3_LOG_ENABLE

  if (m_sent < m_count)
    {
      //m_sendEvent = Simulator::Schedule (m_interval, &V2PCollisionClient::Send, this);
    }
}


uint64_t
V2PCollisionClient::GetTotalTx () const
{
  return m_totalTx;
}


} // Namespace ns3
