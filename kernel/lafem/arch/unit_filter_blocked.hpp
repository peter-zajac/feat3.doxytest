// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2023 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_LAFEM_ARCH_UNIT_FILTER_BLOCKED_HPP
#define KERNEL_LAFEM_ARCH_UNIT_FILTER_BLOCKED_HPP 1

// includes, FEAT
#include <kernel/base_header.hpp>
#include <kernel/util/runtime.hpp>

/// \cond internal
namespace FEAT
{
  namespace LAFEM
  {
    namespace Arch
    {
      struct UnitFilterBlocked
      {
        template <typename DT_, typename IT_, int BlockSize_>
        static void filter_rhs(DT_ * v, const DT_ * const sv_elements, const IT_ * const sv_indices, const Index ue)
        {
          filter_rhs_generic<DT_, IT_, BlockSize_>(v, sv_elements, sv_indices, ue);
        }

        template <int BlockSize_>
        static void filter_rhs(float * v, const float * const sv_elements, const unsigned long * const sv_indices, const Index ue)
        {
          constexpr auto filter_rhs_generic_float_ulong = &filter_rhs_generic<float, unsigned long, BlockSize_>;
          constexpr auto filter_rhs_cuda_float_ulong = &filter_rhs_cuda<float, unsigned long, BlockSize_>;
          BACKEND_SKELETON_VOID(filter_rhs_cuda_float_ulong, filter_rhs_generic_float_ulong, filter_rhs_generic_float_ulong, v, sv_elements, sv_indices, ue)
        }

        template <int BlockSize_>
        static void filter_rhs(double * v, const double * const sv_elements, const unsigned long * const sv_indices, const Index ue)
        {
          constexpr auto filter_rhs_generic_double_ulong = &filter_rhs_generic<double, unsigned long, BlockSize_>;
          constexpr auto filter_rhs_cuda_double_ulong = &filter_rhs_cuda<double, unsigned long, BlockSize_>;
          BACKEND_SKELETON_VOID(filter_rhs_cuda_double_ulong, filter_rhs_generic_double_ulong, filter_rhs_generic_double_ulong, v, sv_elements, sv_indices, ue)
        }

        template <int BlockSize_>
        static void filter_rhs(float * v, const float * const sv_elements, const unsigned int * const sv_indices, const Index ue)
        {
          constexpr auto filter_rhs_generic_float_uint = &filter_rhs_generic<float, unsigned int, BlockSize_>;
          constexpr auto filter_rhs_cuda_float_uint = &filter_rhs_cuda<float, unsigned int, BlockSize_>;
          BACKEND_SKELETON_VOID(filter_rhs_cuda_float_uint, filter_rhs_generic_float_uint, filter_rhs_generic_float_uint, v, sv_elements, sv_indices, ue)
        }

        template <int BlockSize_>
        static void filter_rhs(double * v, const double * const sv_elements, const unsigned int * const sv_indices, const Index ue)
        {
          constexpr auto filter_rhs_generic_double_uint = &filter_rhs_generic<double, unsigned int, BlockSize_>;
          constexpr auto filter_rhs_cuda_double_uint = &filter_rhs_cuda<double, unsigned int, BlockSize_>;
          BACKEND_SKELETON_VOID(filter_rhs_cuda_double_uint, filter_rhs_generic_double_uint, filter_rhs_generic_double_uint, v, sv_elements, sv_indices, ue)
        }

        template <typename DT_, typename IT_, int BlockSize_>
        static void filter_def(DT_ * v, const IT_ * const sv_indices, const Index ue)
        {
          filter_def_generic<DT_, IT_, BlockSize_>(v, sv_indices, ue);
        }

        template <int BlockSize_>
        static void filter_def(float * v, const float * const sv_elements, const unsigned long * const sv_indices, const Index ue)
        {
          constexpr auto filter_def_generic_float_ulong = &filter_def_generic<float, unsigned long, BlockSize_>;
          constexpr auto filter_def_cuda_float_ulong = &filter_def_cuda<float, unsigned long, BlockSize_>;
          BACKEND_SKELETON_VOID(filter_def_cuda_float_ulong, filter_def_generic_float_ulong, filter_def_generic_float_ulong, v, sv_elements, sv_indices, ue)
        }

        template <int BlockSize_>
        static void filter_def(double * v, const double * const sv_elements, const unsigned long * const sv_indices, const Index ue)
        {
          constexpr auto filter_def_generic_double_ulong = &filter_def_generic<double, unsigned long, BlockSize_>;
          constexpr auto filter_def_cuda_double_ulong = &filter_def_cuda<double, unsigned long, BlockSize_>;
          BACKEND_SKELETON_VOID(filter_def_cuda_double_ulong, filter_def_generic_double_ulong, filter_def_generic_double_ulong, v, sv_elements, sv_indices, ue)
        }

        template <int BlockSize_>
        static void filter_def(float * v, const float * const sv_elements, const unsigned int * const sv_indices, const Index ue)
        {
          constexpr auto filter_def_generic_float_uint = &filter_def_generic<float, unsigned int, BlockSize_>;
          constexpr auto filter_def_cuda_float_uint = &filter_def_cuda<float, unsigned int, BlockSize_>;
          BACKEND_SKELETON_VOID(filter_def_cuda_float_uint, filter_def_generic_float_uint, filter_def_generic_float_uint, v, sv_elements, sv_indices, ue)
        }

        template <int BlockSize_>
        static void filter_def(double * v, const double * const sv_elements, const unsigned int * const sv_indices, const Index ue)
        {
          constexpr auto filter_def_generic_double_uint = &filter_def_generic<double, unsigned int, BlockSize_>;
          constexpr auto filter_def_cuda_double_uint = &filter_def_cuda<double, unsigned int, BlockSize_>;
          BACKEND_SKELETON_VOID(filter_def_cuda_double_uint, filter_def_generic_double_uint, filter_def_generic_double_uint, v, sv_elements, sv_indices, ue)
        }

        template <typename DT_, typename IT_, int BlockSize_>
        static void filter_rhs_generic(DT_ * v, const DT_ * const sv_elements, const IT_ * const sv_indices, const Index ue);

        template <typename DT_, typename IT_, int BlockSize_>
        static void filter_def_generic(DT_ * v, const IT_ * const sv_indices, const Index ue);

        template <typename DT_, typename IT_, int BlockSize_>
        static void filter_rhs_cuda(DT_ * v, const DT_ * const sv_elements, const IT_ * const sv_indices, const Index ue);

        template <typename DT_, typename IT_, int BlockSize_>
        static void filter_def_cuda(DT_ * v, const IT_ * const sv_indices, const Index ue);
      };

      // Do not instantiate the following templates as this is done in unit_filter_blocked_generic.cpp and then linked
      // into the shared library
#ifdef FEAT_EICKT
      extern template void UnitFilterBlocked::filter_rhs_generic<float, unsigned long, 2>(float * v, const float * const sv_elements, const unsigned long * const sv_indices, const Index ue);
      extern template void UnitFilterBlocked::filter_rhs_generic<double, unsigned long, 2>(double * v, const double * const sv_elements, const unsigned long * const sv_indices, const Index ue);
      extern template void UnitFilterBlocked::filter_rhs_generic<float, unsigned int, 2>(float * v, const float * const sv_elements, const unsigned int * const sv_indices, const Index ue);
      extern template void UnitFilterBlocked::filter_rhs_generic<double, unsigned int, 2>(double * v, const double * const sv_elements, const unsigned int * const sv_indices, const Index ue);

      extern template void UnitFilterBlocked::filter_def_generic<float, unsigned long, 2>(float * v, const unsigned long * const sv_indices, const Index ue);
      extern template void UnitFilterBlocked::filter_def_generic<double, unsigned long, 2>(double * v, const unsigned long * const sv_indices, const Index ue);
      extern template void UnitFilterBlocked::filter_def_generic<float, unsigned int, 2>(float * v, const unsigned int * const sv_indices, const Index ue);
      extern template void UnitFilterBlocked::filter_def_generic<double, unsigned int, 2>(double * v, const unsigned int * const sv_indices, const Index ue);

      extern template void UnitFilterBlocked::filter_rhs_generic<float, unsigned long, 3>(float * v, const float * const sv_elements, const unsigned long * const sv_indices, const Index ue);
      extern template void UnitFilterBlocked::filter_rhs_generic<double, unsigned long, 3>(double * v, const double * const sv_elements, const unsigned long * const sv_indices, const Index ue);
      extern template void UnitFilterBlocked::filter_rhs_generic<float, unsigned int, 3>(float * v, const float * const sv_elements, const unsigned int * const sv_indices, const Index ue);
      extern template void UnitFilterBlocked::filter_rhs_generic<double, unsigned int, 3>(double * v, const double * const sv_elements, const unsigned int * const sv_indices, const Index ue);

      extern template void UnitFilterBlocked::filter_def_generic<float, unsigned long, 3>(float * v, const unsigned long * const sv_indices, const Index ue);
      extern template void UnitFilterBlocked::filter_def_generic<double, unsigned long, 3>(double * v, const unsigned long * const sv_indices, const Index ue);
      extern template void UnitFilterBlocked::filter_def_generic<float, unsigned int, 3>(float * v, const unsigned int * const sv_indices, const Index ue);
      extern template void UnitFilterBlocked::filter_def_generic<double, unsigned int, 3>(double * v, const unsigned int * const sv_indices, const Index ue);
#endif

    } // namespace Arch
  } // namespace LAFEM
} // namespace FEAT

/// \endcond
#ifndef  __CUDACC__
#include <kernel/lafem/arch/unit_filter_blocked_generic.hpp>
#endif
#endif // KERNEL_LAFEM_ARCH_UNIT_FILTER_BLOCKED_HPP
