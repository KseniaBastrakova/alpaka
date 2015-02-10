/**
* \file
* Copyright 2014-2015 Benjamin Worpitz
*
* This file is part of alpaka.
*
* alpaka is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* alpaka is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with alpaka.
* If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <alpaka/interfaces/IAcc.hpp>        // IAcc

#include <alpaka/core/WorkDivHelpers.hpp>    // workdiv::isValidWorkDiv

namespace alpaka
{
    namespace detail
    {
        //#############################################################################
        //! Extensions to the standard library.
        //#############################################################################
        namespace std_extension
        {
            // This could be replaced with c++14 std::integer_sequence if we raise the minimum.
            template<class T, T... TVals>
            struct integer_sequence
            {
                static_assert(std::is_integral<T>::value, "integer_sequence<T, I...> requires T to be an integral type.");

                typedef integer_sequence<T, TVals...> type;
                typedef T value_type;

                static std::size_t size() noexcept
                {
                    return (sizeof...(TVals));
                }
            };

            // Bug: https://connect.microsoft.com/VisualStudio/feedback/details/1085630/template-alias-internal-error-in-the-compiler-because-of-tmp-c-integer-sequence-for-c-11
#if (BOOST_COMP_MSVC) && (BOOST_COMP_MSVC < BOOST_VERSION_NUMBER(14, 0, 0))
            template<bool TbNegative, bool TbZero, class TIntCon, class TIntSeq>
            struct make_integer_sequence_helper
            {
                static_assert(!TbNegative, "make_integer_sequence<T, N> requires N to be non-negative.");
            };

            template<class T, T... TVals>
            struct make_integer_sequence_helper<false, true, std::integral_constant<T, 0>, integer_sequence<T, TVals...> > :
                integer_sequence<T, TVals...>
            {};

            template<class T, T TIdx, T... TVals>
            struct make_integer_sequence_helper<false, false, std::integral_constant<T, TIdx>, integer_sequence<T, TVals...> > :
                make_integer_sequence_helper<false, TIdx == 1, std::integral_constant<T, TIdx - 1>, integer_sequence<T, TIdx - 1, TVals...> >
            {};

            template<class T, T TuiSize>
            struct make_integer_sequence
            {
                using type = typename make_integer_sequence_helper<(TuiSize < 0), (TuiSize == 0), std::integral_constant<T, TuiSize>, integer_sequence<T> >::type;
            };
#else
            template<bool TbNegative, bool TbZero, class TIntCon, class TIntSeq>
            struct make_integer_sequence_helper
            {
                static_assert(!TbNegative, "make_integer_sequence<T, N> requires N to be non-negative.");
            };

            template<class T, T... TVals>
            struct make_integer_sequence_helper<false, true, std::integral_constant<T, 0>, integer_sequence<T, TVals...> > :
                integer_sequence<T, TVals...>
            {};

            template<class T, T TIdx, T... TVals>
            struct make_integer_sequence_helper<false, false, std::integral_constant<T, TIdx>, integer_sequence<T, TVals...> > :
                make_integer_sequence_helper<false, TIdx == 1, std::integral_constant<T, TIdx - 1>, integer_sequence<T, TIdx - 1, TVals...> >
            {};

            template<class T, T TuiSize>
            using make_integer_sequence = typename make_integer_sequence_helper<(TuiSize < 0), (TuiSize == 0), std::integral_constant<T, TuiSize>, integer_sequence<T> >::type;
#endif

            template<std::size_t... TVals>
            using index_sequence = integer_sequence<std::size_t, TVals...>;

            template<std::size_t TuiSize>
            using make_index_sequence = make_integer_sequence<std::size_t, TuiSize>;

            template<typename... Ts>
            using index_sequence_for = make_index_sequence<sizeof...(Ts)>;
        }

        //#############################################################################
        //! The executor for an accelerated serial kernel.
        //#############################################################################
        template<
            typename TKernelExecutor, 
            typename... TKernelConstrArgs>
        class KernelExecutorExtent
        {
            using Acc = acc::GetAccT<TKernelExecutor>;
            using Stream = stream::GetStreamT<Acc>;
        public:
            //-----------------------------------------------------------------------------
            //! Constructor.
            //-----------------------------------------------------------------------------
            ALPAKA_FCT_HOST KernelExecutorExtent(TKernelConstrArgs && ... args) :
                m_tupleKernelConstrArgs(std::forward<TKernelConstrArgs>(args)...)
            {}
            //-----------------------------------------------------------------------------
            //! Copy constructor.
            //-----------------------------------------------------------------------------
            ALPAKA_FCT_HOST KernelExecutorExtent(KernelExecutorExtent const &) = default;
#if (!BOOST_COMP_MSVC) || (BOOST_COMP_MSVC >= BOOST_VERSION_NUMBER(14, 0, 0))
            //-----------------------------------------------------------------------------
            //! Move constructor.
            //-----------------------------------------------------------------------------
            ALPAKA_FCT_HOST KernelExecutorExtent(KernelExecutorExtent &&) = default;
#endif
            //-----------------------------------------------------------------------------
            //! Copy assignment.
            //-----------------------------------------------------------------------------
            ALPAKA_FCT_HOST KernelExecutorExtent & operator=(KernelExecutorExtent const &) = delete;
            //-----------------------------------------------------------------------------
            //! Destructor.
            //-----------------------------------------------------------------------------
            ALPAKA_FCT_HOST virtual ~KernelExecutorExtent() noexcept = default;

            //-----------------------------------------------------------------------------
            //! \return An KernelExecutor with the given extents.
            //-----------------------------------------------------------------------------
            template<
                typename TWorkDiv>
            ALPAKA_FCT_HOST TKernelExecutor operator()(
                TWorkDiv const & workDiv, 
                Stream const & stream) const
            {
                if(!workdiv::isValidWorkDiv<Acc>(workDiv))
                {
                    throw std::runtime_error("The given work division is not supported by the " + acc::getAccName<Acc>() + " accelerator!");
                }

                return createKernelExecutor(
                    workDiv, 
                    stream, 
                    KernelConstrArgsIdxSequence());
            }
            //-----------------------------------------------------------------------------
            //! \return An KernelExecutor with the given extents.
            //-----------------------------------------------------------------------------
            template<
                typename TGridBlocksExtents,
                typename TBlockKernelsExtents>
            ALPAKA_FCT_HOST TKernelExecutor operator()(
                TGridBlocksExtents const & gridBlocksExtent,
                TBlockKernelsExtents const & blockKernelsExtents, 
                Stream const & stream) const
            {
                return this->operator()(
                    workdiv::BasicWorkDiv(
                        Vec<3u>(extent::getWidth(gridBlocksExtent), extent::getHeight(gridBlocksExtent), extent::getDepth(gridBlocksExtent)),
                        Vec<3u>(extent::getWidth(blockKernelsExtents), extent::getHeight(blockKernelsExtents), extent::getDepth(blockKernelsExtents))), 
                    stream);
            }

        private:
            //-----------------------------------------------------------------------------
            //! \return An KernelExecutor with the given extents.
            //-----------------------------------------------------------------------------
            template<
                typename TWorkDiv,
                std::size_t... TIndices>
            ALPAKA_FCT_HOST TKernelExecutor createKernelExecutor(
                TWorkDiv const & workDiv, 
                Stream const & stream,
#if !BOOST_COMP_MSVC     // MSVC 190022512 introduced a new bug with alias templates: error C3520: 'TIndices': parameter pack must be expanded in this context
                std_extension::index_sequence<TIndices...> const &) const
#else
                std_extension::integer_sequence<std::size_t, TIndices...> const &) const
#endif
            {
                if(workdiv::getWorkDiv<Grid, Blocks, dim::Dim1>(workDiv)[0] == 0u)
                {
                    throw std::runtime_error("The workDiv grid blocks extents is not allowed to be zero in any dimension!");
                }
                if(workdiv::getWorkDiv<Block, Kernels, dim::Dim1>(workDiv)[0] == 0u)
                {
                    throw std::runtime_error("The workDiv block kernels extents is not allowed to be zero in any dimension!");
                }

                return TKernelExecutor(workDiv, stream, std::forward<TKernelConstrArgs>(std::get<TIndices>(m_tupleKernelConstrArgs))...);
            }

        private:
            std::tuple<TKernelConstrArgs...> m_tupleKernelConstrArgs;
#if (BOOST_COMP_MSVC) && (BOOST_COMP_MSVC < BOOST_VERSION_NUMBER(14, 0, 0))
            using KernelConstrArgsIdxSequence = typename std_extension::make_index_sequence<sizeof...(TKernelConstrArgs)>::type;
#else
            using KernelConstrArgsIdxSequence = std_extension::make_index_sequence<sizeof...(TKernelConstrArgs)>;
#endif
        };

        //#############################################################################
        //! The kernel executor builder.
        //#############################################################################
        template<
            typename TAcc, 
            typename TKernel, 
            typename... TKernelConstrArgs>
        class KernelExecCreator;
    }

    //#############################################################################
    //! Builds a kernel executor.
    //!
    //! Requirements for type TKernel:
    //! The kernel type has to be inherited from 'alpaka::IAcc<boost::mpl::_1>'directly.
    //! All template parameters have to be types. No value parameters are allowed. Use boost::mpl::int_ or similar to use values.
    //#############################################################################
    template<
        typename TAcc, 
        typename TKernel, 
        typename... TKernelConstrArgs>
    auto createKernelExecutor(
        TKernelConstrArgs && ... args)
    -> typename std::result_of<detail::KernelExecCreator<TAcc, TKernel, TKernelConstrArgs...>(TKernelConstrArgs...)>::type
    {
#if (!BOOST_COMP_GNUC) || (BOOST_COMP_GNUC >= BOOST_VERSION_NUMBER(5, 0, 0))
        static_assert(std::is_trivially_copyable<TKernel>::value, "The given kernel functor has to fulfill is_trivially_copyable!");
#endif

        // Use the specialized KernelExecCreator for the given accelerator.
        return detail::KernelExecCreator<TAcc, TKernel, TKernelConstrArgs...>()(std::forward<TKernelConstrArgs>(args)...);
    }
}
