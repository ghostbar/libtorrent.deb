// libTorrent - BitTorrent library
// Copyright (C) 2005-2007, Jari Sundell
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// In addition, as a special exception, the copyright holders give
// permission to link the code of portions of this program with the
// OpenSSL library under certain conditions as described in each
// individual source file, and distribute linked combinations
// including the two.
//
// You must obey the GNU General Public License in all respects for
// all of the code used other than OpenSSL.  If you modify file(s)
// with this exception, you may extend this exception to your version
// of the file(s), but you are not obligated to do so.  If you do not
// wish to do so, delete this exception statement from your version.
// If you delete this exception statement from all source files in the
// program, then also delete it here.
//
// Contact:  Jari Sundell <jaris@ifi.uio.no>
//
//           Skomakerveien 33
//           3185 Skoppum, NORWAY

// Add some helpfull words here.

#ifndef LIBTORRENT_CONNECTION_MANAGER_H
#define LIBTORRENT_CONNECTION_MANAGER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <sigc++/connection.h>
#include <sigc++/signal.h>
#include <sigc++/functors/slot.h>

#include <torrent/common.h>

namespace torrent {

// Standard pair of up/down throttles.
// First element is upload throttle, second element is download throttle.
typedef std::pair<Throttle*, Throttle*> ThrottlePair;

class LIBTORRENT_EXPORT ConnectionManager {
public:
  typedef uint32_t                              size_type;
  typedef uint16_t                              port_type;
  typedef uint8_t                               priority_type;
  typedef sigc::slot<uint32_t, const sockaddr*> slot_filter_type;

  static const priority_type iptos_default     = 0;
  static const priority_type iptos_lowdelay    = IPTOS_LOWDELAY;
  static const priority_type iptos_throughput  = IPTOS_THROUGHPUT;
  static const priority_type iptos_reliability = IPTOS_RELIABILITY;

#if defined IPTOS_MINCOST
  static const priority_type iptos_mincost     = IPTOS_MINCOST;
#elif defined IPTOS_LOWCOST
  static const priority_type iptos_mincost     = IPTOS_LOWCOST;
#else
  static const priority_type iptos_mincost     = iptos_throughput;
#endif

  static const uint32_t encryption_none             = 0;
  static const uint32_t encryption_allow_incoming   = (1 << 0);
  static const uint32_t encryption_try_outgoing     = (1 << 1);
  static const uint32_t encryption_require          = (1 << 2);
  static const uint32_t encryption_require_RC4      = (1 << 3);
  static const uint32_t encryption_enable_retry     = (1 << 4);
  static const uint32_t encryption_prefer_plaintext = (1 << 5);

  // Internal to libtorrent.
  static const uint32_t encryption_use_proxy        = (1 << 6);
  static const uint32_t encryption_dont_filter_recent = (1 << 7);

  enum {
    handshake_incoming           = 1,
    handshake_outgoing           = 2,
    handshake_outgoing_encrypted = 3,
    handshake_outgoing_proxy     = 4,
    handshake_success            = 5,
    handshake_dropped            = 6,
    handshake_failed             = 7,
    handshake_retry_plaintext    = 8,
    handshake_retry_encrypted    = 9
  };

  // The sockaddr argument in the result slot call is NULL if the resolve failed, and the int holds the errno.
  typedef sigc::slot2<void, const sockaddr*, int>                                                   slot_resolver_result_type;
  typedef sigc::slot4<slot_resolver_result_type*, const char*, int, int, slot_resolver_result_type> slot_resolver_type;

  typedef sigc::slot4<void, const sockaddr*, int, int, const HashString*>   slot_handshake_type;
  typedef sigc::signal4<void, const sockaddr*, int, int, const HashString*> signal_handshake_type;

  typedef sigc::slot1<ThrottlePair, const sockaddr*>  slot_address_throttle_type;

  ConnectionManager();
  ~ConnectionManager();
  
  // Check that we have not surpassed the max number of open sockets
  // and that we're allowed to connect to the socket address.
  //
  // Consider only checking max number of open sockets.
  bool                can_connect() const;

  // Call this to keep the socket count up to date.
  void                inc_socket_count()                      { m_size++; }
  void                dec_socket_count()                      { m_size--; }

  size_type           size() const                            { return m_size; }

  size_type           max_size() const                        { return m_maxSize; }
  void                set_max_size(size_type s)               { m_maxSize = s; }

  priority_type       priority() const                        { return m_priority; }
  void                set_priority(priority_type p)           { m_priority = p; }

  uint32_t            send_buffer_size() const                { return m_sendBufferSize; }
  void                set_send_buffer_size(uint32_t s);

  uint32_t            receive_buffer_size() const             { return m_receiveBufferSize; }
  void                set_receive_buffer_size(uint32_t s);

  uint32_t            encryption_options()                    { return m_encryptionOptions; }
  void                set_encryption_options(uint32_t options); 

  // Setting the addresses creates a copy of the address.
  const sockaddr*     bind_address() const                    { return m_bindAddress; }
  void                set_bind_address(const sockaddr* sa);

  const sockaddr*     local_address() const                   { return m_localAddress; }
  void                set_local_address(const sockaddr* sa);

  const sockaddr*     proxy_address() const                   { return m_proxyAddress; }
  void                set_proxy_address(const sockaddr* sa);

  uint32_t            filter(const sockaddr* sa);
  void                set_filter(const slot_filter_type& s)   { m_slotFilter = s; }

  bool                listen_open(port_type begin, port_type end);
  void                listen_close();  

  signal_handshake_type& signal_handshake_log()                          { return m_signalHandshakeLog; }
  sigc::connection       set_signal_handshake_log(slot_handshake_type s) { return m_signalHandshakeLog.connect(s); }

  // The resolver returns a pointer to its copy of the result slot
  // which the caller may set blocked to prevent the slot from being
  // called. The pointer must be NULL if the result slot was already
  // called because the resolve was synchronous.
  const slot_resolver_type& resolver() const                             { return m_slotResolver; }
  void                      set_resolver(const slot_resolver_type& s)    { m_slotResolver = s; }

  // Since trackers need our port number, it doesn't get cleared after
  // 'listen_close()'. The client may change the reported port number,
  // but do note that it gets overwritten after 'listen_open(...)'.
  port_type           listen_port() const                     { return m_listenPort; }
  void                set_listen_port(port_type p)            { m_listenPort = p; }

  // The slot returns a ThrottlePair to use for the given address, or NULLs to use the default throttle.
  const slot_address_throttle_type&
                      address_throttle() const                { return m_slotAddressThrottle; }
  void                set_address_throttle(const slot_address_throttle_type& s)
                                                              { m_slotAddressThrottle = s; }

  // For internal usage.
  Listen*             listen()                                { return m_listen; }

private:
  ConnectionManager(const ConnectionManager&);
  void operator = (const ConnectionManager&);

  size_type           m_size;
  size_type           m_maxSize;

  priority_type       m_priority;
  uint32_t            m_sendBufferSize;
  uint32_t            m_receiveBufferSize;
  int                 m_encryptionOptions;

  sockaddr*           m_bindAddress;
  sockaddr*           m_localAddress;
  sockaddr*           m_proxyAddress;

  Listen*             m_listen;
  port_type           m_listenPort;

  slot_filter_type      m_slotFilter;
  signal_handshake_type m_signalHandshakeLog;
  slot_resolver_type    m_slotResolver;
  slot_address_throttle_type  m_slotAddressThrottle;
};

}

#endif

