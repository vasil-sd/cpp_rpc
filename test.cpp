#include <iostream>

#include "rpc.h"
#include "link.h"

/*
TODO: tuple equivalence up to permutations to pass args out of order to rpc::server
*/

// structures to pass around
struct A {
  int val_i;
  std::string val_s;
};

// support serialization of A, not using macro
template<typename format, typename wbuffer, typename rbuffer>
struct serdes<format, wbuffer, rbuffer, A> : struct_serdes<format, wbuffer, rbuffer, A> {};

struct B {
  A a_in_b;
};

// support serialization of B with macro
STRUCT_SERDES(B)


// Objects to hadle RPC
struct Obj1 {
  A a;
  B b;
  bool store_ab(A a_, B b_) {
      a = a_;
      b = b_;
      return true;
  }
  auto get_ab() {
      return std::make_tuple(a, b);
  }
};

struct Obj2 {
    int c;
    int tick() {
        return c++;
    }
};

int main()
{
    // define rpc type
    using rpc_12 = rpc<
        method<&Obj1::store_ab>,
        method<&Obj1::get_ab>,
        method<&Obj2::tick>
    >;

    // imitating network
    link l{};

    // make rpc server
    auto srv = rpc_12::make_server(l, Obj1{}, Obj2{});

    // make rpc client
    auto client = rpc_12::make_client(std::move(l));

    // note: in general case there is no guarantee on order of
    // received results

    // call remote methods
    client->call<method<&Obj1::store_ab>>({{42, "qwe"},{{22, "abc"}}}, [](bool&& b){
        std::cout << "from Obj1::store_ab" << std::endl;
        std::cout << b << std::endl;
    });

    // call remote methods
    client->call<method<&Obj1::get_ab>>({}, [](std::tuple<A, B>&& ab){
        std::cout << "from Obj1::get_ab" << std::endl;
        auto&& [a, b] = ab;
        std::cout << "a.val_i = " << a.val_i << std::endl;
        std::cout << "a.val_s = " << a.val_s << std::endl;
        std::cout << "b.a_in_b.val_i = " << b.a_in_b.val_i << std::endl;
        std::cout << "b.a_in_b..val_s = " << b.a_in_b.val_s << std::endl;
    });

    client->call<method<&Obj2::tick>>({}, [](int&& t){
        std::cout << "from Obj2::tick" << std::endl;
        std::cout << t << std::endl;
    });

    client->call<method<&Obj2::tick>>({}, [](int&& t){
        std::cout << "from Obj2::tick" << std::endl;
        std::cout << t << std::endl;
    });
}