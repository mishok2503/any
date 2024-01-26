#include "any.h"

namespace utils {
    any::any(const any &other) : v_(other.v_ ? other.v_->clone() : nullptr) {
    }

    any::any(any &&other) noexcept: v_(std::exchange(other.v_, nullptr)) {
    }

    any &any::operator=(const any &other) {
        if (this != &other) {
            auto *t = v_;
            v_ = other.v_ ? other.v_->clone() : nullptr;
            delete t;
        }
        return *this;
    }

    any &any::operator=(any &&other) noexcept {
        delete v_;
        v_ = std::exchange(other.v_, nullptr);
        return *this;
    }

    [[nodiscard]] bool any::empty() const {
        return v_ == nullptr;
    }

    void any::clear() {
        delete v_;
        v_ = nullptr;
    }

    void any::swap(any &other) {
        std::swap(v_, other.v_);
    }

    const std::type_info &any::type() const {
        if (v_) {
            return v_->type();
        }
        throw bad_any_cast{"Any is empty"};
    }

    any::~any() {
        clear();
    }

}  // namespace utils