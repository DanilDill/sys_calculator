#include <gtest/gtest.h>
#include "libmath.h"

// Тесты для функции сложения
TEST(MathAddTest, PositiveNumbers)
{
    int err = 0;
    EXPECT_EQ(math::add(2, 3, err), 5);
    EXPECT_EQ(err, 0);
}

TEST(MathAddTest, NegativeNumbers)
{
    int err = 0;
    EXPECT_EQ(math::add(-2, -3, err), -5);
    EXPECT_EQ(err, 0);
}

TEST(MathAddTest, MixedSigns)
{
    int err = 0;
    EXPECT_EQ(math::add(-2, 3, err), 1);
    EXPECT_EQ(err, 0);
}

TEST(MathAddTest, WithZero)
{
    int err = 0;
    EXPECT_EQ(math::add(0, 5, err), 5);
    EXPECT_EQ(err, 0);
    EXPECT_EQ(math::add(5, 0, err), 5);
    EXPECT_EQ(err, 0);
}

TEST(MathAddTest, OverflowDetection)
{
    int err = 0;
    math::add(INT_MAX, 1, err);
    EXPECT_EQ(err, OVERFLOW);
}

// Тесты для функции вычитания
TEST(MathSubTest, PositiveResult)
{
    int err = 0;
    EXPECT_EQ(math::sub(5, 3, err), 2);
    EXPECT_EQ(err, 0);
}

TEST(MathSubTest, NegativeResult)
{
    int err = 0;
    EXPECT_EQ(math::sub(3, 5, err), -2);
    EXPECT_EQ(err, 0);
}

TEST(MathSubTest, WithZero)
{
    int err = 0;
    EXPECT_EQ(math::sub(5, 0, err), 5);
    EXPECT_EQ(err, 0);
    EXPECT_EQ(math::sub(0, 5, err), -5);
    EXPECT_EQ(err, 0);
}

TEST(MathSubTest, SameNumbers)
{
    int err = 0;
    EXPECT_EQ(math::sub(5, 5, err), 0);
    EXPECT_EQ(err, 0);
}

TEST(MathSubTest, OverflowDetection)
{
    int err = 0;
    math::sub(INT_MIN, 1, err);
    EXPECT_EQ(err, OVERFLOW);
}

// Тесты для функции умножения
TEST(MathMulTest, PositiveNumbers)
{
    int err = 0;
    EXPECT_EQ(math::mul(3, 4, err), 12);
    EXPECT_EQ(err, 0);
}

TEST(MathMulTest, NegativeNumbers)
{
    int err = 0;
    EXPECT_EQ(math::mul(-3, -4, err), 12);
    EXPECT_EQ(err, 0);
}

TEST(MathMulTest, MixedSigns)
{
    int err = 0;
    EXPECT_EQ(math::mul(-3, 4, err), -12);
    EXPECT_EQ(err, 0);
}

TEST(MathMulTest, WithZero)
{
    int err = 0;
    EXPECT_EQ(math::mul(0, 5, err), 0);
    EXPECT_EQ(err, 0);
    EXPECT_EQ(math::mul(5, 0, err), 0);
    EXPECT_EQ(err, 0);
}

TEST(MathMulTest, WithOne)
{
    int err = 0;
    EXPECT_EQ(math::mul(5, 1, err), 5);
    EXPECT_EQ(err, 0);
}

TEST(MathMulTest, OverflowDetection)
{
    int err = 0;
    math::mul(INT_MAX, 2, err);
    EXPECT_EQ(err, OVERFLOW);
}

// Тесты для функции деления
TEST(MathDivTest, ExactDivision)
{
    int err = 0;
    EXPECT_EQ(math::div(10, 2, err), 5);
    EXPECT_EQ(err, 0);
}

TEST(MathDivTest, DivisionWithRemainder)
{
    int err = 0;
    EXPECT_EQ(math::div(10, 3, err), 3);
    EXPECT_EQ(err, 0);
}

TEST(MathDivTest, NegativeNumbers)
{
    int err = 0;
    EXPECT_EQ(math::div(-10, 2, err), -5);
    EXPECT_EQ(err, 0);
    EXPECT_EQ(math::div(10, -2, err), -5);
    EXPECT_EQ(err, 0);
}

TEST(MathDivTest, DivisionByZero)
{
    int err = 0;
    EXPECT_EQ(math::div(10, 0, err), 0);
    EXPECT_EQ(err, DIV_BY_ZERO);
}

TEST(MathDivTest, ZeroDividedByNumber)
{
    int err = 0;
    EXPECT_EQ(math::div(0, 5, err), 0);
    EXPECT_EQ(err, 0);
}

// Тесты для функции возведения в степень
TEST(MathPowTest, PositiveExponent)
{
    int err = 0;
    EXPECT_EQ(math::pow(2, 3, err), 8);
    EXPECT_EQ(err, 0);
}

TEST(MathPowTest, ZeroExponent)
{
    int err = 0;
    EXPECT_EQ(math::pow(5, 0, err), 1);
    EXPECT_EQ(err, 0);
}

TEST(MathPowTest, FirstPower)
{
    int err = 0;
    EXPECT_EQ(math::pow(5, 1, err), 5);
    EXPECT_EQ(err, 0);
}

TEST(MathPowTest, BaseZero)
{
    int err = 0;
    EXPECT_EQ(math::pow(0, 5, err), 0);
    EXPECT_EQ(err, 0);
}

TEST(MathPowTest, OverflowDetection)
{
    int err = 0;
    math::pow(2, 31, err);
    EXPECT_EQ(err, OVERFLOW);
}

// Тесты для функции факториала
TEST(MathFactorialTest, ZeroFactorial)
{
    int err = 0;
    EXPECT_EQ(math::factorial(0, err), 1);
    EXPECT_EQ(err, 0);
}

TEST(MathFactorialTest, OneFactorial)
{
    int err = 0;
    EXPECT_EQ(math::factorial(1, err), 1);
    EXPECT_EQ(err, 0);
}

TEST(MathFactorialTest, SmallNumbers)
{
    int err = 0;
    EXPECT_EQ(math::factorial(5, err), 120);
    EXPECT_EQ(err, 0);
    EXPECT_EQ(math::factorial(6, err), 720);
    EXPECT_EQ(err, 0);
}

TEST(MathFactorialTest, MaxValidFactorial)
{
    int err = 0;
    EXPECT_EQ(math::factorial(12, err), 479001600);
    EXPECT_EQ(err, 0);
}

TEST(MathFactorialTest, OverflowDetection)
{
    int err = 0;
    math::factorial(13, err);
    EXPECT_EQ(err, OVERFLOW);
}
