#include <llapi/LoggerAPI.h>

#include <Nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

#include "version.h"

Logger logger(PLUGIN_NAME);

namespace fs {
using namespace std::filesystem;
using namespace nlohmann;
using std::fstream;
using std::getline;
using std::ios;
} // namespace fs

struct _climateUtils_parameter {
  long long max, min;
  _climateUtils_parameter(long long max, long long min) {
    this->max = max, this->min = min;
  }
  _climateUtils_parameter(float max, float min) {
    this->max = 10000 * max, this->min = 10000 * min;
  }
  _climateUtils_parameter() { max = 0, min = 0; }
};

#include <llapi/mc/Biome.hpp>
namespace std {
std::string to_string(const _climateUtils_parameter &p) {
  return "max:" + std::to_string((float)p.max / 10000) +
         " min:" + std::to_string((float)p.min / 10000);
}
std::string to_string(const Biome &b) {
  return "name:" + b.getName() + " id:" + std::to_string(b.getId());
}
} // namespace std

struct biome_data_t {
  std::array<_climateUtils_parameter, 5> first;
  float second;
  std::string type;
};

std::unordered_map<std::string, biome_data_t, std::hash<std::string>> defs;

#define LOG_KEY_NOT_FOUND(KEY) logger.info("key not found:{}", #KEY);

void load_single_climate_def(fs::path p) {
  logger.info(__func__);
  std::wcout << p.c_str() << "\n";
  std::error_code ec;
  auto s = fs::status(p, ec);
  if (ec)
    return;
  if (s.type() != fs::file_type::regular)
    return;
  fs::fstream fin(p, fs::ios::in);
  std::string d, t;
  while (fs::getline(fin, t)) {
    d += t;
    d += "\n";
  }
  fin.close();
  fs::json j = fs::json::parse(d, nullptr, true, true);
  if (!j.is_object())
    return;
  logger.info << j << Logger::endl;
  for (auto &i : j.items()) {
    _climateUtils_parameter temperature, humidity, continentalness, erosion,
        weirdness;
    std::string type;
    float offset;
    try {
      temperature.max =
          i.value().at("temperature").at("max").get<double>() * 10000;
    } catch (...) {
      LOG_KEY_NOT_FOUND(temperature.max)
    }
    try {
      temperature.min =
          i.value().at("temperature").at("min").get<double>() * 10000;
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
      continentalness.max =
          i.value().at("continentalness").at("max").get<double>() * 10000;
    } catch (...) {
      LOG_KEY_NOT_FOUND(continentalness.max)
    }
    try {
      continentalness.min =
          i.value().at("continentalness").at("min").get<double>() * 10000;
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
    logger.info(i.key());
    defs[(i.key())].first[0] = temperature;
    defs[(i.key())].first[1] = humidity;
    defs[(i.key())].first[2] = continentalness;
    defs[(i.key())].first[3] = erosion;
    defs[(i.key())].first[4] = weirdness;
    defs[(i.key())].second = offset;
    defs[(i.key())].type = type;
    logger.info(__func__);
  }
}

void load_all_defs() {
  logger.info(__func__);
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
  for (auto &i : it) {
    if (i.is_directory()) {
      fs::path p = i;
      p = p / "climate_defs.json";
      load_single_climate_def(p);
    }
  }
  logger.info(__func__);
}

#include <llapi/Global.h>
#include <llapi/ScheduleAPI.h>
#include <llapi/mc/Level.hpp>
#include <llapi/mc/Player.hpp>

std::string bn;

void plugin_init() {
  logger.info << PLUGIN_NAME << " loaded" << Logger::endl;
  try {
    load_all_defs();
  } catch (std::exception &e) {
    logger.error(e.what());
  }
}

#include <llapi/HookAPI.h>
#include <llapi/mc/BiomeRegistry.hpp>
#include <llapi/mc/OverworldBiomeBuilder.hpp>
#include <llapi/mc/VanillaBiomes.hpp>

struct _BiomeNoiseTarget {
  char filler[224];
};

THook(void,
      "?addBiomes@OverworldBiomeBuilder@@QEBAXAEAV?$vector@UBiomeNoiseTarget@@"
      "V?$allocator@UBiomeNoiseTarget@@@std@@@std@@AEBVBiomeRegistry@@@Z",
      OverworldBiomeBuilder *_this, std::vector<_BiomeNoiseTarget> &a1,
      BiomeRegistry &a2) {
  original(_this, a1, a2);
  for (auto &def : defs) {
    auto biome = a2.lookupByName(def.first);
    if (def.second.type == "underground")
      SymCall("?_addUndergroundBiome@OverworldBiomeBuilder@@AEBAXAEAV?$vector@"
              "UBiomeNoiseTarget@@V?$allocator@UBiomeNoiseTarget@@@std@@@std@@"
              "AEBUParameter@ClimateUtils@@1111MPEAVBiome@@@Z",
              void, OverworldBiomeBuilder *, std::vector<_BiomeNoiseTarget> &,
              void *, void *, void *, void *, void *, float,
              void *)(_this, a1, &def.second.first[0], &def.second.first[1],
                      &def.second.first[2], &def.second.first[3],
                      &def.second.first[4], def.second.second, biome);
    else if (def.second.type == "surface")
      SymCall("?_addSurfaceBiome@OverworldBiomeBuilder@@AEBAXAEAV?$vector@"
              "UBiomeNoiseTarget@@V?$allocator@UBiomeNoiseTarget@@@std@@@std@@"
              "AEBUParameter@ClimateUtils@@1111MPEAVBiome@@@Z",
              void, OverworldBiomeBuilder *, std::vector<_BiomeNoiseTarget> &,
              void *, void *, void *, void *, void *, float,
              void *)(_this, a1, &def.second.first[0], &def.second.first[1],
                      &def.second.first[2], &def.second.first[3],
                      &def.second.first[4], def.second.second, biome);
  }
}

#include <llapi/mc/Experiments.hpp>

THook(bool, "?DataDrivenBiomes@Experiments@@QEBA_NXZ", void *_this) {
  return true;
}

THook(void,
      "?initBiomes@VanillaBiomes@@SAXAEAVBiomeRegistry@@AEBUSpawnSettings@@"
      "AEBVBaseGameVersion@@AEBVExperiments@@@Z",
      class BiomeRegistry *a1, struct SpawnSettings const &a2,
      class BaseGameVersion const &a3, class Experiments const &a4) {
  for (auto &def : defs) {
    logger.info("biome:{}", def.first);
    a1->registerBiome(def.first);
  }
  return original(a1, a2, a3, a4);
}