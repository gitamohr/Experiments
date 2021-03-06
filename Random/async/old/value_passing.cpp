#include <atomic>
#include <chrono>
#include <cstdio>
#include <ecst/thread_pool.hpp>
#include <ecst/utils.hpp>
#include <experimental/tuple>
#include <functional>
#include <thread>
#include <tuple>

#ifdef NOT_ATOM
#define IF_CONSTEXPR \
    if               \
    constexpr
#else
#define IF_CONSTEXPR if
#endif

namespace ll
{
    using pool = ecst::thread_pool;

    inline void sleep_ms(int ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    template <typename T>
    inline void print_sleep_ms(int ms, const T& x)
    {
        std::puts(x);
        sleep_ms(ms);
    }

    template <typename TTuple, typename TF>
    void for_tuple(TTuple&& t, TF&& f)
    {
        std::experimental::apply(
            [&f](auto&&... xs) { (f(FWD(xs)), ...); }, FWD(t));
    }

    struct nothing_t
    {
    };

    constexpr nothing_t nothing{};

    struct context
    {
        pool& _p;
        context(pool& p) : _p{p}
        {
        }

        template <typename TReturn, typename TF>
        auto build(TF&& f);
    };

    struct base_node
    {
        context& _ctx;
        base_node(context& ctx) noexcept : _ctx{ctx}
        {
        }
    };

    struct root : base_node
    {
        using base_node::base_node;

        template <typename TNode, typename... TNodes>
        void start(TNode& n, TNodes&... ns)
        {
            n.execute(nothing, ns...);
        }

        auto& ctx() & noexcept
        {
            return this->_ctx;
        }

        const auto& ctx() const & noexcept
        {
            return this->_ctx;
        }
    };

    template <typename TParent>
    struct parent_holder
    {
        TParent _p;

        template <typename TParentFwd>
        parent_holder(TParentFwd&& p) : _p{FWD(p)}
        {
        }
    };

    template <typename TParent>
    struct child_of : parent_holder<TParent>
    {
        using parent_holder<TParent>::parent_holder;

        auto& parent() & noexcept
        {
            return this->_p;
        }

        const auto& parent() const & noexcept
        {
            return this->_p;
        }

        auto parent() && noexcept
        {
            return std::move(this->_p);
        }

        auto& ctx() & noexcept
        {
            return this->_p.ctx();
        }

        const auto& ctx() const & noexcept
        {
            return this->_p.ctx();
        }
    };

    template <typename TF>
    decltype(auto) call_ignoring_nothing(TF&& f);

    template <typename TF, typename... Ts>
    decltype(auto) call_ignoring_nothing(TF&& f, nothing_t, Ts&&... xs);

    template <typename TF, typename T, typename... Ts>
    decltype(auto) call_ignoring_nothing(TF&& f, T&& x, Ts&&... xs)
    {
        return call_ignoring_nothing(
            [ f = FWD(f), x = FWD(x) ](auto&&... ys) mutable->decltype(
                auto) { return f(FWD(x), FWD(ys)...); },
            FWD(xs)...);
    }

    template <typename TF, typename... Ts>
    decltype(auto) call_ignoring_nothing(TF&& f, nothing_t, Ts&&... xs)
    {
        return call_ignoring_nothing(f, FWD(xs)...);
    }

    template <typename TF>
    decltype(auto) call_ignoring_nothing(TF&& f)
    {
        return f();
    }

    template <typename TF, typename... Ts>
    decltype(auto) with_void_to_nothing(TF&& f, Ts&&... xs)
    {
#define BOUND_F() call_ignoring_nothing(f, FWD(xs)...)

        // clang-format off
        IF_CONSTEXPR(std::is_same<decltype(BOUND_F()), void>{})
        {
            BOUND_F();
            return nothing;
        }
        else
        {
            return BOUND_F();
        }
// clang-format on

#undef BOUND_F
    }

    template <typename TParent, typename TReturn, typename TF>
    struct node_then : child_of<TParent>, TF
    {
        using this_type = node_then<TParent, TReturn, TF>;

        auto& as_f() noexcept
        {
            return static_cast<TF&>(*this);
        }

        template <typename TParentFwd, typename TFFwd>
        node_then(TParentFwd&& p, TFFwd&& f)
            : child_of<TParent>{FWD(p)}, TF{FWD(f)}
        {
        }

        template <typename T>
        auto execute(T&& x) &
        {
            this->ctx()._p.post([ this, x = FWD(x) ]() mutable {
                with_void_to_nothing(as_f(), FWD(x));
            });
        }

        template <typename T, typename TNode, typename... TNodes>
        auto execute(T&& x, TNode& n, TNodes&... ns) &
        {
            this->ctx()._p.post([ this, x = FWD(x), &n, &ns... ]() mutable {
                decltype(auto) res = with_void_to_nothing(as_f(), FWD(x));

                this->ctx()._p.post(
                    [ res = std::move(res), &n, &ns... ]() mutable {
                        n.execute(std::move(res), ns...);
                    });
            });
        }

        template <typename TContReturn, typename TCont>
        auto then(TCont&& cont) &&
        {
            return node_then<this_type, TContReturn, TCont>{
                std::move(*this), FWD(cont)};
        }

        template <typename... TConts>
        auto wait_all(TConts&&... cont) &&;

        template <typename... TNodes>
        auto start(TNodes&... ns)
        {
            this->parent().start(*this, ns...);
        }
    };

    template <typename T>
    struct movable_atomic : std::atomic<T>
    {
        using base_type = std::atomic<T>;
        using base_type::base_type;

        movable_atomic(movable_atomic&& r) : base_type{r.load()}
        {
        }

        movable_atomic& operator=(movable_atomic&& r)
        {
            static_cast<base_type&>(*this).store(r.load());
            return *this;
        }
    };

    template <typename TParent, typename... TFs>
    struct node_wait_all : child_of<TParent>, TFs...
    {
        using this_type = node_wait_all<TParent, TFs...>;

        movable_atomic<int> _ctr{sizeof...(TFs)};

        template <typename TParentFwd, typename... TFFwds>
        node_wait_all(TParentFwd&& p, TFFwds&&... fs)
            : child_of<TParent>{FWD(p)}, TFs{FWD(fs)}...
        {
        }

        auto execute() &
        {
            //(this->ctx()._p.post([&] { static_cast<TFs&> (*this)(); }), ...);
        }

        template <typename TNode, typename... TNodes>
        auto execute(TNode& n, TNodes&... ns) &
        {
            auto exec = [this, &n, &ns...](auto& f) {
                this->ctx()._p.post([&] {
                    f();
                    if(--_ctr == 0)
                    {
                        n.execute(ns...);
                    }
                });
            };

            // TODO: gcc bug?
            //    (exec(static_cast<TFs&>(*this)), ...);
        }

        template <typename TReturn, typename TCont>
        auto then(TCont&& cont) &
        {
            return node_then<this_type&, TReturn, TCont>{*this, FWD(cont)};
        }
        template <typename TReturn, typename TCont>
        auto then(TCont&& cont) &&
        {
            return node_then<this_type, TReturn, TCont>{
                std::move(*this), FWD(cont)};
        }

        template <typename... TNodes>
        auto start(TNodes&... ns) &
        {
            this->parent().start(*this, ns...);
        }
    };

    template <typename TParent, typename TReturn, typename TF>
    template <typename... TConts>
    auto node_then<TParent, TReturn, TF>::wait_all(TConts&&... conts) &&
    {
        return node_wait_all<this_type, TConts...>{
            std::move(*this), FWD(conts)...};
    }

    template <typename TReturn, typename TF>
    auto context::build(TF&& f)
    {
        return node_then<root, TReturn, TF>(root{*this}, FWD(f));
    }
}

template <typename T>
void execute_after_move(T x)
{
    x.start();
    ll::sleep_ms(1200);
}

int main()
{
    ll::pool p;
    ll::context ctx{p};

    auto computation =
        ctx.build<int>([] { return 10; })
            .then<int>([](int x) { return std::to_string(x); })
            .then<std::string>([](std::string x) { return "num: " + x; })
            .then<void>([](std::string x) { std::printf("%s\n", x.c_str()); });

    ctx.build<int>([] { return 10; })
        .then<int>([](int x) { return std::to_string(x); })
        .then<std::string>([](std::string x) { return "num: " + x; })
        .then<void>([](std::string x) { std::printf("%s\n", x.c_str()); })
        .start();

    execute_after_move(std::move(computation));
    ll::sleep_ms(200);
}
