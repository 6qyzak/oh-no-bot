#ifndef __QYZK_CACHE_H__
#define __QYZK_CACHE_H__

/*
 * an shitty implemetaion of cache managing
 */

#include <optional>
#include <tuple>

namespace qyzk
{

template <
    typename descriptor,
    typename... types
>
class cache
{
public:
    using key_type = typename descriptor::key_type;
    static_assert(std::is_convertible_v< std::underlying_type_t< key_type >, std::size_t >);

    template <key_type index>
    constexpr auto has() const noexcept -> bool
    {
        return std::get< static_cast< std::size_t >(index) >(m_cache).has_value();
    }

    template <key_type index>
    constexpr auto get() const noexcept -> auto const&
    {
        return std::get< static_cast< std::size_t >(index) >(m_cache).value();
    }

    template <key_type index, typename value_type>
    auto set(value_type&& value) noexcept
    {
        std::get< static_cast< std::size_t >(index) >(m_cache) = value;
    }

private:
    std::tuple< std::optional< types >... > m_cache;

    static_assert(descriptor::number() == std::tuple_size< decltype(m_cache) >());
};

} // namespace

#endif
