#include <iostream>

#include "rpc/rpc.h"
#include "link.h"
#include "serdes/abstract/struct.h"
#include "serdes/concrete/format/msgpack.h"
#include "hash/hash.h"

// structures to pass around
struct A {
  int val_i;
  std::string val_s;
};

STRUCT_SERDES(A)
STRUCT_HASH(A)

struct B {
  A a_in_b;
};

// support serialization of B with macro
STRUCT_SERDES(B)
STRUCT_HASH(B)

// Objects to hadle RPC
struct Obj1 {
  A a;
  B b;
  bool store_ab(A&& a_, B&& b_) {
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
    template<typename T>
    T tock(T&& t) {
        return t;
    }
};

// Classes of objects which process calls should have a hash
// because calls are dispatched via hashes of whole signatures of methods,
// including class: hash(class, result, arguments)
SET_HASH(Obj1, 1)
SET_HASH(Obj2, 2)

int main()
{
    // define rpc type

    // example how to use specialization of template method
    constexpr std::string (Obj2::* tock_s)(std::string&&) = &Obj2::tock;

    using tock_i = method<static_cast<int (Obj2::*)(int&&)>(&Obj2::tock)>;

    using rpc_ab = rpc<
        method<&Obj1::store_ab>,
        method<&Obj1::get_ab>,
        method<&Obj2::tick>,
        tock_i,
        method<tock_s>
    >;


    // imitating network
    link<serdes::format::msgpack> l{};

    // make rpc server
    // you should pass all objects, which participate in rpc
    auto srv = rpc_ab::make_server(l, Obj1{}, Obj2{});
    // note: order of passing objects is irrelevant
    // this is accepted too: 
    //   auto srv = rpc_ab::make_server(l, Obj2{}, Obj1{});

    // make an rpc client
    auto client = rpc_ab::make_client(std::move(l));

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

    client->call<method<tock_s>>({{"qwerty"}}, [](std::string&& t){
        std::cout << "from Obj2::tock for string" << std::endl;
        std::cout << t << std::endl;
    });

    client->call<tock_i>({123}, [](int&& t){
        std::cout << "from Obj2::tock for int" << std::endl;
        std::cout << t << std::endl;
    });
}
