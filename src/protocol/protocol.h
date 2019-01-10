/*  Copyright (C) 2014-2018 FastoGT. All right reserved.
    This file is part of iptv_cloud.
    iptv_cloud is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    iptv_cloud is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with iptv_cloud.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <map>

#include <common/libev/io_client.h>

#include "protocol/types.h"

namespace iptv_cloud {
namespace protocol {

typedef uint32_t protocoled_size_t;  // sizeof 4 byte
enum { MAX_COMMAND_SIZE = 1024 * 8 };

namespace detail {
common::ErrnoError WriteRequest(common::libev::IoClient* client, const request_t& request) WARN_UNUSED_RESULT;
common::ErrnoError WriteResponce(common::libev::IoClient* client, const responce_t& responce) WARN_UNUSED_RESULT;
common::ErrnoError ReadCommand(common::libev::IoClient* client, std::string* out) WARN_UNUSED_RESULT;
}  // namespace detail

template <typename Client>
class ProtocolClient : public Client {
 public:
  typedef Client base_class;

  template <typename... Args>
  ProtocolClient(Args... args) : base_class(args...) {}

  common::ErrnoError WriteRequest(const request_t& request) WARN_UNUSED_RESULT {
    common::ErrnoError err = detail::WriteRequest(this, request);
    if (!err && !request.IsNotification()) {
      requests_queue_[request.id] = request;
    }
    return err;
  }

  common::ErrnoError WriteResponce(const responce_t& responce) WARN_UNUSED_RESULT {
    return detail::WriteResponce(this, responce);
  }

  common::ErrnoError ReadCommand(std::string* out) WARN_UNUSED_RESULT { return detail::ReadCommand(this, out); }

  bool PopRequestByID(sequance_id_t sid, request_t* req) {
    if (!req || !sid) {
      return false;
    }

    auto found_it = requests_queue_.find(sid);
    if (found_it == requests_queue_.end()) {
      return false;
    }

    *req = found_it->second;
    requests_queue_.erase(found_it);
    return true;
  }

 private:
  std::map<sequance_id_t, request_t> requests_queue_;
  using Client::Read;
  using Client::Write;
};

typedef ProtocolClient<common::libev::IoClient> protocol_client_t;

}  // namespace protocol
}  // namespace iptv_cloud
