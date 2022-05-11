/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 */
#include "v2p-udp-client-server-helper.h"
#include "ns3/v2p-udp-server.h"
#include "ns3/v2p-udp-client.h"
#include "ns3/v2p-udp-trace-client.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

namespace ns3 {

V2PUdpServerHelper::V2PUdpServerHelper ()
{
  m_factory.SetTypeId (V2PUdpServer::GetTypeId ());
}

V2PUdpServerHelper::V2PUdpServerHelper (uint16_t port)
{
  m_factory.SetTypeId (V2PUdpServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void
V2PUdpServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
V2PUdpServerHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;

      m_server = m_factory.Create<V2PUdpServer> ();
      node->AddApplication (m_server);
      apps.Add (m_server);

    }
  return apps;
}

Ptr<V2PUdpServer>
V2PUdpServerHelper::GetServer (void)
{
  return m_server;
}

V2PUdpClientHelper::V2PUdpClientHelper ()
{
  m_factory.SetTypeId (V2PUdpClient::GetTypeId ());
}

V2PUdpClientHelper::V2PUdpClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (V2PUdpClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

V2PUdpClientHelper::V2PUdpClientHelper (Address address)
{
  m_factory.SetTypeId (V2PUdpClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void
V2PUdpClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

//Add SetFill method

void
V2PUdpClientHelper::SetFill (Ptr<Application> apps, std::string fill)
{
  apps->GetObject<V2PUdpClient>()->SetFill (fill);
}

void
V2PUdpClientHelper::SetFill (Ptr<Application> apps, uint8_t fill, uint32_t dataLength)
{
  apps->GetObject<V2PUdpClient>()->SetFill (fill, dataLength);
}

void
V2PUdpClientHelper::SetFill (Ptr<Application> apps, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  apps->GetObject<V2PUdpClient>()->SetFill (fill, fillLength, dataLength);
}

void 
V2PUdpClientHelper::SetInterval (Ptr<Application> apps, ns3::Time newInterval)
{
  apps->GetObject<V2PUdpClient>()->SetInterval (newInterval);
}

void 
V2PUdpClientHelper::Send(Ptr<Application> apps)
{
  apps->GetObject<V2PUdpClient>()->Send();
}

double
V2PUdpClientHelper::GetInterval (Ptr<Application> apps)
{
  double interval;
  interval = apps->GetObject<V2PUdpClient>()->GetInterval ();
  return interval;
}

void 
V2PUdpClientHelper::StartAgain (Ptr<Application> apps)
{
  apps->GetObject<V2PUdpClient>()->StartAgain ();
}


ApplicationContainer
V2PUdpClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<V2PUdpClient> client = m_factory.Create<V2PUdpClient> ();
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

V2PUdpTraceClientHelper::V2PUdpTraceClientHelper ()
{
  m_factory.SetTypeId (V2PUdpTraceClient::GetTypeId ());
}

V2PUdpTraceClientHelper::V2PUdpTraceClientHelper (Address address, uint16_t port, std::string filename)
{
  m_factory.SetTypeId (V2PUdpTraceClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
  SetAttribute ("TraceFilename", StringValue (filename));
}

V2PUdpTraceClientHelper::V2PUdpTraceClientHelper (Address address, std::string filename)
{
  m_factory.SetTypeId (V2PUdpTraceClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("TraceFilename", StringValue (filename));
}

void
V2PUdpTraceClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
V2PUdpTraceClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<V2PUdpTraceClient> client = m_factory.Create<V2PUdpTraceClient> ();
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

} // namespace ns3
