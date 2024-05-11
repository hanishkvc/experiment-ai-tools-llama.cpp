#pragma once

/**
 * Allows grouping key-value pairs into groups.
 * by Humans for All
 * 
 * Allows one to maintain a bunch of groups, where each group contains key-value pairs in them.
 * The values could belong to any one of these types ie strings, int, float and bool.
 * 
 * It also tries to provide a crude expanded form of array wrt any of the above supported types.
 * For this one needs to define keys using the pattern TheKeyName-0, TheKeyName-1, ....
 */

#include <map>
#include <string>
#include <vector>
#include <variant>
#include <sstream>


#define GKV_DEBUG

#define GKV_TEST_PRG
#ifdef GKV_TEST_PRG
#include <iostream>
#include <format>
#define LINFO_LN(FMT, ...) fprintf(stdout, FMT"\n", ##__VA_ARGS__)
#define LDBUG(FMT, ...) fprintf(stderr, FMT, ##__VA_ARGS__)
#define LDBUG_LN(FMT, ...) fprintf(stderr, FMT"\n", ##__VA_ARGS__)
#define LERRR_LN(FMT, ...) fprintf(stderr, FMT"\n", ##__VA_ARGS__)
#define LWARN_LN(FMT, ...) fprintf(stderr, FMT"\n", ##__VA_ARGS__)
#else
#include "log.h"
#define LINFO_LN LOG_TEELN
#define LDBUG LOG
#define LDBUG_LN LOGLN
#define LERRR_LN LOG_TEELN
#define LWARN_LN LOG_TEELN
#endif


typedef std::variant<std::string, bool, int64_t, double> GroupKVData;
typedef std::vector<std::string> MultiPart;
typedef std::map<std::string, std::map<std::string, GroupKVData>> GroupKVMapMapVariant;

class GroupKV {

private:

    GroupKVMapMapVariant mapV = {};

public:

    GroupKV(GroupKVMapMapVariant defaultMap) : mapV(defaultMap) {}

    static std::string joiner(const MultiPart& parts) {
        std::stringstream joined;
        int iCnt = 0;
        for(auto part: parts) {
            if (iCnt != 0) {
                joined << "-";
            }
            iCnt += 1;
            joined << part;
        }
        return joined.str();
    }

    std::string to_str(const GroupKVData &value) {
        auto visitor = [](auto value) -> auto {
            std::stringstream ss;
            ss << value;
            return ss.str();
        };
        return std::visit(visitor, value);
    }

    template<typename TypeWithStrSupp>
    std::string to_str(std::vector<TypeWithStrSupp> values) {
        std::stringstream ss;
        ss << "[ ";
        int cnt = 0;
        for(auto value: values) {
            cnt += 1;
            if (cnt != 1) ss << ", ";
            ss << value;
        }
        ss << " ]";
        return ss.str();
    }


    template<typename SupportedDataType>
    void set_value(const std::string &group, const MultiPart &keyParts, const SupportedDataType &value, const std::string &callerName="") {
        auto key = joiner(keyParts);
        auto &gm = mapV[group];
        gm[key] = value;
#ifdef GKV_DEBUG
        LDBUG_LN("DBUG:GKV:%s_%s:%s:%s:%s", __func__, callerName.c_str(), group.c_str(), key.c_str(), to_str(value).c_str());
#endif
    }

    // Dump info about the specified group.
    // If group is empty, then dump info about all groups maintained in this instance.
    void dump(const std::string &group) {
        for (auto gm: mapV) {
            if (!group.empty() && (gm.first != group)) {
                LINFO_LN("INFO:GKV:%s:%s:Skipping...", __func__, gm.first.c_str());
                continue;
            }
            for(auto k: gm.second) {
                LINFO_LN("DBUG:GKV:%s:%s:Iterate:%s:%s", __func__, gm.first.c_str(), k.first.c_str(), to_str(k.second).c_str());
            }
        }
    }

    template<typename SupportedDataType>
    SupportedDataType get_value(const std::string &group, const MultiPart &keyParts, const SupportedDataType &defaultValue, const std::string &callerName="") {
        auto key = joiner(keyParts);
        auto gm = mapV[group];
        if (gm.find(key) == gm.end()) {
#ifdef GKV_DEBUG
            LWARN_LN("WARN:GKV:%s_%s:%s:%s:%s[default]", __func__, callerName.c_str(), group.c_str(), key.c_str(), to_str(defaultValue).c_str());
#endif
            return defaultValue;
        }
        auto value = gm[key];
#ifdef GKV_DEBUG
        LDBUG_LN("DBUG:GKV:%s_%s:%s:%s:%s", __func__, callerName.c_str(), group.c_str(), key.c_str(), to_str(value).c_str());
#endif
        return std::get<SupportedDataType>(value);
    }

    template<typename SupportedDataType>
    std::vector<SupportedDataType> get_vector(const std::string &group, const MultiPart &keyParts, const std::vector<SupportedDataType> &defaultValue, const std::string &callerName="") {
        auto key = joiner(keyParts);
        auto gm = mapV[group];
        std::vector<SupportedDataType> array;
        int i = 0;
        while(true) {
            std::stringstream ssArrayKey;
            ssArrayKey << key << "-" << i;
            auto arrayKey = ssArrayKey.str();
            if (gm.find(arrayKey) == gm.end()) {
                break;
            }
            array.push_back(std::get<SupportedDataType>(gm[arrayKey]));
            i += 1;
        }
        if (array.empty()) {
#ifdef GKV_DEBUG
            LWARN_LN("DBUG:GKV:%s_%s:%s:%s:%s[default]", __func__, callerName.c_str(), group.c_str(), key.c_str(), to_str(defaultValue).c_str());
#endif
            return defaultValue;
        }
#ifdef GKV_DEBUG
        LDBUG_LN("DBUG:SC:%s_%s:%s:%s:%s", __func__, callerName.c_str(), group.c_str(), key.c_str(), to_str(array).c_str());
#endif
        return array;
    }

};


#ifdef GKV_TEST_PRG


void gkv_inited() {
    GroupKV gkv = {{
        {"Group1",{
            {"testkey11", 11},
            {"testkey12", true}
        }},
        {"Group2", {
            {"key21", "val21"},
            {"key22", 22},
            {"key23", 2.3}
        }}
    }};

    std::cout << "**** gkv inited **** " << std::endl;
    gkv.dump("");

}

void gkv_set() {

    std::cout << "**** gkv set **** " << std::endl;
    GroupKV gkv = {{}};
    gkv.dump("");

    gkv.get_value("testme", {"key101b"}, false);
    gkv.get_value<std::string>("testme", {"key101s"}, "Not found");
    gkv.get_value<int64_t>("testme", {"key101i"}, 123456);
    gkv.get_value("testme", {"key101d"}, 123456.789);

    gkv.set_value("testme", {"key201b"}, true);
    gkv.set_value("testme", {"key201s"}, "hello world");
    gkv.set_value("testme", {"key201i"}, 987654);
    gkv.set_value("testme", {"key201d"}, 9988.7766);

    gkv.dump("testme");
    gkv.get_value("testme", {"key201b"}, false);
    gkv.get_value<std::string>("testme", {"key201s"}, "Not found");
    gkv.get_value("testme", {"key201i"}, 123456LL);
    gkv.get_value("testme", {"key201d"}, 123456.789);

    gkv.get_vector<int64_t>("testme", {"keyA100"}, {1, 2, 3});
    gkv.get_vector<std::string>("testme", {"keyA100"}, { "A", "അ", "अ", "ಅ" });
    gkv.set_value("testme", {"keyA300-0"}, 330);
    gkv.set_value("testme", {"keyA300-1"}, 331);
    gkv.set_value("testme", {"keyA300-2"}, 332);
    gkv.set_value("testme", {"keyA301-0"}, "India");
    gkv.set_value<std::string>("testme", {"keyA301", "1"}, "World");
    gkv.set_value("testme", {"keyA301", "2"}, "AkashaGanga");
    gkv.get_vector<int64_t>("testme", {"keyA300"}, {1, 2, 3});
    gkv.get_vector<std::string>("testme", {"keyA301"}, { "yes 1", "No 2", "very well 3" });
}

int main(int argc, char **argv) {
    gkv_inited();
    gkv_set();
    return 0;
}
#endif
