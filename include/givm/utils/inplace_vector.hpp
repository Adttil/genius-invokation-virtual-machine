#ifndef GIVM_UTILS_INPLACE_VECTOR_HPP
#define GIVM_UTILS_INPLACE_VECTOR_HPP

#include <memory>

namespace givm
{
    template<class T, size_t capacity>
    class inplace_vector
    {
    public:

        constexpr T* begin()
        {
            return reinterpret_cast<T*>(storage);
        }

        constexpr const T* begin() const
        {
            return reinterpret_cast<const T*>(storage);
        }

        constexpr T* end()
        {
            return begin() + capacity;
        }

        constexpr const T* end() const
        {
            return begin() + capacity;
        }

        constexpr T& operator[](size_t offset)
        {
            return begin()[offset];
        }

        constexpr const T& operator[](size_t offset) const
        {
            return begin()[offset];
        }

        constexpr size_t size() const
        {
            return size_;
        }

        constexpr void push_back(const T& t)
        {
            std::construct_at(begin() + size_, t);
            ++size_;
        }

        constexpr void pop_back()
        {
            --size_;
            std::destroy_at(begin() + size_);
        }

    private:
        size_t size_ = 0;
        alignas(alignof(T)) unsigned char storage[sizeof(T) * capacity];
    };

}

#endif
