#include "gtest/gtest.h"

class SomeTestFixture : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};

void SomeTestFixture::SetUp()
{
}

void SomeTestFixture::TearDown()
{
}

TEST_F(SomeTestFixture, TestSomething)
{

}