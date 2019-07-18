#pragma once

#include "lw_msgpack.h"
#include <unordered_map>
#include <functional>

// siple imitation of network link

struct link {
  std::unordered_map<int, std::function<buffer(wbuffer&&)>> callbacks;
  template<int N, typename Result, typename Args>
  void on_receive(std::function<Result(Args&&)>&& handler) {
      callbacks.emplace(N, [handler=std::move(handler)](buffer&& b) -> buffer {
          rbuffer rbuf = std::move(b);
          Args args = deserialize<Args>(rbuf);
          Result result = handler(std::move(args));
          wbuffer wbuf = serialize<wbuffer>(result);
          return std::move(wbuf);
      }); 
  }
  template<int N, typename Result, typename Args>
  void send_async(Args&& args, const std::function<void(Result&&)>& handler) {
      wbuffer wbuf = serialize<wbuffer>(args);
      auto cb = callbacks.at(N);
      rbuffer rbuf{cb(std::move(wbuf))};
      handler(deserialize<Result>(rbuf));
  }
};
