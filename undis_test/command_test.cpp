#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

using namespace std::literals;

#include "../undis/command.h"
#include "../undis/kvstore.h"

class CommandTest : public ::testing::Test {
  protected:
    void SetUp() override { store.set("exists", "value", 42u, 0); }
    KVStore store;
};

TEST_F(CommandTest, EmptyIsInvalid) {
    Command c{};
    EXPECT_EQ(c.status(), Command::invalid_command);
}

TEST_F(CommandTest, NonexistentCommand) {
    Command c{"foo bar baz"};
    EXPECT_EQ(c.status(), Command::invalid_command);
    EXPECT_EQ(c.set_command("Get foo"), Command::invalid_command);
}

TEST_F(CommandTest, GetCommand) {
    Command c{"get exists not_exists"};
    EXPECT_EQ(c.execute(store), "VALUE exists 42 5\r\nvalue\r\nEND\r\n");
}

TEST_F(CommandTest, DeleteCommand) {
    Command c{"delete exists"};
    EXPECT_EQ(c.execute(store), "DELETED\r\n");
    c.set_command("delete exists");
    EXPECT_EQ(c.execute(store), "NOT_FOUND\r\n");
}

TEST_F(CommandTest, SetCommand) {
    Command c{"set exists 43 1000 9"};
    EXPECT_EQ(c.status(), Command::data_required);
    EXPECT_EQ(c.execute(store, "new_value"), "STORED\r\n");
    auto v = store.get("exists");
    EXPECT_EQ(v.value().str_val, "new_value");
    EXPECT_EQ(v.value().flags, 43u);
    EXPECT_NE(v.value().exp_time, static_cast<unsigned>(-1));
}

TEST_F(CommandTest, AddCommand) {
    Command c{"add exists 0 0 9"};
    EXPECT_EQ(c.status(), Command::data_required);
    EXPECT_EQ(c.execute(store, "new_value"), "NOT_STORED\r\n");

    c.set_command("add not_exists 0 0 5");
    EXPECT_EQ(c.execute(store, "value"), "STORED\r\n");
    EXPECT_EQ(store.get("not_exists").value().str_val, "value");
}

TEST_F(CommandTest, ReplaceCommand) {
    Command c{"replace exists 0 0 9"};
    EXPECT_EQ(c.status(), Command::data_required);
    EXPECT_EQ(c.execute(store, "new_value"), "STORED\r\n");
    EXPECT_EQ(store.get("exists").value().str_val, "new_value");

    c.set_command("replace not_exists 0 0 5");
    EXPECT_EQ(c.execute(store, "value"), "NOT_STORED\r\n");
    EXPECT_FALSE(store.get("not_exists").has_value());
}

TEST_F(CommandTest, AppendPrependCommands) {
    Command c{"append exists 0 0 7"};
    EXPECT_EQ(c.status(), Command::data_required);
    EXPECT_EQ(c.execute(store, "_suffix"), "STORED\r\n");
    EXPECT_EQ(store.get("exists").value().str_val, "value_suffix");

    c.set_command("prepend exists 0 0 7");
    EXPECT_EQ(c.status(), Command::data_required);
    EXPECT_EQ(c.execute(store, "prefix_"s), "STORED\r\n");
    EXPECT_EQ(store.get("exists").value().str_val, "prefix_value_suffix");

    c.set_command("append not_exists 0 0 7");
    EXPECT_EQ(c.execute(store, "_suffix"), "NOT_STORED\r\n");

    c.set_command("prepend not_exists 0 0 7");
    EXPECT_EQ(c.execute(store, "prefix_"), "NOT_STORED\r\n");
}

TEST_F(CommandTest, ChecksBytes) {
    Command c{"set exists 0 0 3"};
    EXPECT_THROW(c.execute(store, "four"), std::invalid_argument);
}

TEST_F(CommandTest, BadCommandErrors) {
    Command c;
    EXPECT_EQ(c.set_command("get"), Command::invalid_command);
    EXPECT_EQ(c.set_command("delete"), Command::invalid_command);
    EXPECT_EQ(c.set_command("set"), Command::invalid_command);
    EXPECT_EQ(c.set_command("add exists"), Command::invalid_command);
    EXPECT_EQ(c.set_command("replace exists 0"), Command::invalid_command);
    EXPECT_EQ(c.set_command("append exists 0 0"), Command::invalid_command);
    EXPECT_EQ(c.set_command("prepend exists 0 0 -1"), Command::invalid_command);
}
