/**
 * @file test_compiler.cc
 * @brief Compiler subsystem unit tests.
 *
 * Copyright (C) 2012 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 **/

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <stdint.h>

#include <expr.hh>
#include <expr_mgr.hh>
#include <printer.hh>

#include <model.hh>
#include <model_mgr.hh>
#include <model/module.hh>
#include <model/compiler/compiler.hh>

using LList = std::initializer_list<std::initializer_list<int>>;
class DDChecker {
  std::list<std::list<int>> f_expected;
  std::list<std::list<int>> f_actual;

public:
  DDChecker(LList list_of_ints)
  {
    std::list<int> tmp;
    for (auto literals : list_of_ints) {
      tmp.clear();
      for (auto value : literals) {
        tmp.push_back(value);
      }
      f_expected.push_back(tmp);
    }
  }

  void add(std::list<int> lst) {
    f_actual.push_back(lst);
  }

  bool check() const
  {
    bool res { f_expected == f_actual };

    if (! res) {
      std::cerr
        << "Expected"
        << std::endl ;

      std::for_each(begin(f_expected), end(f_expected),
                    [](std::list<int> lst) {
                      std::for_each(begin(lst), end(lst),
                                    [](int elem) {
                                      std::cerr
                                        << elem
                                        << " ";
                                    });
                      std::cerr
                        << std::endl;
                    });

      std::cerr
        << "Actual"
        << std::endl;

      std::for_each(begin(f_actual), end(f_actual),
                    [](std::list<int> lst) {
                      std::for_each(begin(lst), end(lst),
                                    [](int elem) {
                                      std::cerr
                                        << elem
                                        << " ";
                                    });
                      std::cerr
                        << std::endl;
                    });
    }

    return res;
  }
};

static void callback(void *obj, int* list, int size)
{
  assert (NULL != obj);
  DDChecker* pchecker = (DDChecker *) obj;

  std::list<int> tmp;
  for (int i = 0; i < size; ++ i) {
    int c { *(list+i) };
    tmp.push_back(c);
  }

  pchecker->add(tmp);
}

BOOST_AUTO_TEST_SUITE(tests)
BOOST_AUTO_TEST_CASE(compiler_boolean)
{
    ModelMgr& mm
        (ModelMgr::INSTANCE());

    ExprMgr& em
        (ExprMgr::INSTANCE());

    TypeMgr& tm
        (TypeMgr::INSTANCE());

    Compiler f_compiler;

    Model& model
        (mm.model());

    Atom a_main("main");
    Expr_ptr main_expr(em.make_identifier(a_main));

    Module_ptr main_module = new Module(main_expr);
    model.add_module(*main_module);

    Type_ptr u2 = tm.find_unsigned(2);

    Atom a_x("x"); Expr_ptr x = em.make_identifier(a_x);
    main_module->add_var(x, new Variable(main_expr, x, u2));

    Atom a_y("y"); Expr_ptr y = em.make_identifier(a_y);
    main_module->add_var(y, new Variable(main_expr, y, u2));

    // mm.analyze();

    const int F = 0;
    const int T = 1;
    const int X = 2; /* DON'T CARE */

    /* NOT x */
    {
      Expr_ptr test_expr
        (em.make_not(x));

      DDChecker checker {
        {F}
      } ;

      CompilationUnit cu
        (f_compiler.process(em.make_empty(),
                            test_expr));

      DDVector dv
        (cu.dds());

      BOOST_CHECK(dv.size() == 1);
      ADD dd
        (dv.at(0));

      dd.Callback(callback, &checker, 1);
      BOOST_CHECK(checker.check());
    }

    /* NOT NOT x */
    {
      Expr_ptr test_expr
        (em.make_not(em.make_not(x)));

      DDChecker checker {
        {T}
      } ;

      CompilationUnit cu
        (f_compiler.process(em.make_empty(),
                            test_expr));

      DDVector dv
        (cu.dds());

      BOOST_CHECK(dv.size() == 1);
      ADD dd
        (dv.at(0));

      dd.Callback(callback, &checker, 1);
      BOOST_CHECK(checker.check());
    }

    /* x NE y */
    {
        Expr_ptr test_expr
          (em.make_ne(x, y));

        DDChecker checker {
          {F, T}, {T, F}
        };

        CompilationUnit cu
            (f_compiler.process(em.make_empty(),
                                test_expr));

        DDVector dv
            (cu.dds());

        BOOST_CHECK(dv.size() == 1);
        ADD dd
            (dv.at(0));

        dd.Callback(callback, &checker, 1);
        BOOST_CHECK(checker.check());
    }

    /* x EQ y */
    {
      Expr_ptr test_expr
        (em.make_eq(x, y));

      DDChecker checker {
        {F, F}, {T, T}
      };

      CompilationUnit cu
        (f_compiler.process(em.make_empty(),
                            test_expr));

      DDVector dv
        (cu.dds());

      BOOST_CHECK(dv.size() == 1);
      ADD dd
        (dv.at(0));

      dd.Callback(callback, &checker, 1);
      // dd.PrintMinterm();
      BOOST_CHECK(checker.check());
    }

    /* (NOT x NE y) <-> x EQ y */
    {
      Expr_ptr test_expr
        (em.make_eq(em.make_not(em.make_ne(x, y)),
                    em.make_eq(x, y)));

      DDChecker checker {
        {X, X}
      };

      CompilationUnit cu
        (f_compiler.process(em.make_empty(),
                            test_expr));

      DDVector dv
        (cu.dds());

      BOOST_CHECK(dv.size() == 1);
      ADD dd
        (dv.at(0));

      dd.Callback(callback, &checker, 1);
      // dd.PrintMinterm();
      BOOST_CHECK(checker.check());
    }

    /* x AND y */
    {
      Expr_ptr test_expr
        (em.make_and(x, y));

      DDChecker checker {
        {T, T}
      };

      CompilationUnit cu
        (f_compiler.process(em.make_empty(),
                            test_expr));

      DDVector dv
        (cu.dds());

      BOOST_CHECK(dv.size() == 1);
      ADD dd
        (dv.at(0));

      dd.Callback(callback, &checker, 1);
      BOOST_CHECK(checker.check());
    }

    /* x OR y */
    {
      Expr_ptr test_expr
        (em.make_or(x, y));

      DDChecker checker {
        {F, T}, {T, X}
      };

      CompilationUnit cu
        (f_compiler.process(em.make_empty(),
                            test_expr));

      DDVector dv
        (cu.dds());

      BOOST_CHECK(dv.size() == 1);
      ADD dd
        (dv.at(0));

      dd.Callback(callback, &checker, 1);
      BOOST_CHECK(checker.check());
    }

    /* (NOT (X AND Y)) == (NOT X) OR (NOT Y) */
    {
      Expr_ptr test_expr
        (em.make_eq(em.make_not(em.make_and(x, y)),
                    em.make_or(em.make_not(x), em.make_not(y))));

      DDChecker checker {
        {X, X}
      } ;

      CompilationUnit cu
        (f_compiler.process(em.make_empty(),
                            test_expr));

      DDVector dv
        (cu.dds());

      BOOST_CHECK(dv.size() == 1);
      ADD dd
        (dv.at(0));

      dd.Callback(callback, &checker, 1);
      BOOST_CHECK(checker.check());
    }

    /* (NOT (X OR Y)) == (NOT X) AND (NOT Y) */
    {
      Expr_ptr test_expr
        (em.make_eq(em.make_not(em.make_or(x, y)),
                    em.make_and(em.make_not(x), em.make_not(y))));

      DDChecker checker {
        {X, X}
      } ;

      CompilationUnit cu
        (f_compiler.process(em.make_empty(),
                            test_expr));

      DDVector dv
        (cu.dds());

      BOOST_CHECK(dv.size() == 1);
      ADD dd
        (dv.at(0));

      dd.Callback(callback, &checker, 1);
      BOOST_CHECK(checker.check());
    }

    /* x IMPL y */
    {
      Expr_ptr test_expr
        (em.make_implies(x, y));

      DDChecker checker {
        {F, X}, {T, T}
      };

      CompilationUnit cu
        (f_compiler.process(em.make_empty(),
                            test_expr));

      DDVector dv
        (cu.dds());

      BOOST_CHECK(dv.size() == 1);
      ADD dd
        (dv.at(0));

      dd.Callback(callback, &checker, 1);
      BOOST_CHECK(checker.check());
    }

    /* (x IMPL y) AND (y IMPL X) == (x == y) */
    {
      Expr_ptr test_expr
        (em.make_eq(em.make_and(em.make_implies(x, y),
                                em.make_implies(y, x)),
                    em.make_eq(x, y)));

      DDChecker checker {
        {X, X}
      } ;

      CompilationUnit cu
        (f_compiler.process(em.make_empty(),
                            test_expr));

      DDVector dv
        (cu.dds());

      BOOST_CHECK(dv.size() == 1);
      ADD dd
        (dv.at(0));

      dd.Callback(callback, &checker, 1);
      BOOST_CHECK(checker.check());
    }
}

BOOST_AUTO_TEST_SUITE_END()
