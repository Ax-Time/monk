#ifndef MONK_TEST_HPP
#define MONK_TEST_HPP

#include <functional>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

namespace monk::test
{
    namespace impl
    {
        class TestException : public std::exception
        {
        public:
            TestException() = default;
            TestException(std::string message)
                : _message(message) {}
            TestException(TestException const &) = default;
            TestException(TestException &&) = default;
            TestException &operator=(TestException const &) = default;
            TestException &operator=(TestException &&) = default;
            ~TestException() = default;

            const char *what() const noexcept override
            {
                return _message.c_str();
            }

        private:
            std::string _message;
        };

        class Test
        {
        public:
            Test(std::string name, std::function<void()> test)
                : _name(name), _test(test) {}
            Test(Test const &) = default;
            Test(Test &&) = default;
            Test &operator=(Test const &) = default;
            Test &operator=(Test &&) = default;
            ~Test() = default;

            bool run() const
            {
                try
                {
                    _test();
                    std::cout << "Test " << _name << " passed." << std::endl;
                    return true;
                }
                catch (TestException const &e)
                {
                    std::cout << "Test " << _name << " failed: " << e.what() << std::endl;
                    return false;
                }
            }

        private:
            std::string _name;
            std::function<void()> _test;
        };

        class TestManager
        {
        public:
            static TestManager &instance()
            {
                static TestManager instance;
                return instance;
            }

            void addTest(Test test)
            {
                _tests.push_back(test);
            }

            bool runTests()
            {
                for (auto &test : _tests)
                {
                    if (!test.run())
                    {
                        return false;
                    }
                }
                return true;
            }

        private:
            TestManager() = default;
            ~TestManager() = default;
            std::vector<Test> _tests;
        };
    };

    void add(std::string name, std::function<void()> test)
    {
        impl::TestManager::instance().addTest(impl::Test(name, test));
    }

    class Tests
    {
    public:
        Tests(std::string name) : _name(name) {}
        Tests &add(std::string name, std::function<void()> test)
        {
            monk::test::add(_name + "::" + name, test);
            return *this;
        }

    private:
        std::string _name;
    };

    void runAll()
    {
        impl::TestManager::instance().runTests();
    }

    namespace impl
    {
        template <bool verbose, typename Fun, typename... Args>
        void assertCondition(std::string cond_name, Fun test, Args... args)
        {
            if (!test(args...))
            {
                std::ostringstream oss;
                oss << "Condition " << cond_name << " not met. ";
                if constexpr (verbose)
                {
                    oss << "Values were (";
                    ((oss << args << ", "), ...);
                    oss << "\b\b).";
                }
                throw TestException(oss.str());
            }
        }
    }

    template <typename T, bool verbose = true>
    void assertEqual(T const &actual, T const &expected)
    {
        impl::assertCondition<true>(
            "assertEqual",
            [](T const &a, T const &b) constexpr -> bool
            { return a == b; },
            actual,
            expected);
    }

    template <typename T>
    void assertNotEqual(T const &actual, T const &expected, bool verbose = true)
    {
        impl::assertCondition<true>(
            "assertNotEqual",
            [](T const &a, T const &b) constexpr -> bool
            { return a != b; },
            actual,
            expected);
    }

    void assertTrue(bool value, bool verbose = true)
    {
        impl::assertCondition<true>(
            "assertTrue",
            [](bool cond) constexpr -> bool
            { return cond; },
            value);
    }

    void assertFalse(bool value, bool verbose = true)
    {
        impl::assertCondition<true>(
            "assertFalse",
            [](bool cond) constexpr -> bool
            { return !cond; },
            value);
    }

    template <typename T>
    void assertLt(T const &lhs, T const &rhs, bool verbose = true)
    {
        impl::assertCondition<true>(
            "asserLt",
            [](T const &a, T const &b) constexpr -> bool
            { return a < b; },
            lhs,
            rhs);
    }

    template <typename T>
    void assertLte(T const &lhs, T const &rhs, bool verbose = true)
    {
        impl::assertCondition<true>(
            "asserLte",
            [](T const &a, T const &b) constexpr -> bool
            { return a <= b; },
            lhs,
            rhs);
    }

    template <typename T>
    void assertGt(T const &lhs, T const &rhs, bool verbose = true)
    {
        impl::assertCondition<true>(
            "asserGt",
            [](T const &a, T const &b) constexpr -> bool
            { return a > b; },
            lhs,
            rhs);
    }

    template <typename T>
    void assertGte(T const &lhs, T const &rhs, bool verbose = true)
    {
        impl::assertCondition<true>(
            "asserGte",
            [](T const &a, T const &b) constexpr -> bool
            { return a >= b; },
            lhs,
            rhs);
    }

    namespace impl
    {
        template <typename Exception, typename... Rest>
        bool validException(std::exception_ptr eptr)
        {
            try
            {
                if (eptr)
                {
                    std::rethrow_exception(eptr);
                }
            }
            catch (Exception const &)
            {
                return true;
            }
            catch (...)
            {
                if constexpr (sizeof...(Rest) > 0)
                {
                    return validException<Rest...>(eptr);
                }
            }
            return false;
        }
    }

    template <typename... Exceptions>
    void assertThrows(std::function<void()> test)
    {
        auto lmbda = [](std::function<void()> _test) -> bool
        {
            try
            {
                _test();
            }
            catch (...)
            {
                std::exception_ptr eptr = std::current_exception();
                return impl::validException<Exceptions...>(eptr);
            }
            return false;
        };
        impl::assertCondition<false>("assertThrows", lmbda, test);
    }
};

#endif // SUGAR_TEST_HPP