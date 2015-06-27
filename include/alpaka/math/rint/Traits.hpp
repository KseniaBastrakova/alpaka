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

#include <alpaka/core/Common.hpp>   // ALPAKA_FCT_HOST_ACC

#include <type_traits>              // std::enable_if, std::is_base_of, std::is_same, std::decay

namespace alpaka
{
    namespace math
    {
        namespace traits
        {
            //#############################################################################
            //! The rint trait.
            //#############################################################################
            template<
                typename T,
                typename TArg,
                typename TSfinae = void>
            struct Rint;

            //#############################################################################
            //! The rint trait.
            //#############################################################################
            template<
                typename T,
                typename TArg,
                typename TSfinae = void>
            struct Lrint;

            //#############################################################################
            //! The rint trait.
            //#############################################################################
            template<
                typename T,
                typename TArg,
                typename TSfinae = void>
            struct Llrint;
        }

        //-----------------------------------------------------------------------------
        //! Computes the nearest integer value to arg (in floating-point format), rinting halfway cases away from zero, regardless of the current rinting mode.
        //!
        //! \tparam T The type of the object specializing Rint.
        //! \tparam TArg The arg type.
        //! \param rint The object specializing Rint.
        //! \param arg The arg.
        //-----------------------------------------------------------------------------
        template<
            typename T,
            typename TArg>
        ALPAKA_FCT_HOST_ACC auto rint(
            T const & rint,
            TArg const & arg)
        -> decltype(
            traits::Rint<
                T,
                TArg>
            ::rint(
                rint,
                arg))
        {
            return traits::Rint<
                T,
                TArg>
            ::rint(
                rint,
                arg);
        }
        //-----------------------------------------------------------------------------
        //! Computes the nearest integer value to arg (in integer format), rinting halfway cases away from zero, regardless of the current rinting mode.
        //!
        //! \tparam T The type of the object specializing Rint.
        //! \tparam TArg The arg type.
        //! \param rint The object specializing Rint.
        //! \param arg The arg.
        //-----------------------------------------------------------------------------
        template<
            typename T,
            typename TArg>
        ALPAKA_FCT_HOST_ACC auto lrint(
            T const & lrint,
            TArg const & arg)
        -> long int
        {
            return traits::Lrint<
                T,
                TArg>
            ::lrint(
                lrint,
                arg);
        }
        //-----------------------------------------------------------------------------
        //! Computes the nearest integer value to arg (in integer format), rinting halfway cases away from zero, regardless of the current rinting mode.
        //!
        //! \tparam T The type of the object specializing Rint.
        //! \tparam TArg The arg type.
        //! \param rint The object specializing Rint.
        //! \param arg The arg.
        //-----------------------------------------------------------------------------
        template<
            typename T,
            typename TArg>
        ALPAKA_FCT_HOST_ACC auto llrint(
            T const & llrint,
            TArg const & arg)
        -> long long int
        {
            return traits::Llrint<
                T,
                TArg>
            ::llrint(
                llrint,
                arg);
        }

        namespace traits
        {
            //#############################################################################
            //! The Rint specialization for classes with RintBase member type.
            //#############################################################################
            template<
                typename T,
                typename TArg>
            struct Rint<
                T,
                TArg,
                typename std::enable_if<
                    std::is_base_of<typename T::RintBase, typename std::decay<T>::type>::value
                    && (!std::is_same<typename T::RintBase, typename std::decay<T>::type>::value)>::type>
            {
                //-----------------------------------------------------------------------------
                //
                //-----------------------------------------------------------------------------
                ALPAKA_FCT_HOST_ACC static auto rint(
                    T const & rint,
                    TArg const & arg)
                -> decltype(
                    math::rint(
                        static_cast<typename T::RintBase const &>(rint),
                        arg))
                {
                    // Delegate the call to the base class.
                    return
                        math::rint(
                            static_cast<typename T::RintBase const &>(rint),
                            arg);
                }
            };
            //#############################################################################
            //! The Lrint specialization for classes with RintBase member type.
            //#############################################################################
            template<
                typename T,
                typename TArg>
            struct Lrint<
                T,
                TArg,
                typename std::enable_if<
                    std::is_base_of<typename T::RintBase, typename std::decay<T>::type>::value
                    && (!std::is_same<typename T::RintBase, typename std::decay<T>::type>::value)>::type>
            {
                //-----------------------------------------------------------------------------
                //
                //-----------------------------------------------------------------------------
                ALPAKA_FCT_HOST_ACC static auto lrint(
                    T const & lrint,
                    TArg const & arg)
                -> decltype(
                    math::lrint(
                        static_cast<typename T::RintBase const &>(lrint),
                        arg))
                {
                    // Delegate the call to the base class.
                    return
                        math::lrint(
                            static_cast<typename T::RintBase const &>(lrint),
                            arg);
                }
            };
            //#############################################################################
            //! The Llrint specialization for classes with RintBase member type.
            //#############################################################################
            template<
                typename T,
                typename TArg>
            struct Llrint<
                T,
                TArg,
                typename std::enable_if<
                    std::is_base_of<typename T::RintBase, typename std::decay<T>::type>::value
                    && (!std::is_same<typename T::RintBase, typename std::decay<T>::type>::value)>::type>
            {
                //-----------------------------------------------------------------------------
                //
                //-----------------------------------------------------------------------------
                ALPAKA_FCT_HOST_ACC static auto llrint(
                    T const & llrint,
                    TArg const & arg)
                -> decltype(
                    math::llrint(
                        static_cast<typename T::RintBase const &>(llrint),
                        arg))
                {
                    // Delegate the call to the base class.
                    return
                        math::llrint(
                            static_cast<typename T::RintBase const &>(llrint),
                            arg);
                }
            };
        }
    }
}