#pragma once

#include "method_info.h"

#include "meta/dedup.h"
#include "meta/idx.h"

#include <memory>
#include <functional>
#include <tuple>
#include <type_traits>

// TODO: support failed calls: for instance via additional lambda for error handling

template<class... methods>
struct rpc {
    using objects = typename meta::dedup::dedup_template_args<std::tuple, std::tuple<typename methods::class_type ...>>::type;

    struct server : std::enable_shared_from_this<server> {
        objects objs;

        template<unsigned long long N, typename link, class... meths>
        struct register_cb {
            void operator()(link&, server&) {};
        };

        template<unsigned long long N, typename, class... meths>
        friend class register_cb;

        server(objects&& o) : objs(std::move(o)) { }
    };

    template<typename link>
    struct client : std::enable_shared_from_this<client<link>> {
        link lnk;
        client(link&& l) : lnk(std::move(l)) { }

        template<typename m>
        void call(typename m::args_type&& args, const std::function<void(typename m::result_type&&)>& fn) {
            using result_t = typename m::result_type;
            using args_t = typename m::args_type;

            lnk.link::template send_async<m::signature_hash, result_t, args_t>(std::move(args), fn);
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
template<unsigned long long N, typename link, class m, class... meths>
struct rpc<methods...>::server::register_cb<N, link, m, meths...> {
    using result_t = typename m::result_type;
    using args_t = typename m::args_type;
    using class_t = typename m::class_type;

    void operator()(link& l, server& s) {
        auto self = s.shared_from_this();
        l.link::template on_receive<m::signature_hash, result_t, args_t>([self](args_t&& args) -> result_t {
            return m::call(std::get<class_t>(self->objs), std::move(args));
        });
        register_cb<N+1, link, meths...>{}(l, s);
    }
};
