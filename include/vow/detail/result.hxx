#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_RESULT
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_RESULT

#include <new>
#include <utility>
#include <exception>
#include <cassert>
#include <memory>

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

struct with_value_t
{
    explicit with_value_t() = default;
};

inline constexpr auto with_value = with_value_t{};

struct with_exception_t
{
    explicit with_exception_t() = default;
};

inline constexpr auto with_exception = with_exception_t{};


template <typename Value>
struct result
{
    template <typename... Args>
    result(with_value_t, Args&&... args)
    noexcept
    {
        try {
            new(&value_) auto(std::forward<Args>(args)...);
            has_exception_ = false;
        } catch (...) {
            new(&exception_ptr_) auto(std::current_exception());
            has_exception_ = true;
        }
    }

    result(with_exception_t, std::exception_ptr exception_ptr)
    noexcept
    : exception_ptr_(std::move(exception_ptr))
    , has_exception_(true)
    { assert(exception_ptr_); }

    result(result&& other)
    noexcept
    {
        if (other.has_exception_) {
            new(&exception_ptr_) auto(std::move(other.exception_ptr_));
            has_exception_ = true;
        } else {
            try {
                new(&value_) auto(std::move(other.value_));
                has_exception_ = false;
            } catch (...) {
                new(&exception_ptr_) auto(std::current_exception());
                has_exception_ = true;
            }
        }
    }

    result& operator=(result&& other)
    noexcept
    {
        if (has_exception_) {
            if (other.has_exception_) {
                exception_ptr_ = std::move(other.exception_ptr_);
            } else {
                exception_ptr_.~exception_ptr();
                try {
                    new(&value_) auto(std::move(other.value_));
                    has_exception_ = false;
                } catch (...) {
                    new(&exception_ptr_) auto(std::current_exception());
                }
            }
        } else {
            if (other.has_exception_) {
                value_.~Value();
                new(&exception_ptr_) auto(std::move(other.exception_ptr_));
                has_exception_ = true;
            } else {
                try {
                    value_ = std::move(other.value_);
                } catch (...) {
                    value_.~Value();
                    new(&exception_ptr_) auto(std::current_exception());
                    has_exception_ = true;
                }
            }
        }
        return *this;
    }

    ~result()
    {
        if (has_exception_) {
            exception_ptr_.~exception_ptr();
        } else {
            value_.~Value();
        }
    }

    auto get() &&
    -> Value&&
    {
        if (has_exception_) {
            std::rethrow_exception(std::move(exception_ptr_));
        }
        return std::move(value_);
    }

private:
    union {
        Value value_;
        std::exception_ptr exception_ptr_;
    };
    bool has_exception_;
};

template <typename Value>
struct result<Value&>
{
    result(with_value_t, Value& value)
    noexcept
    : value_ptr_(std::addressof(value))
    , has_exception_(false)
    {}

    result(with_exception_t, std::exception_ptr exception_ptr)
    noexcept
    : exception_ptr_(std::move(exception_ptr))
    , has_exception_(true)
    { assert(exception_ptr_); }

    result(result&& other)
    noexcept
    {
        if (other.has_exception_) {
            new(&exception_ptr_) auto(std::move(other.exception_ptr_));
            has_exception_ = true;
        } else {
            value_ptr_ = other.value_ptr_;
            has_exception_ = false;
        }
    }

    result& operator=(result&& other)
    noexcept
    {
        if (has_exception_) {
            if (other.has_exception_) {
                exception_ptr_ = std::move(other.exception_ptr_);
            } else {
                exception_ptr_.~exception_ptr();
                value_ptr_ = other.value_ptr_;
                has_exception_ = false;
            }
        } else {
            if (other.has_exception_) {
                new(&exception_ptr_) auto(std::move(other.exception_ptr_));
                has_exception_ = true;
            } else {
                value_ptr_ = other.value_ptr_;
            }
        }
        return *this;
    }

    ~result()
    {
        if (has_exception_) {
            exception_ptr_.~exception_ptr();
        }
    }

    auto get() &&
    -> Value&
    {
        if (has_exception_) {
            std::rethrow_exception(std::move(exception_ptr_));
        }
        return *value_ptr_;
    }

private:
    union {
        Value* value_ptr_;
        std::exception_ptr exception_ptr_;
    };
    bool has_exception_;
};

template <>
struct result<void>
{
    result(with_value_t)
    noexcept
    : exception_ptr_(nullptr)
    {}

    result(with_exception_t, std::exception_ptr exception_ptr)
    noexcept
    : exception_ptr_(std::move(exception_ptr))
    { assert(exception_ptr_); }

    result(result&& other)
    noexcept
    : exception_ptr_(std::move(other.exception_ptr_))
    {}

    result& operator=(result&& other)
    noexcept
    {
        exception_ptr_ = std::move(other.exception_ptr_);
        return *this;
    }

    void get() &&
    {
        if (exception_ptr_) {
            std::rethrow_exception(std::move(exception_ptr_));
        }
    }

private:
    std::exception_ptr exception_ptr_;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
