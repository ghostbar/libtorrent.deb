// libTorrent - BitTorrent library
// Copyright (C) 2005-2011, Jari Sundell
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

#ifndef LIBTORRENT_UTILS_LOG_H
#define LIBTORRENT_UTILS_LOG_H

#include <string>
#include <vector>
#include <tr1/array>
#include <tr1/functional>
#include <torrent/common.h>

namespace torrent {

// TODO: Add option_strings support.
enum {
  LOG_CRITICAL,
  LOG_ERROR,
  LOG_WARN,
  LOG_NOTICE,
  LOG_INFO,
  LOG_DEBUG,

  LOG_CONNECTION_CRITICAL,
  LOG_CONNECTION_ERROR,
  LOG_CONNECTION_WARN,
  LOG_CONNECTION_NOTICE,
  LOG_CONNECTION_INFO,
  LOG_CONNECTION_DEBUG,

  LOG_DHT_CRITICAL,
  LOG_DHT_ERROR,
  LOG_DHT_WARN,
  LOG_DHT_NOTICE,
  LOG_DHT_INFO,
  LOG_DHT_DEBUG,

  LOG_PEER_CRITICAL,
  LOG_PEER_ERROR,
  LOG_PEER_WARN,
  LOG_PEER_NOTICE,
  LOG_PEER_INFO,
  LOG_PEER_DEBUG,

  LOG_RPC_CRITICAL,
  LOG_RPC_ERROR,
  LOG_RPC_WARN,
  LOG_RPC_NOTICE,
  LOG_RPC_INFO,
  LOG_RPC_DEBUG,

  LOG_STORAGE_CRITICAL,
  LOG_STORAGE_ERROR,
  LOG_STORAGE_WARN,
  LOG_STORAGE_NOTICE,
  LOG_STORAGE_INFO,
  LOG_STORAGE_DEBUG,

  LOG_THREAD_CRITICAL,
  LOG_THREAD_ERROR,
  LOG_THREAD_WARN,
  LOG_THREAD_NOTICE,
  LOG_THREAD_INFO,
  LOG_THREAD_DEBUG,

  LOG_TRACKER_CRITICAL,
  LOG_TRACKER_ERROR,
  LOG_TRACKER_WARN,
  LOG_TRACKER_NOTICE,
  LOG_TRACKER_INFO,
  LOG_TRACKER_DEBUG,

  LOG_TORRENT_CRITICAL,
  LOG_TORRENT_ERROR,
  LOG_TORRENT_WARN,
  LOG_TORRENT_NOTICE,
  LOG_TORRENT_INFO,
  LOG_TORRENT_DEBUG,

  LOG_GROUP_MAX_SIZE
};

#define lt_log_print(log_group, ...) { if (torrent::log_groups[log_group].valid()) torrent::log_groups[log_group].internal_print(__VA_ARGS__); }
#define lt_log_print_info(log_group, log_info, ...) { if (torrent::log_groups[log_group].valid()) torrent::log_groups[log_group].internal_print_info(log_info, __VA_ARGS__); }
#define lt_log_is_valid(log_group) (torrent::log_groups[log_group].valid())

struct log_cached_outputs;
class DownloadInfo;

typedef std::tr1::function<void (const char*, unsigned int, int)> log_slot;
typedef std::vector<std::pair<std::string, log_slot> >            log_output_list;

class LIBTORRENT_EXPORT log_group {
public:
  log_group() : m_outputs(0), m_cached_outputs(0), m_first(NULL), m_last(NULL) {}

  bool                valid() const { return m_first != NULL; }
  bool                empty() const { return m_first == NULL; }

  size_t              size_outputs() const { return std::distance(m_first, m_last); }

  // Internal:
  void                internal_print(const char* fmt, ...);
  void                internal_print_info(const DownloadInfo* info, const char* fmt, ...);

  uint64_t            outputs() const                    { return m_outputs; }
  uint64_t            cached_outputs() const             { return m_cached_outputs; }

  void                clear_cached_outputs()             { m_cached_outputs = m_outputs; }

  void                set_outputs(uint64_t val)          { m_outputs = val; }
  void                set_cached_outputs(uint64_t val)   { m_cached_outputs = val; }

  void                set_cached(log_slot* f, log_slot* l) { m_first = f; m_last = l; }

private:
  uint64_t            m_outputs;
  uint64_t            m_cached_outputs;

  log_slot*           m_first;
  log_slot*           m_last;
};

typedef std::tr1::array<log_group, LOG_GROUP_MAX_SIZE> log_group_list;

extern log_group_list  log_groups LIBTORRENT_EXPORT;
extern log_output_list log_outputs LIBTORRENT_EXPORT;

// Called by torrent::initialize().
void log_initialize() LIBTORRENT_EXPORT;
void log_cleanup() LIBTORRENT_EXPORT;

void log_open_output(const char* name, log_slot slot) LIBTORRENT_EXPORT;
void log_close_output(const char* name) LIBTORRENT_EXPORT;

void log_open_file_output(const char* name, const char* filename) LIBTORRENT_EXPORT;

void log_add_group_output(int group, const char* name) LIBTORRENT_EXPORT;
void log_remove_group_output(int group, const char* name) LIBTORRENT_EXPORT;

void log_add_child(int group, int child) LIBTORRENT_EXPORT;
void log_remove_child(int group, int child) LIBTORRENT_EXPORT;

}

#endif
