#pragma once
#ifndef TEST_SYSTEM_TEST_SYSTEM_HPP
/// Header guard
#define TEST_SYSTEM_TEST_SYSTEM_HPP 1

// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/util/type_traits.hpp>
#include <kernel/util/instantiation_policy.hpp>
#include <kernel/util/exception.hpp>
#include <kernel/archs.hpp>

// includes, system
#include <string>
#include <exception>
#include <list>
#include <typeinfo>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <cmath>

#define CHECK_INTERNAL(was_ok, message)\
        if(! (was_ok))\
          throw TestFailedException(__func__, __FILE__, __LINE__, message);

/**
* \file
*
* Implementation of Test and related classes.
*/

namespace FEAST
{
  /// TestSystem namespace
  namespace TestSystem
  {
    // Forwared declaration
    class BaseTest;

    /// exception thrown by the check method in BaseTest
    class TestFailedException
      : public std::exception
    {

    private:

      /// failure message
      const String _message;


    public:

      /**
        * \brief CTOR
        *
        * \param[in] function
        * The function that trows the exception.
        *
        * \param[in] file
        * The file the exception was thrown in.
        *
        * \param[in] line
        * The line the exception was thrown in.
        *
        * \param[in] message
        * A message describing the exception.
        */
      TestFailedException(
          const char* const function,
          const char* const file,
          const long line,
          const String & message) throw ()
        : _message(stringify(file) + ":" + stringify(line) + ": in " + stringify(function) + ": " + message )
      {
      }

      /// DTOR
      virtual ~TestFailedException() throw ()
      {
      }

      /// description
      const char* what() const throw ()
      {
        return _message.c_str();
      }
    }; // class TestFailedException

    /// list of all instantiated tests
    class TestList
      : public InstantiationPolicy<TestList, Singleton>
    {

    private:

      /// internal STL list representation of TestList
      std::list<BaseTest*> _tests;

      /// default CTOR
      TestList()
      {
      }


    public:

      /// pointer to TestList singleton
      friend TestList* InstantiationPolicy<TestList, Singleton>::instance();

      /// TestList forward iterator.
      typedef std::list<BaseTest*>::iterator Iterator;

      /**
        * \brief adds a test to the TestList
        *
        * \param[in] test
        * the test that will be added
        */
      void register_test(BaseTest* const test)
      {
        _tests.push_back(test);
      }

      /// returns an iterator to the front of the TestList
      Iterator begin_tests()
      {
        return _tests.begin();
      }

      /// returns an iterator beyond the last element of the TestList
      Iterator end_tests()
      {
        return _tests.end();
      }

      /// returns the size of the TestList
      size_t size()
      {
        return _tests.size();
      }

      /// removes iterator target from the TestList
      Iterator erase (Iterator i)
      {
        return _tests.erase(i);
      }
    }; // class TestList


    /**
     * \brief base class for all Tests
     *
     * \author Dirk Ribbrock
     */
    class BaseTest
    {
    protected:

      /// test description String
      const String _id;
      /// memory description String
      String _mem_name;
      /// precision description String
      String _prec_name;
      /// index type description String
      String _index_name;


    public:

      /**
        * \brief CTOR
        *
        * \param[in] id
        * the testcase's id string
        */
      BaseTest(const String& id_in)
        : _id(id_in),
        _mem_name(Type::Traits<Archs::None>::name()),
        _prec_name(Type::Traits<Archs::None>::name()),
        _index_name(Type::Traits<Archs::None>::name())
      {
        TestList::instance()->register_test(this);
      }

      /// DTOR
      virtual ~BaseTest() {}

      /// returns our id string
      virtual const String id() const
      {
        return _id;
      }

      /// utility method used bei TEST_CHECK_THROWS
      virtual void check(const char * const function, const char * const file,
          const long line, bool was_ok, const String & message) const
      {
        if(! was_ok)
          throw TestFailedException(function, file, line, message);
      }

      /**
        * \brief runs the test case
        *
        * Called by unittest framework only.
        */
      virtual void run() const = 0;

      /// returns our target platform
      virtual String get_memory_name()
      {
        return _mem_name;
      }

      /// returns our target platform
      virtual String get_prec_name()
      {
        return _prec_name;
      }

      /// returns our target platform
      virtual String get_index_name()
      {
        return _index_name;
      }
    }; // class BaseTest

    struct NotSet
    {
      static String name()
      {
        return "not-set";
      }
    };

    /**
     * \brief abstract base class for all memory tagged test classes
     *
     * \author Dirk Ribbrock
     */
    template<
      typename Mem_,
      typename DataType_>
    class TaggedTest
      : public BaseTest
    {
    public:
      /**
      * \brief CTOR
      *
      * \param[in] id
      * the testcase's id string
      */
      TaggedTest(const String & id_in)
        : BaseTest(id_in)
      {
        _mem_name = Type::Traits<Mem_>::name();
        _prec_name = Type::Traits<DataType_>::name();
        _index_name = NotSet::name();
      }

      /// DTOR
      virtual ~TaggedTest()
      {
      }
    }; // class TaggedTest

    /**
     * \brief abstract base class for all full tagged test classes
     *
     * \author Dirk Ribbrock
     */
    template<
      typename Mem_,
      typename DT_,
      typename IT_>
    class FullTaggedTest
      : public BaseTest
    {
    public:
      /**
      * \brief CTOR
      *
      * \param[in] id
      * the testcase's id string
      */
      FullTaggedTest(const String & id_in)
        : BaseTest(id_in)
      {
        _mem_name = Type::Traits<Mem_>::name();
        _prec_name = Type::Traits<DT_>::name();
        _index_name = Type::Traits<IT_>::name();
      }

      /// DTOR
      virtual ~FullTaggedTest()
      {
      }
    }; // class FullTaggedTest
  } // namespace TestSystem
} // namespace FEAST
/// checks if a == b
#define TEST_CHECK_EQUAL(a, b) \
  do { \
    CHECK_INTERNAL((a)==(b), this->_id + "\n" +  "Expected '" #a "' to equal \n'" + FEAST::stringify(b) + "'\nbut got\n'" + FEAST::stringify(a) + "'")\
  } while (false)

/// checks if a != b
#define TEST_CHECK_NOT_EQUAL(a, b) \
  do { \
    CHECK_INTERNAL(!((a)==(b)), this->_id + "\n" +  "Expected '" #a "' that is'" + FEAST::stringify(a) + "' to equal not '" + FEAST::stringify(b) + "'")\
  } while (false)

/// checks if stringify(a) == stringify(b)
#define TEST_CHECK_STRINGIFY_EQUAL(a, b) \
  do { \
    String s_a(FEAST::stringify(a)); \
    String s_b(FEAST::stringify(b)); \
    CHECK_INTERNAL(s_a == s_b, this->_id + "\n" +  "Expected '" #a "' to equal '" + s_b + "'\nbut got\n'" + s_a + "'")\
  } while (false)

/// checks if a is true
#define TEST_CHECK(a) \
  do { \
    CHECK_INTERNAL(a, this->_id + "\n" +  "Check '" #a "' failed") \
  } while (false)

/// checks if \c a is true; prints \c msg if \c a is false
#define TEST_CHECK_MSG(a,msg) \
  do { \
    CHECK_INTERNAL(a, this->_id + "\n" + (msg))\
  } while (false)

/// checks if a throws an exception of type b
#define TEST_CHECK_THROWS(a, b) \
  do { \
    try { \
      try { \
        a; \
        this->check(__func__, __FILE__, __LINE__, false, \
            this->_id + "\n" +  "Expected exception of type '" #b "' not thrown"); \
      } catch (b &) { \
        TEST_CHECK(true); \
      } \
    } catch (const TestFailedException &) { \
      throw; \
    } catch (const std::exception & test_e) { \
      throw TestFailedException(__func__, __FILE__, __LINE__, \
          "Test threw unexpected exception "+ FEAST::stringify(test_e.what()) + \
          " inside a TEST_CHECK_THROWS block"); \
    } catch (...) { \
      throw TestFailedException(__func__, __FILE__, __LINE__, \
          "Test threw unexpected unknown exception inside a TEST_CHECK_THROWS block"); \
    } \
  } while (false)

/// checks if |a - b| <= epsilon
#define TEST_CHECK_EQUAL_WITHIN_EPS(a, b, eps) \
  do { \
    CHECK_INTERNAL(((a) <= ((b) + (eps))) && ((b) <= ((a) + (eps))), \
        this->_id + "\n" + "Expected '|" #a " - " #b \
        "|' <= '" + FEAST::stringify(eps) + "' but was '" + \
        FEAST::stringify((a) < (b) ? (b) - (a) : (a) - (b)) + "'" \
        + ", with " #a "=" + FEAST::stringify(a) + " and " #b "=" + FEAST::stringify(b))\
  } while (false)

/// runs the given test with pre- and postprocessing
#define TEST(pre, test, post) \
  do { \
    pre; \
    try { \
      test; \
    } catch (const TestFailedException & test_e) { \
      post; \
      throw; } \
    post; \
  } while (false)

#endif // TEST_SYSTEM_TEST_SYSTEM_HPP
