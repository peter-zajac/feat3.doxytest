#include <kernel/base_header.hpp>
#include <test_system/test_system.hpp>
#include <string>

using namespace TestSystem;
using namespace FEAST;

/**
* \brief Test class for the base_header.
*
* \tparam Tag_
* description missing
*
* \tparam DT_
* description missing
*
* \test
* test description missing
*
* \author Dirk Ribbrock
*/
template<
  typename Tag_,
  typename DT_>
class BaseHeaderTest
  : public TaggedTest<Tag_, DT_>
{
  public:
    /// Constructor
    BaseHeaderTest(const std::string & id) :
      TaggedTest<Tag_, DT_>(id)
    {
    }

    virtual void run() const
    {
      //TEST_CHECK_EQUAL(nullptr, NULL);

#ifndef FEAST_COMPILER
#error "Error: Feast cannot detect the used compiler!"
#endif
    }
};
BaseHeaderTest<Nil, Nil> base_header_test ("BaseHeaderTest");
