//  Copyright (c) 2007-2014 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/parallel_rotate.hpp>
#include <hpx/iterator_support/iterator_range.hpp>
#include <hpx/testing.hpp>

#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

#include "test_utils.hpp"

///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_rotate(ExPolicy policy, IteratorTag)
{
    static_assert(
        hpx::parallel::execution::is_execution_policy<ExPolicy>::value,
        "hpx::parallel::execution::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    typedef test::test_container<std::vector<std::size_t>, IteratorTag>
        test_vector;

    test_vector c(10007);
    std::vector<std::size_t> d1;

    std::iota(std::begin(c.base()), std::end(c.base()), std::rand());
    std::copy(std::begin(c.base()), std::end(c.base()), std::back_inserter(d1));

    std::size_t mid_pos = std::rand() % c.size();    //-V104
    auto mid = std::begin(c);
    std::advance(mid, mid_pos);

    hpx::parallel::rotate(policy, c, iterator(mid));

    base_iterator mid1 = std::begin(d1);
    std::advance(mid1, mid_pos);
    std::rotate(std::begin(d1), mid1, std::end(d1));

    std::size_t count = 0;
    HPX_TEST(std::equal(std::begin(c.base()), std::end(c.base()),
        std::begin(d1), [&count](std::size_t v1, std::size_t v2) -> bool {
            HPX_TEST_EQ(v1, v2);
            ++count;
            return v1 == v2;
        }));
    HPX_TEST_EQ(count, d1.size());
}

template <typename ExPolicy, typename IteratorTag>
void test_rotate_async(ExPolicy p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    typedef test::test_container<std::vector<std::size_t>, IteratorTag>
        test_vector;

    test_vector c(10007);
    std::vector<std::size_t> d1;

    std::iota(std::begin(c.base()), std::end(c.base()), std::rand());
    std::copy(std::begin(c.base()), std::end(c.base()), std::back_inserter(d1));

    std::size_t mid_pos = std::rand() % c.size();    //-V104

    auto mid = std::begin(c);
    std::advance(mid, mid_pos);

    auto f = hpx::parallel::rotate(p, c, iterator(mid));
    f.wait();

    base_iterator mid1 = std::begin(d1);
    std::advance(mid1, mid_pos);
    std::rotate(std::begin(d1), mid1, std::end(d1));

    std::size_t count = 0;
    HPX_TEST(std::equal(std::begin(c.base()), std::end(c.base()),
        std::begin(d1), [&count](std::size_t v1, std::size_t v2) -> bool {
            HPX_TEST_EQ(v1, v2);
            ++count;
            return v1 == v2;
        }));
    HPX_TEST_EQ(count, d1.size());
}

template <typename IteratorTag>
void test_rotate()
{
    using namespace hpx::parallel;
    test_rotate(execution::seq, IteratorTag());
    test_rotate(execution::par, IteratorTag());
    test_rotate(execution::par_unseq, IteratorTag());

    test_rotate_async(execution::seq(execution::task), IteratorTag());
    test_rotate_async(execution::par(execution::task), IteratorTag());
}

void rotate_test()
{
    test_rotate<std::random_access_iterator_tag>();
    test_rotate<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_rotate_exception(ExPolicy policy, IteratorTag)
{
    static_assert(
        hpx::parallel::execution::is_execution_policy<ExPolicy>::value,
        "hpx::parallel::execution::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::iota(std::begin(c), std::end(c), std::rand());

    base_iterator mid = std::begin(c);

    // move at least one element to guarantee an exception to be thrown
    std::size_t delta =
        (std::max)(std::rand() % c.size(), std::size_t(2));    //-V104
    std::advance(mid, delta);

    bool caught_exception = false;
    try
    {
        hpx::parallel::rotate(policy,
            hpx::util::make_iterator_range(
                decorated_iterator(
                    std::begin(c), []() { throw std::runtime_error("test"); }),
                decorated_iterator(std::end(c))),
            decorated_iterator(mid));
        HPX_TEST(false);
    }
    catch (hpx::exception_list const& e)
    {
        caught_exception = true;
        test::test_num_exceptions<ExPolicy, IteratorTag>::call(policy, e);
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
}

template <typename ExPolicy, typename IteratorTag>
void test_rotate_exception_async(ExPolicy p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::iota(std::begin(c), std::end(c), std::rand());

    base_iterator mid = std::begin(c);

    // move at least one element to guarantee an exception to be thrown
    std::size_t delta =
        (std::max)(std::rand() % c.size(), std::size_t(2));    //-V104
    std::advance(mid, delta);

    bool caught_exception = false;
    bool returned_from_algorithm = false;
    try
    {
        auto f = hpx::parallel::rotate(p,
            hpx::util::make_iterator_range(
                decorated_iterator(
                    std::begin(c), []() { throw std::runtime_error("test"); }),
                decorated_iterator(std::end(c))),
            decorated_iterator(mid));
        returned_from_algorithm = true;
        f.get();

        HPX_TEST(false);
    }
    catch (hpx::exception_list const& e)
    {
        caught_exception = true;
        test::test_num_exceptions<ExPolicy, IteratorTag>::call(p, e);
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
    HPX_TEST(returned_from_algorithm);
}

template <typename IteratorTag>
void test_rotate_exception()
{
    using namespace hpx::parallel;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_rotate_exception(execution::seq, IteratorTag());
    test_rotate_exception(execution::par, IteratorTag());

    test_rotate_exception_async(execution::seq(execution::task), IteratorTag());
    test_rotate_exception_async(execution::par(execution::task), IteratorTag());
}

void rotate_exception_test()
{
    test_rotate_exception<std::random_access_iterator_tag>();
    test_rotate_exception<std::forward_iterator_tag>();
}

//////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_rotate_bad_alloc(ExPolicy policy, IteratorTag)
{
    static_assert(
        hpx::parallel::execution::is_execution_policy<ExPolicy>::value,
        "hpx::parallel::execution::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::iota(std::begin(c), std::end(c), std::rand());

    base_iterator mid = std::begin(c);

    // move at least one element to guarantee an exception to be thrown
    std::size_t delta =
        (std::max)(std::rand() % c.size(), std::size_t(2));    //-V104
    std::advance(mid, delta);

    bool caught_bad_alloc = false;
    try
    {
        hpx::parallel::rotate(policy,
            hpx::util::make_iterator_range(
                decorated_iterator(
                    std::begin(c), []() { throw std::bad_alloc(); }),
                decorated_iterator(std::end(c))),
            decorated_iterator(mid));
        HPX_TEST(false);
    }
    catch (std::bad_alloc const&)
    {
        caught_bad_alloc = true;
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_bad_alloc);
}

template <typename ExPolicy, typename IteratorTag>
void test_rotate_bad_alloc_async(ExPolicy p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::iota(std::begin(c), std::end(c), std::rand());

    base_iterator mid = std::begin(c);

    // move at least one element to guarantee an exception to be thrown
    std::size_t delta =
        (std::max)(std::rand() % c.size(), std::size_t(2));    //-V104
    std::advance(mid, delta);

    bool caught_bad_alloc = false;
    bool returned_from_algorithm = false;
    try
    {
        auto f = hpx::parallel::rotate(p,
            hpx::util::make_iterator_range(
                decorated_iterator(
                    std::begin(c), []() { throw std::bad_alloc(); }),
                decorated_iterator(std::end(c))),
            decorated_iterator(mid));
        returned_from_algorithm = true;
        f.get();

        HPX_TEST(false);
    }
    catch (std::bad_alloc const&)
    {
        caught_bad_alloc = true;
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_bad_alloc);
    HPX_TEST(returned_from_algorithm);
}

template <typename IteratorTag>
void test_rotate_bad_alloc()
{
    using namespace hpx::parallel;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_rotate_bad_alloc(execution::seq, IteratorTag());
    test_rotate_bad_alloc(execution::par, IteratorTag());

    test_rotate_bad_alloc_async(execution::seq(execution::task), IteratorTag());
    test_rotate_bad_alloc_async(execution::par(execution::task), IteratorTag());
}

void rotate_bad_alloc_test()
{
    test_rotate_bad_alloc<std::random_access_iterator_tag>();
    test_rotate_bad_alloc<std::forward_iterator_tag>();
}

int hpx_main(hpx::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int) std::time(nullptr);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    std::srand(seed);

    rotate_test();
    rotate_exception_test();
    rotate_bad_alloc_test();
    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // add command line option which controls the random number generator seed
    using namespace hpx::program_options;
    options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()("seed,s", value<unsigned int>(),
        "the random number generator seed to use for this run");

    // By default this test should run on all available cores
    std::vector<std::string> const cfg = {"hpx.os_threads=all"};

    // Initialize and run HPX
    HPX_TEST_EQ_MSG(hpx::init(desc_commandline, argc, argv, cfg), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}
