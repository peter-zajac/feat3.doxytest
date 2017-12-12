#pragma once
#ifndef KERNEL_UTIL_TYPE_TRAITS_HPP
#define KERNEL_UTIL_TYPE_TRAITS_HPP 1

// includes, FEAT
#include <kernel/util/string.hpp>

#include <typeindex>

#if defined(FEAT_HAVE_HALFMATH) && !defined(__CUDACC__)
FEAT_DISABLE_WARNINGS
#define HALF_ROUND_STYLE 2
//#define HALF_ROUND_TIES_TO_EVEN 1
#include <half.hpp>
FEAT_RESTORE_WARNINGS
#endif // FEAT_HAVE_HALFMATH $$ !defined(__CUDACC__)

namespace FEAT
{
  /**
   * \brief Type namespace
   *
   * This namespace encapsulates the TypeTraits class template as well as tag classes used by it.
   */
  namespace Type
  {
    struct Helper
    {
      /// extracts sizeof datatype from a given types feature hash
      static inline size_t extract_type_size(uint64_t feature_hash)
      {
        return (size_t) feature_hash&0xFFFFFFFF; //take only the lower 32 bits
      }

      /// extracts integral feature from a given types feature hash
      static inline bool extract_intness(uint64_t feature_hash)
      {
        return (feature_hash&(uint64_t(1)<<32)) != uint64_t(0);
      }

      /// extracts floating point feature from a given types feature hash
      static inline bool extract_floatness(uint64_t feature_hash)
      {
        return (feature_hash&(uint64_t(1)<<33)) != uint64_t(0);
      }

      /// extracts sign feature from a given types feature hash
      static inline bool extract_signedness(uint64_t feature_hash)
      {
        return (feature_hash&(uint64_t(1)<<34)) != uint64_t(0);
      }
    };

    /// Tag class for any data type not matching any other type class
    class AnotherClass {};
    /// Tag class for integral data types
    class IntegralClass {};
    /// Tag class for floating data types
    class FloatingClass {};
    /// Tag class for the one and only boolean data type
    class BooleanClass {};

    /**
     * \brief basic Type Traits struct
     *
     * \author Dirk Ribbrock
     */
    template<typename DT_>
    struct Traits
    {
      /// this type is of another class
      typedef AnotherClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return DT_::name();
      }
    };

    /**
     * \brief Type Traits specialisation for <c>float</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<float>
    {
      /// this type is not integral
      static constexpr bool is_int = false;
      /// this type is floating
      static constexpr bool is_float = true;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is signed
      static constexpr bool is_signed = true;

      /// this type is of floating class
      typedef FloatingClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "float";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(float)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>double</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<double>
    {
      /// this type is not integral
      static constexpr bool is_int = false;
      /// this type is floating
      static constexpr bool is_float = true;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is signed
      static constexpr bool is_signed = true;

      /// this type is of floating class
      typedef FloatingClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "double";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(double)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>long double</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<long double>
    {
      /// this type is not integral
      static constexpr bool is_int = false;
      /// this type is floating
      static constexpr bool is_float = true;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is signed
      static constexpr bool is_signed = true;

      /// this type is of floating class
      typedef FloatingClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "long double";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(long double)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>unsigned int</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<unsigned int>
    {
      /// this type is  integral
      static constexpr bool is_int = true;
      /// this type is not floating
      static constexpr bool is_float = false;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is unsigned
      static constexpr bool is_signed = false;

      /// this type is of integral class
      typedef IntegralClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "unsigned int";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(unsigned int)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>signed int</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<signed int>
    {
      /// this type is integral
      static constexpr bool is_int = true;
      /// this type is not floating
      static constexpr bool is_float = false;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is signed
      static constexpr bool is_signed = true;

      /// this type is of integral class
      typedef IntegralClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "signed int";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(signed int)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>unsigned char</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<unsigned char>
    {
      /// this type is integral
      static constexpr bool is_int = true;
      /// this type is not floating
      static constexpr bool is_float = false;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is unsigned
      static constexpr bool is_signed = false;

      /// this type is of integral class
      typedef IntegralClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "unsigned char";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(unsigned char)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>signed char</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<signed char>
    {
      /// this type is integral
      static constexpr bool is_int = true;
      /// this type is not floating
      static constexpr bool is_float = false;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is signed
      static constexpr bool is_signed = true;

      /// this type is of integral class
      typedef IntegralClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "signed char";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(signed char)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>unsigned short</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<unsigned short>
    {
      /// this type is integral
      static constexpr bool is_int = true;
      /// this type is not floating
      static constexpr bool is_float = false;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is unsigned
      static constexpr bool is_signed = false;

      /// this type is of integral class
      typedef IntegralClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "unsigned short";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(unsigned short)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>signed short</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<signed short>
    {
      /// this type is integral
      static constexpr bool is_int = true;
      /// this type is not floating
      static constexpr bool is_float = false;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is signed
      static constexpr bool is_signed = true;

      /// this type is of integral class
      typedef IntegralClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "signed short";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(signed short)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>unsigned long</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<unsigned long>
    {
      /// this type is integral
      static constexpr bool is_int = true;
      /// this type is not floating
      static constexpr bool is_float = false;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is unsigned
      static constexpr bool is_signed = false;

      /// this type is of integral class
      typedef IntegralClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "unsigned long";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(unsigned long)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>signed long</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<signed long>
    {
      /// this type is integral
      static constexpr bool is_int = true;
      /// this type is not floating
      static constexpr bool is_float = false;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is signed
      static constexpr bool is_signed = true;

      /// this type is of integral class
      typedef IntegralClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "signed long";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(signed long)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>unsigned long long</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<unsigned long long>
    {
      /// this type is integral
      static constexpr bool is_int = true;
      /// this type is not floating
      static constexpr bool is_float = false;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is unsigned
      static constexpr bool is_signed = false;

      /// this type is of integral class
      typedef IntegralClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "unsigned long long";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(unsigned long long)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>signed long long</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<signed long long>
    {
      /// this type is integral
      static constexpr bool is_int = true;
      /// this type is not floating
      static constexpr bool is_float = false;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is signed
      static constexpr bool is_signed = true;

      /// this type is of integral class
      typedef IntegralClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "signed long long";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(signed long long)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

    /**
     * \brief Type Traits specialisation for <c>bool</c>
     *
     * \author Dirk Ribbrock
     * \author Peter Zajac
     */
    template<>
    struct Traits<bool>
    {
      /// this type is not integral
      static constexpr bool is_int = false;
      /// this type is not floating
      static constexpr bool is_float = false;
      /// this type is not boolean
      static constexpr bool is_bool = true;
      /// this type is unsigned
      static constexpr bool is_signed = false;

      /// this type is of boolean class
      typedef BooleanClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "bool";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(bool)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };

#if defined(FEAT_HAVE_QUADMATH) && !defined(__CUDACC__)
    /**
     * \brief Type Traits specialisation for <c>__float128</c>
     *
     * \author Peter Zajac
     */
    template<>
    struct Traits<__float128>
    {
      /// this type is not integral
      static constexpr bool is_int = false;
      /// this type is floating
      static constexpr bool is_float = true;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is signed
      static constexpr bool is_signed = true;

      /// this type is of floating class
      typedef FloatingClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "__float128";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(__float128)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };
#endif // FEAT_HAVE_QUADMATH && !__CUDACC__

#if defined(FEAT_HAVE_HALFMATH) && !defined(__CUDACC__)
    /**
     * \brief Type Traits specialisation for <c>half</c>
     *
     * \author Dirk Ribbrock
     */
    template<>
    struct Traits<half_float::half>
    {
      /// this type is not integral
      static constexpr bool is_int = false;
      /// this type is floating
      static constexpr bool is_float = true;
      /// this type is not boolean
      static constexpr bool is_bool = false;
      /// this type is signed
      static constexpr bool is_signed = true;

      /// this type is of floating class
      typedef FloatingClass TypeClass;

      /// returns a string identifying the datatype
      static String name()
      {
        return "half";
      }

      /// returns composition of datatype size, int-, float- and signed feature
      static uint64_t feature_hash()
      {
        return uint64_t(sizeof(half_float::half)) | uint64_t(is_int) << 32 | uint64_t(is_float) << 33 | uint64_t(is_signed) << 34;
      }
    };
#endif // FEAT_HAVE_HALFMATH $$ !__CUDACC__
  } // namespace Type
} // namespace FEAT

#endif // KERNEL_UTIL_TYPE_TRAITS_HPP
