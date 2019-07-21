#pragma once

#include "buffer/simple_buffer.h"
#include "serdes/serdes.h"

#include <unordered_map>
#include <functional>

// siple imitation of network link

template<class format>
struct link {
  std::unordered_map<unsigned long long, std::function<buffer(buffer&&)>> callbacks;
  template<unsigned long long N, typename Result, typename Args>
  void on_receive(std::function<Result(Args&&)>&& handler) {
      std::cout << "HASH = " << N << std::endl;
      callbacks.emplace(N, [handler=std::move(handler)](buffer&& b) -> buffer {
          buffer rbuf = std::move(b);
          rbuf.rewind();
          Args args = serdes::deserialize<format, Args>(rbuf);
          Result result = handler(std::move(args));
          buffer wbuf = serdes::serialize<format, buffer>(result);
          return std::move(wbuf);
      }); 
  }
  template<unsigned long long N, typename Result, typename Args>
  void send_async(Args&& args, std::function<void(Result&&)>&& handler) {
      buffer wbuf = serdes::serialize<format, buffer>(args);
      auto cb = callbacks.at(N);
      buffer rbuf{cb(std::move(wbuf))};
      rbuf.rewind();
      handler(serdes::deserialize<format, Result>(rbuf));
  }
};
