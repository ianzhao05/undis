#include <gtest/gtest.h>

#include <filesystem>
#include <optional>
#include <string>

#include "utils.h"

#include "../undis/kvstore.h"
#include "../undis/serializer.h"

TEST(KVStoreTest, SetsAndGets) {
    KVStore db{};

    const auto m = map_factory(20);
    for (const auto &[k, v] : m) {
        db.set(k, v.str_val, v.flags);
    }
    EXPECT_EQ(db.size(), m.size());

    for (const auto &[k, v] : m) {
        auto db_val = db.get(k);
        EXPECT_TRUE(db_val.has_value());
        EXPECT_EQ(db_val->str_val, v.str_val);
    }
}

TEST(KVStoreTest, Adds) {
    KVStore db{};

    const auto m = map_factory(20);
    for (const auto &[k, v] : m) {
        EXPECT_TRUE(db.add(k, v.str_val, v.flags));
    }
    EXPECT_EQ(db.size(), m.size());

    for (const auto &[k, v] : m) {
        EXPECT_FALSE(db.add(k, v.str_val + "new", v.flags));
    }

    for (const auto &[k, v] : m) {
        auto db_val = db.get(k);
        EXPECT_TRUE(db_val.has_value());
        EXPECT_EQ(db_val->str_val, v.str_val);
    }
}

TEST(KVStoreTest, Replaces) {
    KVStore db{};

    const auto m = map_factory(20);
    for (const auto &[k, v] : m) {
        EXPECT_FALSE(db.replace(k, v.str_val, v.flags));
    }
    EXPECT_EQ(db.size(), 0);

    for (const auto &[k, v] : m) {
        db.set(k, v.str_val, v.flags);
    }

    for (const auto &[k, v] : m) {
        EXPECT_TRUE(db.replace(k, v.str_val + "new", 0));
    }

    for (const auto &[k, v] : m) {
        auto db_val = db.get(k);
        EXPECT_TRUE(db_val.has_value());
        EXPECT_EQ(db_val->str_val, v.str_val + "new");
    }
}

TEST(KVStoreTest, Deletes) {
    KVStore db{};

    const auto m = map_factory(20);
    for (const auto &[k, v] : m) {
        db.set(k, v.str_val, v.flags);
    }

    for (const auto &[k, v] : m) {
        EXPECT_TRUE(db.del(k));
    }

    EXPECT_EQ(db.size(), 0);
}

TEST(KVStoreTest, Loads) {
    const std::filesystem::path p{"KVStoreTest_Loads.db"};
    Serializer ser{p};

    const auto m = map_factory(20);
    ser << m;

    std::optional<KVStore> db{p};

    EXPECT_EQ(db->size(), m.size());
    for (const auto &[k, v] : m) {
        auto db_val = db->get(k);
        EXPECT_TRUE(db_val.has_value());
        EXPECT_EQ(db_val->str_val, v.str_val);
    }

    db.reset();

    EXPECT_TRUE(std::filesystem::remove(p));
}

TEST(KVStoreTest, Dumps) {
    const std::filesystem::path p{"KVStoreTest_Dumps.db"};
    Serializer ser{p};

    auto m1 = map_factory(20);
    std::optional<KVStore> db{p};
    for (const auto &[k, v] : m1) {
        db->set(k, v.str_val, v.flags);
    }
    db.reset();

    decltype(m1) m2;
    ser >> m2;

    EXPECT_EQ(m1, m2);

    EXPECT_TRUE(std::filesystem::remove(p));
}
