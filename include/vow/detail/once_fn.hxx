#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_ONCE_FN
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_ONCE_FN

#include <vow/detail/on_scope_exit.hxx>

#include <cstddef>
#include <memory>
#include <utility>
#include <functional>
#include <new>
#include <type_traits>
#include <algorithm>

#ifndef VOW_DETAIL_ONCE_FN_DEFAULT_BUFFER_SIZE
    #define VOW_DETAIL_ONCE_FN_DEFAULT_BUFFER_SIZE (sizeof(void*) * 3)
#endif

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <typename Result, typename... Args>
struct once_fn_ops_t
{
    once_fn_ops_t const& (&transfer)(
        void* from_buffer, void* to_buffer, std::size_t to_buffer_size);
    void (&destroy)(void* buffer);
    Result (&invoke)(void* buffer, Args&&... args);
};

template <typename Fn>
auto once_fn_align(void* ptr, std::size_t space = -1)
-> Fn*
{ return static_cast<Fn*>(std::align(alignof(Fn), sizeof(Fn), ptr, space)); }

template <typename Result, typename... Args, typename Fn>
auto once_fn_create(Fn&& fn, void* buffer, std::size_t buffer_size)
-> once_fn_ops_t<Result, Args...> const&;

template <typename Fn, typename Result, typename... Args>
constexpr
once_fn_ops_t<Result, Args...> once_fn_inplace_ops = {
    /* transfer */ *+[](void* from_buffer,
                        void* to_buffer, std::size_t to_buffer_size)
    -> once_fn_ops_t<Result, Args...> const&
    {
        auto& from_fn = *once_fn_align<Fn>(from_buffer);
        auto& ops = once_fn_create<Result, Args...>(
            std::move(from_fn), to_buffer, to_buffer_size);
        from_fn.~Fn(); 
        return ops;
    },

    /* destroy */ *+[](void* buffer)
    -> void
    {
        auto& fn = *once_fn_align<Fn>(buffer);
        fn.~Fn();
    },

    /* invoke */ *+[](void* buffer, Args&&... args)
    -> Result 
    {
        auto& fn = *once_fn_align<Fn>(buffer);
        VOW_DETAIL_ON_SCOPE_EXIT[&]{ fn.~Fn(); };
        return std::invoke(std::move(fn), std::forward<Args>(args)...);
    }
};

template <typename Fn, typename Result, typename... Args>
constexpr
once_fn_ops_t<Result, Args...> once_fn_ptr_ops = {
    /* transfer */ *+[](void* from_buffer,
                        void* to_buffer, std::size_t to_buffer_size)
    -> once_fn_ops_t<Result, Args...> const&
    {
        auto* from_fn_ptr = 
            static_cast<Fn*>(*once_fn_align<void*>(from_buffer));
        if constexpr (std::is_nothrow_move_constructible_v<Fn>) {
            if (auto* to_fn_ptr =
                    once_fn_align<Fn>(to_buffer, to_buffer_size))
            {
                new(to_fn_ptr) Fn(std::move(*from_fn_ptr));
                delete from_fn_ptr;
                return once_fn_inplace_ops<Fn, Result, Args...>;
            } 
        }
        auto*& to_void_fn_ptr = 
            *once_fn_align<void*>(to_buffer, to_buffer_size);
        to_void_fn_ptr = from_fn_ptr;
        return once_fn_ptr_ops<Fn, Result, Args...>;
    },

    /* destroy */ *+[](void* buffer)
    -> void
    {
        auto* fn_ptr = static_cast<Fn*>(*once_fn_align<void*>(buffer));
        delete fn_ptr;
    },

    /* invoke */ *+[](void* buffer, Args&&... args)
    -> Result 
    {
        auto* fn_ptr = static_cast<Fn*>(*once_fn_align<void*>(buffer));
        VOW_DETAIL_ON_SCOPE_EXIT[&]{ delete fn_ptr; };
        return std::invoke(std::move(*fn_ptr), std::forward<Args>(args)...);
    }
};

template <typename Result, typename... Args, typename Fn>
auto once_fn_create(Fn&& fn, void* buffer, std::size_t buffer_size)
-> once_fn_ops_t<Result, Args...> const&
{
    if constexpr (std::is_nothrow_move_constructible_v<std::decay_t<Fn>>) {
        if (auto* fn_ptr =
                once_fn_align<std::decay_t<Fn>>(buffer, buffer_size))
        {
            new(fn_ptr) std::decay_t<Fn>(std::forward<Fn>(fn));
            return once_fn_inplace_ops<std::decay_t<Fn>, Result, Args...>;
        } 
    }
    auto*& void_fn_ptr = *once_fn_align<void*>(buffer, buffer_size);
    void_fn_ptr = new std::decay_t<Fn>(std::forward<Fn>(fn));
    return once_fn_ptr_ops<std::decay_t<Fn>, Result, Args...>;
}

template <typename Signature,
          std::size_t Size = VOW_DETAIL_ONCE_FN_DEFAULT_BUFFER_SIZE>
struct once_fn;

template <typename Result, typename... Args, std::size_t Size>
struct once_fn<Result(Args...), Size>
{
    template <typename Fn, std::enable_if_t<
        std::is_invocable_r_v<Result, std::decay_t<Fn>, Args...>>*...>
    once_fn(Fn&& fn)
    {
        ops_ = &once_fn_create<Result, Args...>(std::forward<Fn>(fn),
                                                buffer_, sizeof(buffer_));
    }

    once_fn(std::nullptr_t = nullptr)
    noexcept
    : ops_(nullptr)
    {}

    once_fn(once_fn&& other)
    noexcept
    {
        if (other.ops_) {
            ops_ = &std::exchange(other.ops_, nullptr)
                ->transfer(other.buffer_, buffer_, sizeof(buffer_));
        } else {
            ops_ = nullptr;
        }
    }

    auto operator=(once_fn&& other)
    noexcept
    -> once_fn&
    {
        this->~once_fn();
        new(this) once_fn(std::move(other));
        return *this;
    }

    ~once_fn()
    { if (ops_) { ops_->destroy(buffer_); } }

    explicit operator bool() const
    noexcept
    { return ops_; }

    template <typename... Args_, std::enable_if_t<
        std::is_invocable_v<void(Args&&...), Args_...>>*...>
    auto operator()(Args_&&... args)
    -> Result
    { return std::exchange(ops_, nullptr)
        ->invoke(buffer_, std::forward<Args_>(args)...); }

private:
    once_fn_ops_t<Result, Args...> const* ops_;

    alignas(void*)
    char buffer_[std::max(sizeof(void*), Size)];
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
