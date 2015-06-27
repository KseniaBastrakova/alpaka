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

// Base classes.
#include <alpaka/workdiv/WorkDivCudaBuiltIn.hpp>    // WorkDivCudaBuiltIn
#include <alpaka/idx/gb/IdxGbCudaBuiltIn.hpp>       // IdxGbCudaBuiltIn
#include <alpaka/idx/bt/IdxBtCudaBuiltIn.hpp>       // IdxBtCudaBuiltIn
#include <alpaka/atomic/AtomicCudaBuiltIn.hpp>      // AtomicCudaBuiltIn
#include <alpaka/math/MathCudaBuiltIn.hpp>          // MathCudaBuiltIn

// Specialized traits.
#include <alpaka/acc/Traits.hpp>                    // AccType
#include <alpaka/dev/Traits.hpp>                    // DevType
#include <alpaka/exec/Traits.hpp>                   // ExecType

// Implementation details.
#include <alpaka/dev/DevCudaRt.hpp>                 // DevCudaRt
#include <alpaka/core/Cuda.hpp>                     // ALPAKA_CUDA_RT_CHECK

#include <boost/predef.h>                           // workarounds
#include <boost/align.hpp>                          // boost::aligned_alloc

namespace alpaka
{
    namespace exec
    {
        template<
            typename TDim>
        class ExecGpuCuda;
    }
    namespace acc
    {
        //-----------------------------------------------------------------------------
        //! The GPU CUDA accelerator.
        //-----------------------------------------------------------------------------
        namespace cuda
        {
            //-----------------------------------------------------------------------------
            //! The GPU CUDA accelerator implementation details.
            //-----------------------------------------------------------------------------
            namespace detail
            {
                // Forward declarations.
                /*template<
                    typename TKernelFunctor,
                    typename... TArgs>
                __global__ void cudaKernel(
                    TKernelFunctor kernelFunctor,
                    TArgs ... args);*/

                //#############################################################################
                //! The GPU CUDA accelerator.
                //!
                //! This accelerator allows parallel kernel execution on devices supporting CUDA.
                //#############################################################################
                template<
                    typename TDim>
                class AccGpuCuda final :
                    public workdiv::WorkDivCudaBuiltIn<TDim>,
                    public idx::gb::IdxGbCudaBuiltIn<TDim>,
                    public idx::bt::IdxBtCudaBuiltIn<TDim>,
                    public atomic::AtomicCudaBuiltIn,
                    public math::MathCudaBuiltIn
                {
                public:
                    /*template<
                        typename TKernelFunctor,
                        typename... TArgs>
                    friend void ::alpaka::cuda::detail::cudaKernel(
                        TKernelFunctor kernelFunctor,
                        TArgs ... args);*/

                    //friend class ::alpaka::cuda::detail::ExecGpuCuda<TDim>;

                //private:
                    //-----------------------------------------------------------------------------
                    //! Constructor.
                    //-----------------------------------------------------------------------------
                    ALPAKA_FCT_ACC_CUDA_ONLY AccGpuCuda() :
                        workdiv::WorkDivCudaBuiltIn<TDim>(),
                        idx::gb::IdxGbCudaBuiltIn<TDim>(),
                        idx::bt::IdxBtCudaBuiltIn<TDim>(),
                        atomic::AtomicCudaBuiltIn(),
                        math::MathCudaBuiltIn()
                    {}

                public:
                    //-----------------------------------------------------------------------------
                    //! Copy constructor.
                    //-----------------------------------------------------------------------------
                    ALPAKA_FCT_ACC_CUDA_ONLY AccGpuCuda(AccGpuCuda const &) = delete;
                    //-----------------------------------------------------------------------------
                    //! Move constructor.
                    //-----------------------------------------------------------------------------
                    ALPAKA_FCT_ACC_CUDA_ONLY AccGpuCuda(AccGpuCuda &&) = delete;
                    //-----------------------------------------------------------------------------
                    //! Copy assignment operator.
                    //-----------------------------------------------------------------------------
                    ALPAKA_FCT_ACC_CUDA_ONLY auto operator=(AccGpuCuda const &) -> AccGpuCuda & = delete;
                    //-----------------------------------------------------------------------------
                    //! Move assignment operator.
                    //-----------------------------------------------------------------------------
                    ALPAKA_FCT_ACC_CUDA_ONLY auto operator=(AccGpuCuda &&) -> AccGpuCuda & = delete;
                    //-----------------------------------------------------------------------------
                    //! Destructor.
                    //-----------------------------------------------------------------------------
                    ALPAKA_FCT_ACC_CUDA_ONLY ~AccGpuCuda() = default;

                    //-----------------------------------------------------------------------------
                    //! Syncs all threads in the current block.
                    //-----------------------------------------------------------------------------
                    ALPAKA_FCT_ACC_CUDA_ONLY auto syncBlockThreads() const
                    -> void
                    {
                        __syncthreads();
                    }

                    //-----------------------------------------------------------------------------
                    //! \return Allocates block shared memory.
                    //-----------------------------------------------------------------------------
                    template<
                        typename T,
                        UInt TuiNumElements>
                    ALPAKA_FCT_ACC_CUDA_ONLY auto allocBlockSharedMem() const
                    -> T *
                    {
                        static_assert(TuiNumElements > 0, "The number of elements to allocate in block shared memory must not be zero!");

                        __shared__ T shMem[TuiNumElements];
                        return shMem;
                    }

                    //-----------------------------------------------------------------------------
                    //! \return The pointer to the externally allocated block shared memory.
                    //-----------------------------------------------------------------------------
                    template<
                        typename T>
                    ALPAKA_FCT_ACC_CUDA_ONLY auto getBlockSharedExternMem() const
                    -> T *
                    {
                        // Because unaligned access to variables is not allowed in device code,
                        // we have to use the widest possible type to have all types aligned correctly.
                        // See: http://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#shared
                        // http://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#vector-types
                        extern __shared__ float4 shMem[];
                        return reinterpret_cast<T *>(shMem);
                    }
                };
            }
        }
    }

    template<
        typename TDim>
    using AccGpuCuda = acc::cuda::detail::AccGpuCuda<TDim>;

    namespace acc
    {
        namespace traits
        {
            //#############################################################################
            //! The GPU CUDA accelerator accelerator type trait specialization.
            //#############################################################################
            template<
                typename TDim>
            struct AccType<
                acc::cuda::detail::AccGpuCuda<TDim>>
            {
                using type = acc::cuda::detail::AccGpuCuda<TDim>;
            };
            //#############################################################################
            //! The GPU CUDA accelerator device properties get trait specialization.
            //#############################################################################
            template<
                typename TDim>
            struct GetAccDevProps<
                acc::cuda::detail::AccGpuCuda<TDim>>
            {
                ALPAKA_FCT_HOST static auto getAccDevProps(
                    dev::DevCudaRt const & dev)
                -> alpaka::acc::AccDevProps<TDim>
                {
                    cudaDeviceProp cudaDevProp;
                    ALPAKA_CUDA_RT_CHECK(cudaGetDeviceProperties(
                        &cudaDevProp,
                        dev.m_iDevice));

                    return {
                        // m_uiMultiProcessorCount
                        static_cast<Uint>(cudaDevProp.multiProcessorCount),
                        // m_uiBlockThreadsCountMax
                        static_cast<Uint>(cudaDevProp.maxThreadsPerBlock),
                        // m_vuiBlockThreadExtentsMax
                        alpaka::extent::getExtentsVecNd<TDim, Uint>(cudaDevProp.maxThreadsDim),
                        // m_vuiGridBlockExtentsMax
                        alpaka::extent::getExtentsVecNd<TDim, Uint>(cudaDevProp.maxGridSize)};
                }
            };
            //#############################################################################
            //! The GPU CUDA accelerator name trait specialization.
            //#############################################################################
            template<
                typename TDim>
            struct GetAccName<
                acc::cuda::detail::AccGpuCuda<TDim>>
            {
                ALPAKA_FCT_HOST_ACC static auto getAccName()
                -> std::string
                {
                    return "AccGpuCuda<" + std::to_string(TDim::value) + ">";
                }
            };
        }
    }
    namespace dev
    {
        namespace traits
        {
            //#############################################################################
            //! The GPU CUDA accelerator device type trait specialization.
            //#############################################################################
            template<
                typename TDim>
            struct DevType<
                acc::cuda::detail::AccGpuCuda<TDim>>
            {
                using type = dev::DevCudaRt;
            };
            //#############################################################################
            //! The GPU CUDA accelerator device manager type trait specialization.
            //#############################################################################
            template<
                typename TDim>
            struct DevManType<
                acc::cuda::detail::AccGpuCuda<TDim>>
            {
                using type = dev::DevManCudaRt;
            };
        }
    }
    namespace dim
    {
        namespace traits
        {
            //#############################################################################
            //! The GPU CUDA accelerator dimension getter trait specialization.
            //#############################################################################
            template<
                typename TDim>
            struct DimType<
                acc::cuda::detail::AccGpuCuda<TDim>>
            {
                using type = TDim;
            };
        }
    }
    namespace exec
    {
        namespace traits
        {
            //#############################################################################
            //! The GPU CUDA accelerator executor type trait specialization.
            //#############################################################################
            template<
                typename TDim>
            struct ExecType<
                acc::cuda::detail::AccGpuCuda<TDim>>
            {
                using type = exec::ExecGpuCuda<TDim>;
            };
        }
    }
}