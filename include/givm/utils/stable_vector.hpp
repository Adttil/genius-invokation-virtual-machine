#ifndef GIVM_UTILS_STABLE_VECTOR_HPP
#define GIVM_UTILS_STABLE_VECTOR_HPP

#include <vector>

namespace givm
{
    template<class T>
    class stable_vector
    {
    public:
        template<class TC>
        class iterator
        {
        public:
        constexpr iterator(stable_vector<T>* vec, size_t index)
            : vec_{ vec }, index_{ index }
            {
                advance_to_next_valid();
            }

            constexpr T& operator*() const
            {
                return vec_->data_[index_];
            }

            constexpr iterator& operator++()
            {
                ++index_;
                advance_to_next_valid();
                return *this;
            }

            constexpr bool operator!=(const iterator& other) const
            {
                return index_ != other.index_;
            }
        private:
            void advance_to_next_valid()
            {
                while(index_ < vec_->data_.size() && vec_->deleted_flags_[index_])
                {
                    ++index_;
                }
            }

            stable_vector<T>* vec_;
            size_t index_;
        };



    private:
        std::vector<T> data_;
        std::vector<bool> deleted_flags_;
    };
}

#endif
