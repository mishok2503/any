#pragma once

#include <concepts>
#include <utility>
#include <typeinfo>
#include <stdexcept>

namespace utils {
    template<class T>
    concept NotAny = !std::same_as<std::remove_cvref_t<T>, class any>;

    class bad_any_cast : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class any {
    private:
        struct value_base {
            [[nodiscard]] virtual value_base *clone() const = 0;

            [[nodiscard]] virtual const std::type_info &type() const = 0;

            virtual ~value_base() = default;
        };

        template<typename T>
        struct Value : value_base {
            T value;

            template<class F>
            Value(F &&v) : value(std::forward<F>(v)) {
            }

            [[nodiscard]] value_base *clone() const override {
                return new Value(value);
            }

            [[nodiscard]] const std::type_info &type() const override {
                return typeid(T);
            }
        };

        value_base *v_ = nullptr;

    public:
        any() = default;

        template<NotAny T>
        any(T &&value) {
            v_ = new Value<std::decay_t<T>>(std::forward<T>(value));
        }

        any(const any &other);

        any(any &&other) noexcept;

        any &operator=(const any &other);

        any &operator=(any &&other) noexcept;

        [[nodiscard]] bool empty() const;

        void clear();

        void swap(any &other);

        [[nodiscard]] const std::type_info &type() const;

        ~any();

        // throws only std::bad_cast
        template<class T>
        const T &getValue() const {
            if (!v_) {
                throw std::bad_cast{};
            }
            return dynamic_cast<Value<T> &>(*v_).value;
        }

        friend void swap(any &lhs, any &rhs) {
            lhs.swap(rhs);
        }

        template<class T>
        friend T *any_cast(any *a) noexcept;

        template<class T>
        friend const T *any_cast(const any *a) noexcept;
    };

    template<class T>
    const T *any_cast(const any *a) noexcept {
        if (!a->v_) {
            return nullptr;
        }
        auto *res = dynamic_cast<any::Value<T> *>(a->v_);
        return res ? &res->value : nullptr;
    }

    template<class T>
    T *any_cast(any *a) noexcept {
        if (!a->v_) {
            return nullptr;
        }
        auto *res = dynamic_cast<any::Value<T> *>(a->v_);
        return res ? &res->value : nullptr;
    }

    // TODO: add demangling for human readable names
#define THROW_BAD_CAST(any, T) \
    throw bad_any_cast{std::string{"Cast error: any type - "} + any.type().name() + ", required type - " + typeid(T).name()};

    template<class T>
    T any_cast(const any &a) {
        if (auto *res = any_cast<std::decay_t<T>>(&a)) {
            return static_cast<T>(*res);
        }
        THROW_BAD_CAST(a, T);
    }

    template<class T>
    T any_cast(any &a) {
        if (auto *res = any_cast<std::decay_t<T>>(&a)) {
            return static_cast<T>(*res);
        }
        THROW_BAD_CAST(a, T);
    }

    template<class T>
    T any_cast(any &&a) {
        if (auto *res = any_cast<std::decay_t<T>>(&a)) {
            return static_cast<T>(std::move(*res));
        }
        THROW_BAD_CAST(a, T);
    }

#undef THROW_BAD_CAST

}  // namespace utils