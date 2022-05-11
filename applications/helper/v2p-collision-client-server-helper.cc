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
#include "v2p-collision-client-server-helper.h"
#include "ns3/v2p-collision-server.h"
#include "ns3/v2p-collision-client.h"
#include "ns3/v2p-collision-trace-client.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

namespace ns3 {

V2PCollisionClientHelper::V2PCollisionClientHelper ()
{
  m_factory.SetTypeId (V2PCollisionClient::GetTypeId ());
}

V2PCollisionClientHelper::V2PCollisionClientHelper(Address address, uint16_t port)
{
  m_factory.SetTypeId (V2PCollisionClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

V2PCollisionClientHelper::V2PCollisionClientHelper (Address address)
{
  m_factory.SetTypeId (V2PCollisionClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void
V2PCollisionClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

//Add SetFill method

void
V2PCollisionClientHelper::SetFill (Ptr<Application> apps, std::string fill)
{
  apps->GetObject<V2PCollisionClient>()->SetFill (fill);
}

void
V2PCollisionClientHelper::SetFill (Ptr<Application> apps, uint8_t fill, uint32_t dataLength)
{
  apps->GetObject<V2PCollisionClient>()->SetFill (fill, dataLength);
}

void
V2PCollisionClientHelper::SetFill (Ptr<Application> apps, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  apps->GetObject<V2PCollisionClient>()->SetFill (fill, fillLength, dataLength);
}


void 
V2PCollisionClientHelper::Send(Ptr<Application> apps)
{
  apps->GetObject<V2PCollisionClient>()->Send();
}



ApplicationContainer
V2PCollisionClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<V2PCollisionClient> client = m_factory.Create<V2PCollisionClient> ();
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}


V2PCollisionServerHelper::V2PCollisionServerHelper ()
{
  m_factory.SetTypeId (V2PCollisionServer::GetTypeId ());
}

V2PCollisionServerHelper::V2PCollisionServerHelper (uint16_t port)
{
  m_factory.SetTypeId (V2PCollisionServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void
V2PCollisionServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
V2PCollisionServerHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;

      m_server = m_factory.Create<V2PCollisionServer> ();
      node->AddApplication (m_server);
      apps.Add (m_server);

    }
  return apps;
}

Ptr<V2PCollisionServer>
V2PCollisionServerHelper::GetServer (void)
{
  return m_server;
}


V2PCollisionTraceClientHelper::V2PCollisionTraceClientHelper ()
{
  m_factory.SetTypeId (V2PCollisionTraceClient::GetTypeId ());
}

V2PCollisionTraceClientHelper::V2PCollisionTraceClientHelper (Address address, uint16_t port, std::string filename)
{
  m_factory.SetTypeId (V2PCollisionTraceClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
  SetAttribute ("TraceFilename", StringValue (filename));
}

V2PCollisionTraceClientHelper::V2PCollisionTraceClientHelper (Address address, std::string filename)
{
  m_factory.SetTypeId (V2PCollisionTraceClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("TraceFilename", StringValue (filename));
}

void
V2PCollisionTraceClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
V2PCollisionTraceClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<V2PCollisionTraceClient> client = m_factory.Create<V2PCollisionTraceClient> ();
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

} // namespace ns3
