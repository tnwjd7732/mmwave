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
#ifndef V2P_UDP_CLIENT_SERVER_HELPER_H
#define V2P_UDP_CLIENT_SERVER_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/v2p-udp-server.h"
#include "ns3/v2p-udp-client.h"
namespace ns3 {
/**
 * \ingroup udpclientserver
 * \brief Create a server application which waits for input UDP packets
 *        and uses the information carried into their payload to compute
 *        delay and to determine if some packets are lost.
 */
class V2PUdpServerHelper
{
public:
  /**
   * Create V2PUdpServerHelper which will make life easier for people trying
   * to set up simulations with udp-client-server application.
   *
   */
  V2PUdpServerHelper ();

  /**
   * Create UdpServerHelper which will make life easier for people trying
   * to set up simulations with udp-client-server application.
   *
   * \param port The port the server will wait on for incoming packets
   */
  V2PUdpServerHelper (uint16_t port);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Create one UDP server application on each of the Nodes in the
   * NodeContainer.
   *
   * \param c The nodes on which to create the Applications.  The nodes
   *          are specified by a NodeContainer.
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (NodeContainer c);

  /**
   * \brief Return the last created server.
   *
   * This function is mainly used for testing.
   *
   * \returns a Ptr to the last created server application
   */
  Ptr<V2PUdpServer> GetServer (void);
private:
  ObjectFactory m_factory; //!< Object factory.
  Ptr<V2PUdpServer> m_server; //!< The last created server application
};

/**
 * \ingroup udpclientserver
 * \brief Create a client application which sends UDP packets carrying
 *  a 32bit sequence number and a 64 bit time stamp.
 *
 */
class V2PUdpClientHelper
{

public:
  /**
   * Create UdpClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server.
   *
   */
  V2PUdpClientHelper ();

  /**
   *  Create UdpClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server. Use this variant with
   * addresses that do not include a port value (e.g., Ipv4Address and
   * Ipv6Address).
   *
   * \param ip The IP address of the remote UDP server
   * \param port The port number of the remote UDP server
   */

  V2PUdpClientHelper (Address ip, uint16_t port);
  /**
   *  Create UdpClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server. Use this variant with
   * addresses that do include a port value (e.g., InetSocketAddress and
   * Inet6SocketAddress).
   *
   * \param addr The address of the remote UDP server
   */

  V2PUdpClientHelper (Address addr);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);


//add setfill method


/**
   * Given a pointer to a UdpEchoClient application, set the data fill of the 
   * packet (what is sent as data to the server) to the contents of the fill
   * string (including the trailing zero terminator).
   *
   * \warning The size of resulting echo packets will be automatically adjusted
   * to reflect the size of the fill string -- this means that the PacketSize
   * attribute may be changed as a result of this call.
   *
   * \param app Smart pointer to the application (real type must be UdpEchoClient).
   * \param fill The string to use as the actual echo data bytes.
   */
  void SetFill (Ptr<Application> app, std::string fill);

  /**
   * Given a pointer to a UdpEchoClient application, set the data fill of the 
   * packet (what is sent as data to the server) to the contents of the fill
   * byte.
   *
   * The fill byte will be used to initialize the contents of the data packet.
   * 
   * \warning The size of resulting echo packets will be automatically adjusted
   * to reflect the dataLength parameter -- this means that the PacketSize
   * attribute may be changed as a result of this call.
   *
   * \param app Smart pointer to the application (real type must be UdpEchoClient).
   * \param fill The byte to be repeated in constructing the packet data..
   * \param dataLength The desired length of the resulting echo packet data.
   */
  void SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength);

  /**
   * Given a pointer to a UdpEchoClient application, set the data fill of the 
   * packet (what is sent as data to the server) to the contents of the fill
   * buffer, repeated as many times as is required.
   *
   * Initializing the fill to the contents of a single buffer is accomplished
   * by providing a complete buffer with fillLength set to your desired 
   * dataLength
   *
   * \warning The size of resulting echo packets will be automatically adjusted
   * to reflect the dataLength parameter -- this means that the PacketSize
   * attribute of the Application may be changed as a result of this call.
   *
   * \param app Smart pointer to the application (real type must be UdpEchoClient).
   * \param fill The fill pattern to use when constructing packets.
   * \param fillLength The number of bytes in the provided fill pattern.
   * \param dataLength The desired length of the final echo data.
   */
  void SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength);
  void SetInterval (Ptr<Application> apps, ns3::Time newInterval);
  void StartAgain (Ptr<Application> apps);
  double GetInterval (Ptr<Application> apps);
  void Send(Ptr<Application> apps);

  /**
     * \param c the nodes
     *
     * Create one UDP client application on each of the input nodes
     *
     * \returns the applications created, one application per input node.
     */
  ApplicationContainer Install (NodeContainer c);

private:
  ObjectFactory m_factory; //!< Object factory.
};
/**
 * \ingroup udpclientserver
 * Create UdpTraceClient application which sends UDP packets based on a trace
 * file of an MPEG4 stream. Trace files could be downloaded form :
 * http://www2.tkn.tu-berlin.de/research/trace/ltvt.html (the 2 first lines of
 * the file should be removed)
 * A valid trace file is a file with 4 columns:
 * \li -1- the first one represents the frame index
 * \li -2- the second one indicates the type of the frame: I, P or B
 * \li -3- the third one indicates the time on which the frame was generated by the encoder
 * \li -4- the fourth one indicates the frame size in byte
*/
class V2PUdpTraceClientHelper
{
public:
  /**
   * Create UdpTraceClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server.
   *
   */
  V2PUdpTraceClientHelper ();

  /**
   * Create UdpTraceClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server. Use this variant with
   * addresses that do not include a port value (e.g., Ipv4Address and
   * Ipv6Address).
   *
   * \param ip The IP address of the remote UDP server
   * \param port The port number of the remote UDP server
   * \param filename the file from which packet traces will be loaded
   */
  V2PUdpTraceClientHelper (Address ip, uint16_t port, std::string filename);
  /**
   * Create UdpTraceClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server. Use this variant with
   * addresses that do include a port value (e.g., InetSocketAddress and
   * Inet6SocketAddress).
   *
   * \param addr The address of the remote UDP server
   * \param filename the file from which packet traces will be loaded
   */
  V2PUdpTraceClientHelper (Address addr, std::string filename);

  /**
    * Record an attribute to be set in each Application after it is is created.
    *
    * \param name the name of the attribute to set
    * \param value the value of the attribute to set
    */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
    * \param c the nodes
    *
    * Create one UDP trace client application on each of the input nodes
    *
    * \returns the applications created, one application per input node.
    */
  ApplicationContainer Install (NodeContainer c);

private:
  ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* UDP_CLIENT_SERVER_H */
