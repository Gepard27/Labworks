#include "test_framework.h"
#include <lib/ArgParser.h>
#include <sstream>
#include <vector>
#include <iterator>

using namespace ArgumentParser;

std::vector<std::string> SplitString(const std::string& str) {
    std::istringstream iss(str);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                  std::istream_iterator<std::string>{}};
    return tokens;
}

TEST(EmptyTest) {
    ArgParser parser("My Empty Parser");
    ASSERT_TRUE(parser.Parse(SplitString("app")));
}

TEST(StringTest) {
    ArgParser parser("My Parser");
    parser.AddStringArgument("param1");

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=value1")));
    ASSERT_EQ(parser.GetStringValue("param1"), std::string("value1"));
}

TEST(ShortNameTest) {
    ArgParser parser("My Parser");
    parser.AddStringArgument('p', "param1");

    ASSERT_TRUE(parser.Parse(SplitString("app -p value1")));
    ASSERT_EQ(parser.GetStringValue("param1"), std::string("value1"));
}

TEST(DefaultTest) {
    ArgParser parser("My Parser");
    parser.AddStringArgument("param1").Default("value1");

    ASSERT_TRUE(parser.Parse(SplitString("app")));
    ASSERT_EQ(parser.GetStringValue("param1"), std::string("value1"));
}

TEST(NoDefaultTest) {
    ArgParser parser("My Parser");
    parser.AddStringArgument("param1");

    ASSERT_TRUE(!parser.Parse(SplitString("app")));
}

TEST(StoreValueTest) {
    ArgParser parser("My Parser");
    std::string value;
    parser.AddStringArgument("param1").StoreValue(value);

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=value1")));
    ASSERT_EQ(value, std::string("value1"));
}

TEST(MultiStringTest) {
    ArgParser parser("My Parser");
    std::string value;
    parser.AddStringArgument("param1").StoreValue(value);
    parser.AddStringArgument('a', "param2");

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=value1 --param2=value2")));
    ASSERT_EQ(parser.GetStringValue("param2"), std::string("value2"));
}

TEST(IntTest) {
    ArgParser parser("My Parser");
    parser.AddIntArgument("param1");

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=100500")));
    ASSERT_EQ(parser.GetIntValue("param1"), 100500);
}

TEST(MultiValueTest) {
    ArgParser parser("My Parser");
    std::vector<int> int_values;
    parser.AddIntArgument('p', "param1").MultiValue().StoreValues(int_values);

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=1 --param1=2 --param1=3")));
    ASSERT_EQ(parser.GetIntValue("param1", 0), 1);
    ASSERT_EQ(int_values[1], 2);
    ASSERT_EQ(int_values[2], 3);
}

TEST(MinCountMultiValueTest) {
    ArgParser parser("My Parser");
    std::vector<int> int_values;
    size_t MinArgsCount = 10;
    parser.AddIntArgument('p', "param1").MultiValue(MinArgsCount).StoreValues(int_values);

    ASSERT_TRUE(!parser.Parse(SplitString("app --param1=1 --param1=2 --param1=3")));
}

TEST(FlagTest) {
    ArgParser parser("My Parser");
    parser.AddFlag('f', "flag1");

    ASSERT_TRUE(parser.Parse(SplitString("app --flag1")));
    ASSERT_TRUE(parser.GetFlag("flag1"));
}

TEST(FlagsTest) {
    ArgParser parser("My Parser");
    bool flag1 = false;
    bool flag3 = false;
    parser.AddFlag('a', "flag1").StoreValue(flag1);
    parser.AddFlag('b', "flag2").Default(true);
    parser.AddFlag('c', "flag3").StoreValue(flag3);

    ASSERT_TRUE(parser.Parse(SplitString("app -ac")));
    ASSERT_TRUE(flag1);
    ASSERT_TRUE(parser.GetFlag("flag2"));
    ASSERT_TRUE(flag3);
}

TEST(PositionalArgTest) {
    ArgParser parser("My Parser");
    std::vector<int> values;
    parser.AddIntArgument("Param1").MultiValue(1).Positional().StoreValues(values);

    ASSERT_TRUE(parser.Parse(SplitString("app 1 2 3 4 5")));
    ASSERT_EQ(values[0], 1);
    ASSERT_EQ(values[2], 3);
    ASSERT_EQ(static_cast<int>(values.size()), 5);
}

TEST(PositionalAndNormalArgTest) {
    ArgParser parser("My Parser");
    std::vector<int> values;
    parser.AddFlag('f', "flag", "Flag");
    parser.AddIntArgument('n', "number", "Some Number");
    parser.AddIntArgument("Param1").MultiValue(1).Positional().StoreValues(values);

    ASSERT_TRUE(parser.Parse(SplitString("app -n 0 1 2 3 4 5 -f")));
    ASSERT_TRUE(parser.GetFlag("flag"));
    ASSERT_EQ(parser.GetIntValue("number"), 0);
    ASSERT_EQ(values[0], 1);
    ASSERT_EQ(values[2], 3);
    ASSERT_EQ(static_cast<int>(values.size()), 5);
}

TEST(HelpTest) {
    ArgParser parser("My Parser");
    parser.AddHelp('h', "help", "Some Description about program");

    ASSERT_TRUE(parser.Parse(SplitString("app --help")));
    ASSERT_TRUE(parser.Help());
}

TEST(HelpStringTest) {
    ArgParser parser("My Parser");
    parser.AddHelp('h', "help", "Some Description about program");
    parser.AddStringArgument('i', "input", "File path for input file").MultiValue(1);
    parser.AddFlag('s', "flag1", "Use some logic").Default(true);
    parser.AddFlag('p', "flag2", "Use some logic");

    ASSERT_TRUE(parser.Parse(SplitString("app --help")));
    ASSERT_TRUE(parser.Help());
}

int main() {
    return TestFramework::TestSuite::GetInstance().RunAll() ? 0 : 1;
}
