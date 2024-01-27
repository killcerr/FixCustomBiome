#include "Plugin.h"

#include <Nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <ll/api/plugin/Plugin.h>
#include <string>
using namespace std::string_view_literals;
ll::Logger logger("FixCustomBiome"sv);

namespace fs {
using namespace std::filesystem;
using namespace nlohmann;
using std::fstream;
using std::getline;
using std::ios;
} // namespace fs

struct _climateUtils_parameter {
    long long max, min;
    _climateUtils_parameter(long long max, long long min) { this->max = max, this->min = min; }
    _climateUtils_parameter(float max, float min) { this->max = 10000 * max, this->min = 10000 * min; }
    _climateUtils_parameter() { max = 0, min = 0; }
};

// #include <llapi/mc/Biome.hpp>
#include <mc/world/level/biome/Biome.h>
namespace std {
std::string to_string(const _climateUtils_parameter& p) { return "max:" + std::to_string((float)p.max / 10000) + " min:" + std::to_string((float)p.min / 10000); }
std::string to_string(const Biome& b) { return "name:" + b.getName() + " id:" + std::to_string(b.getId()); }
} // namespace std

struct biome_data_t {
    std::array<_climateUtils_parameter, 5> parameter;
    float                                  offset;
    std::string                            type;
};

std::unordered_map<std::string, biome_data_t, std::hash<std::string>> defs, modification_defs;

#define LOG_KEY_NOT_FOUND(KEY) logger.error("key not found:{}", #KEY);

void load_single_climate_def(fs::path p) {
    // logger.info(__func__);
    std::wcout << p.c_str() << "\n";
    std::error_code ec;
    auto            s = fs::status(p, ec);
    if (ec) return;
    if (s.type() != fs::file_type::regular) return;
    fs::fstream fin(p, fs::ios::in);
    std::string d, t;
    while (fs::getline(fin, t)) {
        d += t;
        d += "\n";
    }
    fin.close();
    fs::json j = fs::json::parse(d, nullptr, true, true);
    if (!j.is_object()) return;
    logger.info("{}", j.dump(4));
    for (auto& i : j.items()) {
        _climateUtils_parameter temperature, humidity, continentalness, erosion, weirdness;
        std::string             type;
        float                   offset;
        try {
            temperature.max = i.value().at("temperature").at("max").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(temperature.max)
        }
        try {
            temperature.min = i.value().at("temperature").at("min").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(temperature.min)
        }
        try {
            humidity.max = i.value().at("humidity").at("max").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(humidity.max)
        }
        try {
            humidity.min = i.value().at("humidity").at("min").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(humidity.min)
        }
        try {
            continentalness.max = i.value().at("continentalness").at("max").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(continentalness.max)
        }
        try {
            continentalness.min = i.value().at("continentalness").at("min").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(continentalness.min)
        }
        try {
            erosion.max = i.value().at("erosion").at("max").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(erosion.max)
        }
        try {
            erosion.min = i.value().at("erosion").at("min").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(erosion.min)
        }
        try {
            weirdness.max = i.value().at("weirdness").at("max").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(weirdness.max)
        }
        try {
            weirdness.min = i.value().at("weirdness").at("min").get<float>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(weirdness.min)
        }
        try {
            offset = i.value().at("offset").get<float>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(offset)
        }
        try {
            type = i.value().at("type").get<std::string>();
        } catch (...) {
            LOG_KEY_NOT_FOUND(type);
        }
        // logger.info(i.key());
        defs[(i.key())].parameter[0] = temperature;
        defs[(i.key())].parameter[1] = humidity;
        defs[(i.key())].parameter[2] = continentalness;
        defs[(i.key())].parameter[3] = erosion;
        defs[(i.key())].parameter[4] = weirdness;
        defs[(i.key())].offset       = offset;
        defs[(i.key())].type         = type;
        // logger.info(__func__);
    }
}

void load_signal_biome_modification_defs(fs::path p) {
    std::wcout << p.c_str() << "\n";
    std::error_code ec;
    auto            s = fs::status(p, ec);
    if (ec) return;
    if (s.type() != fs::file_type::regular) return;
    fs::fstream fin(p, fs::ios::in);
    std::string d, t;
    while (fs::getline(fin, t)) {
        d += t;
        d += "\n";
    }
    fin.close();
    fs::json j = fs::json::parse(d, nullptr, true, true);
    if (!j.is_object()) return;
    for (auto& i : j.items()) {
        _climateUtils_parameter temperature, humidity, continentalness, erosion, weirdness;
        std::string             type;
        float                   offset;
        try {
            temperature.max = i.value().at("temperature").at("max").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(temperature.max)
        }
        try {
            temperature.min = i.value().at("temperature").at("min").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(temperature.min)
        }
        try {
            humidity.max = i.value().at("humidity").at("max").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(humidity.max)
        }
        try {
            humidity.min = i.value().at("humidity").at("min").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(humidity.min)
        }
        try {
            continentalness.max = i.value().at("continentalness").at("max").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(continentalness.max)
        }
        try {
            continentalness.min = i.value().at("continentalness").at("min").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(continentalness.min)
        }
        try {
            erosion.max = i.value().at("erosion").at("max").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(erosion.max)
        }
        try {
            erosion.min = i.value().at("erosion").at("min").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(erosion.min)
        }
        try {
            weirdness.max = i.value().at("weirdness").at("max").get<double>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(weirdness.max)
        }
        try {
            weirdness.min = i.value().at("weirdness").at("min").get<float>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(weirdness.min)
        }
        try {
            offset = i.value().at("offset").get<float>() * 10000;
        } catch (...) {
            LOG_KEY_NOT_FOUND(offset)
        }
        // try {
        //   type = i.value().at("type").get<std::string>();
        // } catch (...) {
        //   LOG_KEY_NOT_FOUND(type);
        // }
        modification_defs[(i.key())].parameter[0] = temperature;
        modification_defs[(i.key())].parameter[1] = humidity;
        modification_defs[(i.key())].parameter[2] = continentalness;
        modification_defs[(i.key())].parameter[3] = erosion;
        modification_defs[(i.key())].parameter[4] = weirdness;
        modification_defs[(i.key())].offset       = offset;
        // modification_defs[(i.key())].type = type;
    }
}

// #include <llapi/DynamicCommandAPI.h>
#include <ll/api/command/DynamicCommand.h>
// #include <llapi/RegCommandAPI.h>
#include <ll/api/command/CommandRegistrar.h>
// #include <llapi/mc/Command.hpp>

void load_all_defs() {
    // logger.info(__func__);
    fs::fstream fin("./server.properties");
    std::string t, level_name;
    while (fs::getline(fin, t)) {
        std::string d;
        for (auto c : t) {
            if (c == '#') {
                break;
            }
            d += c;
        }
        if (d.find("level-name") != d.npos) {
            auto i = d.find("=") + 1;
            for (; i < d.size(); i++) {
                level_name += d[i];
            }
            break;
        }
    }
    fs::directory_iterator it("./worlds/" + level_name + "/behavior_packs/");
    for (auto& i : it) {
        if (i.is_directory()) {
            fs::path p = i;
            p          = p / "climate_defs.json";
            load_single_climate_def(p);
        }
    }
    // logger.info(__func__);
}

// #include <llapi/Global.h>
#include <ll/api/service/Bedrock.h>
// #include <llapi/ScheduleAPI.h>
#include <ll/api/schedule/Scheduler.h>
// #include <llapi/mc/BiomeRegistry.hpp>
#include <mc/world/level/biome/registry/BiomeRegistry.h>
// #include <llapi/mc/Level.hpp>
#include <mc/world/level/Level.h>
// #include <llapi/mc/Player.hpp>
#include <mc/world/actor/player/Player.h>


struct biome_description_t {
    int         id;
    std::string name;
};
namespace std {
std::string to_string(const biome_description_t& d) { return "id:" + std::to_string(d.id) + " name:" + d.name; }
template <typename T>
std::string to_string(const std::vector<T>& v) {
    std::string r;
    for (auto& i : v) {
        r += std::to_string(i);
        r += "\n";
    }
    return r;
}
} // namespace std

std::string bn;
#include <ll/api/event/EventBus.h>
// #include <ll/api/event/world/ServerStartedEvent.h>
#include <ll/api/event/server/ServerStartedEvent.h>
void plugin_init() {
    logger.info("FixCustomBiome loaded");
    try {
        load_all_defs();
    } catch (std::exception& e) {
        logger.error(e.what());
    }
    // DynamicCommand::setup(
    //     ll::service::getCommandRegistry(),
    //     "biomelist",
    //     "print biome list",
    //     {},
    //     {},
    //     {
    //         {},
    //     },
    //     [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output, std::unordered_map<std::string, DynamicCommand::Result>& results) {
    //         std::vector<biome_description_t> ds;
    //         origin.getLevel()->getBiomeRegistry().forEachBiome([&ds](Biome& b) { ds.push_back({b.getId(), b.getName()}); });
    //         output.success(std::to_string(ds));
    //     }
    // );
}

// #include <llapi/HookAPI.h>
#include <ll/api/memory/Hook.h>
// #include <llapi/mc/BiomeRegistry.hpp>
#include <mc/world/level/biome/registry/BiomeRegistry.h>
// #include <llapi/mc/OverworldBiomeBuilder.hpp>
#include <mc/world/level/levelgen/v1/OverworldBiomeBuilder.h>
// #include <llapi/mc/VanillaBiomes.hpp>
#include <mc/world/level/biome/VanillaBiomes.h>

struct _BiomeNoiseTarget {
    char filler[224];
};

// THook(
//     void,
//     "?addBiomes@OverworldBiomeBuilder@@QEBAXAEAV?$vector@UBiomeNoiseTarget@@"
//     "V?$allocator@UBiomeNoiseTarget@@@std@@@std@@AEBVBiomeRegistry@@@Z",
//     OverworldBiomeBuilder*          _this,
//     std::vector<_BiomeNoiseTarget>& a1,
//     BiomeRegistry&                  a2
// ) {
//     original(_this, a1, a2);
//     for (auto& def : defs) {
//         auto biome = a2.lookupByName(def.first);
//         if (def.second.type == "underground")
//             SymCall("?_addUndergroundBiome@OverworldBiomeBuilder@@AEBAXAEAV?$vector@"
//               "UBiomeNoiseTarget@@V?$allocator@UBiomeNoiseTarget@@@std@@@std@@"
//               "AEBUParameter@ClimateUtils@@1111MPEAVBiome@@@Z",
//               void, OverworldBiomeBuilder *, std::vector<_BiomeNoiseTarget> &,
//               void *, void *, void *, void *, void *, float, void *)(
//           _this, a1, &def.second.parameter[0], &def.second.parameter[1],
//           &def.second.parameter[2], &def.second.parameter[3],
//           &def.second.parameter[4], def.second.offset, biome);
//         else if (def.second.type == "surface")
//             SymCall("?_addSurfaceBiome@OverworldBiomeBuilder@@AEBAXAEAV?$vector@"
//               "UBiomeNoiseTarget@@V?$allocator@UBiomeNoiseTarget@@@std@@@std@@"
//               "AEBUParameter@ClimateUtils@@1111MPEAVBiome@@@Z",
//               void, OverworldBiomeBuilder *, std::vector<_BiomeNoiseTarget> &,
//               void *, void *, void *, void *, void *, float, void *)(
//           _this, a1, &def.second.parameter[0], &def.second.parameter[1],
//           &def.second.parameter[2], &def.second.parameter[3],
//           &def.second.parameter[4], def.second.offset, biome);
//     }
// }

LL_AUTO_TYPE_INSTANCE_HOOK(add_biomes_hook, HookPriority::Normal, OverworldBiomeBuilder, "?addBiomes@OverworldBiomeBuilder@@QEBAXAEAV?$vector@UBiomeNoiseTarget@@V?$allocator@UBiomeNoiseTarget@@@std@@@std@@AEBVBiomeRegistry@@@Z", void, std::vector<_BiomeNoiseTarget>& a1, BiomeRegistry& a2) {
    std::cout << __LINE__ << '\n';
    origin(a1, a2);
    for (auto& def : defs) {
        std::cout << __LINE__ << " " << def.first << '\n';
        auto biome = a2.lookupByName(def.first);
        if (def.second.type == "underground")
            LL_SYMBOL_CALL("?_addUndergroundBiome@OverworldBiomeBuilder@@AEBAXAEAV?$vector@"
                  "UBiomeNoiseTarget@@V?$allocator@UBiomeNoiseTarget@@@std@@@std@@"
                  "AEBUParameter@ClimateUtils@@1111MPEAVBiome@@@Z",
                  void, OverworldBiomeBuilder *, std::vector<_BiomeNoiseTarget> &,
                  void *, void *, void *, void *, void *, float, void *)(
              this, a1, &def.second.parameter[0], &def.second.parameter[1],
              &def.second.parameter[2], &def.second.parameter[3],
              &def.second.parameter[4], def.second.offset, biome);
        else if (def.second.type == "surface")
            LL_SYMBOL_CALL("?_addSurfaceBiome@OverworldBiomeBuilder@@AEBAXAEAV?$vector@"
                  "UBiomeNoiseTarget@@V?$allocator@UBiomeNoiseTarget@@@std@@@std@@"
                  "AEBUParameter@ClimateUtils@@1111MPEAVBiome@@@Z",
                  void, OverworldBiomeBuilder *, std::vector<_BiomeNoiseTarget> &,
                  void *, void *, void *, void *, void *, float, void *)(
              this, a1, &def.second.parameter[0], &def.second.parameter[1],
              &def.second.parameter[2], &def.second.parameter[3],
              &def.second.parameter[4], def.second.offset, biome);
    }
}

// #include <llapi/mc/Experiments.hpp>
#include <mc/world/level/storage/Experiments.h>

// THook(bool, "?DataDrivenBiomes@Experiments@@QEBA_NXZ", void* _this) { return true; }

// THook(
//     void,
//     "?initBiomes@VanillaBiomes@@SAXAEAVBiomeRegistry@@AEBUSpawnSettings@@"
//     "AEBVBaseGameVersion@@AEBVExperiments@@@Z",
//     class BiomeRegistry*         a1,
//     struct SpawnSettings const&  a2,
//     class BaseGameVersion const& a3,
//     class Experiments const&     a4
// ) {
//     for (auto& def : defs) {
//         logger.info("biome:{}", def.first);
//         a1->registerBiome(def.first);
//     }
//     return original(a1, a2, a3, a4);
// }

LL_AUTO_TYPE_STATIC_HOOK(init_biomes_hook, HookPriority::Normal, VanillaBiomes, "?initBiomes@VanillaBiomes@@SAXAEAVBiomeRegistry@@AEBUSpawnSettings@@AEBVBaseGameVersion@@AEBVExperiments@@@Z", void, class BiomeRegistry* a1, struct SpawnSettings const& a2, class BaseGameVersion const& a3, class Experiments const& a4) {
    std::cout << __LINE__ << '\n';

    for (auto& def : defs) {
        // (*logger).info("biome:{}", def.first);
        std::cout << __LINE__ << " " << def.first << '\n';
        a1->registerBiome(def.first);
    }
    return origin(a1, a2, a3, a4);
}

#include <mc/world/events/ServerInstanceEventCoordinator.h>

LL_AUTO_TYPE_INSTANCE_HOOK(server_started_hook, HookPriority::Normal, ServerInstanceEventCoordinator, &ServerInstanceEventCoordinator::sendServerThreadStarted, void, class ServerInstance& ins) {
    std::cout << __LINE__ << '\n';
    origin(ins);
    DynamicCommand::setup(
        ll::service::getCommandRegistry(),
        "biomelist",
        "print biome list",
        {},
        {},
        {
            {},
        },
        [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output, std::unordered_map<std::string, DynamicCommand::Result>& results) {
            std::vector<biome_description_t> ds;
            origin.getLevel()->getBiomeRegistry().forEachBiome([&ds](Biome& b) { ds.push_back({b.getId(), b.getName()}); });
            output.success(std::to_string(ds));
        }
    );
}
