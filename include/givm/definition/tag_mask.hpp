#ifndef GIVM_DEFINITION_TAG_MASK_HPP
#define GIVM_DEFINITION_TAG_MASK_HPP

#include <cstddef>
#include <span>
#include <vector>

#include "../table.hpp"

namespace givm
{
    class tag_mask
    {
    public:
        tag_mask() = default;

        explicit tag_mask(size_t size)
        : bits_(size)
        {}

        void set(tag_id id, bool value = true)
        {
            bits_[id.value()] = value;
        }

        bool has(tag_id id) const
        {
            return bits_[id.value()];
        }

        bool has_all(std::span<const tag_id> tags) const
        {
            for(tag_id tag : tags)
            {
                if(not has(tag))
                {
                    return false;
                }
            }
            return true;
        }

        bool has_any(std::span<const tag_id> tags) const
        {
            for(tag_id tag : tags)
            {
                if(has(tag))
                {
                    return true;
                }
            }
            return false;
        }

        bool matches(
            std::span<const tag_id> required_tags,
            std::span<const tag_id> excluded_tags = {}
        ) const
        {
            if(not has_all(required_tags))
            {
                return false;
            }

            for(tag_id tag : excluded_tags)
            {
                if(has(tag))
                {
                    return false;
                }
            }
            return true;
        }

    private:
        std::vector<bool> bits_;
    };

}

#endif
