/// \file
// Range v3 library
//
//  Copyright Eric Niebler 2013-present
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//

#ifndef RANGES_V3_VIEW_ZIP_HPP
#define RANGES_V3_VIEW_ZIP_HPP

#include <tuple>
#include <utility>

#include <meta/meta.hpp>

#include <range/v3/range_fwd.hpp>

#include <range/v3/iterator/concepts.hpp>
#include <range/v3/iterator/traits.hpp>
#include <range/v3/utility/common_tuple.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/empty.hpp>
#include <range/v3/view/zip_with.hpp>

#include <range/v3/detail/disable_warnings.hpp>

namespace ranges
{
    /// \cond
    namespace detail
    {
        struct indirect_zip_fn_
        {
            // tuple value
            template<typename... Its>
            [[noreturn]] auto operator()(copy_tag, Its...) const
                -> CPP_ret(std::tuple<iter_value_t<Its>...>)( //
                    requires and_v<indirectly_readable<Its>...> && (sizeof...(Its) != 2))
            {
                RANGES_EXPECT(false);
            }

            // tuple reference
            template<typename... Its>
            auto operator()(Its const &... its) const
                noexcept(meta::and_c<noexcept(iter_reference_t<Its>(*its))...>::value)
                    -> CPP_ret(common_tuple<iter_reference_t<Its>...>)( //
                        requires and_v<indirectly_readable<Its>...> && (sizeof...(Its) != 2))
            {
                return common_tuple<iter_reference_t<Its>...>{*its...};
            }

            // tuple rvalue reference
            template<typename... Its>
            auto operator()(move_tag, Its const &... its) const
                noexcept(meta::and_c<noexcept(
                             iter_rvalue_reference_t<Its>(iter_move(its)))...>::value)
                    -> CPP_ret(common_tuple<iter_rvalue_reference_t<Its>...>)( //
                        requires and_v<indirectly_readable<Its>...> && (sizeof...(Its) != 2))
            {
                return common_tuple<iter_rvalue_reference_t<Its>...>{iter_move(its)...};
            }

            // pair value
            template<typename It1, typename It2>
            [[noreturn]] auto operator()(copy_tag, It1, It2) const
                -> CPP_ret(std::pair<iter_value_t<It1>, iter_value_t<It2>>)( //
                    requires indirectly_readable<It1> && indirectly_readable<It2>)
            {
                RANGES_EXPECT(false);
            }

            // pair reference
            template<typename It1, typename It2>
            auto operator()(It1 const & it1, It2 const & it2) const noexcept(
                noexcept(iter_reference_t<It1>(*it1)) &&
                noexcept(iter_reference_t<It2>(*it2)))
                -> CPP_ret(common_pair<iter_reference_t<It1>, iter_reference_t<It2>>)( //
                    requires indirectly_readable<It1> && indirectly_readable<It2>)
            {
                return {*it1, *it2};
            }

            // pair rvalue reference
            template<typename It1, typename It2>
            auto operator()(move_tag, It1 const & it1, It2 const & it2) const
                noexcept(noexcept(iter_rvalue_reference_t<It1>(iter_move(it1))) &&
                         noexcept(iter_rvalue_reference_t<It2>(iter_move(it2))))
                    -> CPP_ret(common_pair<iter_rvalue_reference_t<It1>,
                                           iter_rvalue_reference_t<It2>>)( //
                        requires indirectly_readable<It1> && indirectly_readable<It2>)
            {
                return {iter_move(it1), iter_move(it2)};
            }
        };
    } // namespace detail
    /// \endcond

    /// \addtogroup group-views
    /// @{
    template<typename... Rngs>
    struct zip_view : iter_zip_with_view<detail::indirect_zip_fn_, Rngs...>
    {
        CPP_assert(sizeof...(Rngs) != 0);

        zip_view() = default;
        explicit zip_view(Rngs... rngs)
          : iter_zip_with_view<detail::indirect_zip_fn_, Rngs...>{
                detail::indirect_zip_fn_{},
                std::move(rngs)...}
        {}
    };

    template<typename... Rng>
    RANGES_INLINE_VAR constexpr bool enable_borrowed_range<zip_view<Rng...>> =
        and_v<enable_borrowed_range<Rng>...>;

#if RANGES_CXX_DEDUCTION_GUIDES >= RANGES_CXX_DEDUCTION_GUIDES_17
    template<typename... Rng>
    zip_view(Rng &&...)->zip_view<views::all_t<Rng>...>;
#endif

    namespace views
    {
        struct zip_fn
        {
            constexpr empty_view<std::tuple<>> operator()() const noexcept
            {
                return {};
            }
            template<typename... Rngs>
            auto operator()(Rngs &&... rngs) const -> CPP_ret(
                zip_view<all_t<Rngs>...>)( //
                requires and_v<viewable_range<Rngs>...> && and_v<input_range<Rngs>...> &&
                (sizeof...(Rngs) != 0))
            {
                return zip_view<all_t<Rngs>...>{all(static_cast<Rngs &&>(rngs))...};
            }
#if defined(_MSC_VER)
            template<typename Rng0>
            constexpr auto operator()(Rng0 && rng0) const
                -> CPP_ret(zip_view<all_t<Rng0>>)( //
                    requires input_range<Rng0> && viewable_range<Rng0>)
            {
                return zip_view<all_t<Rng0>>{all(static_cast<Rng0 &&>(rng0))};
            }
            template<typename Rng0, typename Rng1>
            constexpr auto operator()(Rng0 && rng0, Rng1 && rng1) const
                -> CPP_ret(zip_view<all_t<Rng0>, all_t<Rng1>>)(           //
                    requires input_range<Rng0> && viewable_range<Rng0> && //
                             input_range<Rng1> && viewable_range<Rng1>)
            {
                return zip_view<all_t<Rng0>, all_t<Rng1>>{ //
                    all(static_cast<Rng0 &&>(rng0)),       //
                    all(static_cast<Rng1 &&>(rng1))};
            }
            template<typename Rng0, typename Rng1, typename Rng2>
            constexpr auto operator()(Rng0 && rng0, Rng1 && rng1, Rng2 && rng2) const
                -> CPP_ret(zip_view<all_t<Rng0>, all_t<Rng1>, all_t<Rng2>>)( //
                    requires input_range<Rng0> && viewable_range<Rng0> &&    //
                             input_range<Rng1> && viewable_range<Rng1> &&    //
                             input_range<Rng2> && viewable_range<Rng2>)
            {
                return zip_view<all_t<Rng0>, all_t<Rng1>, all_t<Rng2>>{ //
                    all(static_cast<Rng0 &&>(rng0)),                    //
                    all(static_cast<Rng1 &&>(rng1)),                    //
                    all(static_cast<Rng2 &&>(rng2))};
            }
#endif
        };

        /// \relates zip_fn
        /// \ingroup group-views
        RANGES_INLINE_VARIABLE(zip_fn, zip)
    } // namespace views
    /// @}
} // namespace ranges

#include <range/v3/detail/satisfy_boost_range.hpp>
RANGES_SATISFY_BOOST_RANGE(::ranges::zip_view)

#include <range/v3/detail/reenable_warnings.hpp>

#endif
