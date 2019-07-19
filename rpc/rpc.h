#pragma once

#include "method_info.h"

#include <memory>
#include <functional>
#include <tuple>
#include <type_traits>

// TODO: support failed calls: for instance via additional lambda for error handling

namespace {

template<typename needle, typename...haystack>
constexpr bool is_type_present()
{
  return (std::is_same<needle, haystack>::value || ...);
}

template <template<typename...> typename, typename...> struct dedup_template_args;

template<template<typename...> typename T, typename t, typename ... list, typename ... result>
struct dedup_template_args<T, T<t, list...>, T<result...>>
{
  using type= typename
    std::conditional<
      is_type_present<t, list...>(),
      typename dedup_template_args<
        T,
        T<list...>,
        T<result...>
      >::type,
      typename dedup_template_args<
        T,
        T<list...>,
        T<result..., t>
      >::type
    >::type;
};
 
template<template<typename...> typename T, typename ... list>
struct dedup_template_args<T, T<>, T<list...>>
{
  using type = T<list...>;
};

template<template<typename...> typename T, typename ... list>
struct dedup_template_args<T, T<list...>>
{
  using type = typename dedup_template_args<T, T<list...>, T<>>::type;
};

} // anonymous namespace

template<class... methods>
struct rpc {
    using objects = typename dedup_template_args<std::tuple, std::tuple<typename methods::class_type ...>>::type;

    struct server : std::enable_shared_from_this<server> {
        objects objs;

        template<int N, typename link, class... meths>
        struct register_cb {
            void operator()(link&, server&) {};
        };

        template<int N, typename, class... meths>
        friend class register_cb;

        server(objects&& o) : objs(std::move(o)) { }
    };

    template<typename link>
    struct client : std::enable_shared_from_this<client<link>> {
        link lnk;
        client(link&& l) : lnk(std::move(l)) { }

        template<int N, class... meths>
        struct call_method { };

        template<typename m>
        void call(typename m::args_type&& args, const std::function<void(typename m::result_type&&)>& fn) {
            call_method<0, m, methods...>{}(lnk, std::move(args), fn);
        }
    };

    using server_ptr = std::shared_ptr<server>;

    template<class link, typename ... Objects>
    static server_ptr make_server(link& lnk, Objects&&... objs) {
        auto srv = std::make_shared<server>(std::tuple{std::move(objs)...});
        (typename server::template register_cb<0, link, methods...>){}(lnk, *srv);
        return srv;
    }

    template<class link>
    using client_ptr = std::shared_ptr<client<link>>;

    template<class link>
    static client_ptr<link> make_client(link&& lnk) {
        return std::make_shared<client<link>>(std::move(lnk));
    }

};

template<class... methods>
template<int N, typename link, class m, class... meths>
struct rpc<methods...>::server::register_cb<N, link, m, meths...> {
    using result_t = typename m::result_type;
    using args_t = typename m::args_type;
    using class_t = typename m::class_type;

    void operator()(link& l, server& s) {
        auto self = s.shared_from_this();
        l.link::template on_receive<N, result_t, args_t>([self](args_t&& args) -> result_t {
            return m::call(std::get<class_t>(self->objs), std::move(args));
        });
        register_cb<N+1, link, meths...>{}(l, s);
    }
};

template<typename... methods>
template<typename link>
template<int N, class m, class... meths>
struct rpc<methods...>:: template client<link>::call_method<N, m, m, meths...> {
    using result_t = typename m::result_type;
    using args_t = typename m::args_type;
    using handler_t = std::function<void(result_t&&)>;

    void operator()(link& l, args_t&& args, const handler_t& fn){
        using result_t = typename m::result_type;
        using args_t = typename m::args_type;
        l.link::template send_async<N, result_t, args_t>(std::move(args), fn);
    }
};

template<typename... methods>
template<typename link>
template<int N, class m, class n, class... meths>
struct rpc<methods...>::template client<link>::call_method<N, m, n, meths...> :
       rpc<methods...>::template client<link>::template call_method<N+1, m, meths...> {};
