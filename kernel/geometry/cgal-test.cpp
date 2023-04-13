// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2021 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#include <test_system/test_system.hpp>

#ifdef FEAT_HAVE_CGAL
#include <kernel/geometry/cgal.hpp>

using namespace FEAT;
using namespace FEAT::TestSystem;
using namespace FEAT::Geometry;


class CGALTest
  : public UnitTest
{
public:
  CGALTest() :
    UnitTest("CGALTest")
  {
  }

  virtual ~CGALTest()
  {
  }

#if defined(__clang__)
  __attribute__((no_sanitize("undefined")))
#endif
  virtual void run() const override
  {
    std::stringstream mts;
    mts<<"OFF"<<std::endl;
    mts<<"4 4 6"<<std::endl;
    mts<<"0.0 0.0 2.0"<<std::endl;
    mts<<"1.632993 -0.942809 -0.666667"<<std::endl;
    mts<<"0.000000 1.885618 -0.666667"<<std::endl;
    mts<<"-1.632993 -0.942809 -0.666667"<<std::endl;
    mts<<"3 1 0 3"<<std::endl;
    mts<<"3 2 0 1"<<std::endl;
    mts<<"3 3 0 2"<<std::endl;
    mts<<"3 3 2 1"<<std::endl;

    CGALWrapper cw(mts);
    TEST_CHECK_EQUAL(cw.point_inside(50, 50, 50), false);
    TEST_CHECK_EQUAL(cw.point_inside(0.1, 0.1, 0.1), true);
  }

} cgal_test;
#endif //FEAT_HAVE_CGAL
