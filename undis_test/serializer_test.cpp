#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <unordered_map>

#include "utils.h"

#include "../undis/serializer.h"
#include "../undis/storevalue.h"

TEST(SerializerTest, CreatesFile) {
    const std::filesystem::path p{"SerializerTest_CreatesFile.db"};
    Serializer ser{p};

    std::unordered_map<std::string, StoreValue> m;
    ser << m;

    EXPECT_TRUE(std::filesystem::remove(p));
}

TEST(SerializerTest, DumpsAndLoads) {
    const std::filesystem::path p{"SerializerTest_DumpsAndLoads.db"};
    Serializer ser{p};

    auto m1 = map_factory(10);
    decltype(m1) m2;
    ser << m1;
    ser >> m2;
    EXPECT_EQ(m1, m2);

    EXPECT_TRUE(std::filesystem::remove(p));
}
