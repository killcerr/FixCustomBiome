/**
 * @file plugin.cpp
 * @brief The main file of the plugin
 */

#include <llapi/LoggerAPI.h>

#include <filesystem>
#include <fstream>

#include "version.h"

// We recommend using the global logger.
Logger logger(PLUGIN_NAME);

/**
 * @brief The entrypoint of the plugin. DO NOT remove or rename this function.
 *
 */

namespace FS {
using namespace std::filesystem;
}

struct _ClimateUtils_Parameter {
  long long max, min;
  _ClimateUtils_Parameter(long long max, long long min) {
    this->max = max, this->min = min;
  }
  _ClimateUtils_Parameter(float max, float min) {
    this->max = 10000 * max, this->min = 10000 * min;
  }
  _ClimateUtils_Parameter(std::string max, std::string min) {
    auto _max = std::stof(max);
    auto _min = std::stof(min);
    this->max = 10000 * _max, this->min = 10000 * _min;
  }
  _ClimateUtils_Parameter() { max = 0, min = 0; }
};

std::unordered_map<std::string,
                   std::pair<std::array<_ClimateUtils_Parameter, 5>, float>,
                   std::hash<std::string>>
    defs;

void PluginInit() {
  logger.info << PLUGIN_NAME << " loaded" << Logger::endl;
  const auto climateDefinitionPath = "./climatedefs.txt";
  std::fstream fin(climateDefinitionPath, std::ios::in);
  std::string biomeName;
  std::array<_ClimateUtils_Parameter, 5> parameters;
  float offset;
  for (auto i = 0ull;; i++) {
    fin >> biomeName;
    std::cout << biomeName << "\n";
    for (auto j = 0; j < 5; j++) {
      std::string max, min;
      fin >> max >> min;
      parameters[j] = _ClimateUtils_Parameter(max, min);
    }
    fin >> offset;
    defs[biomeName] = {parameters, offset};
    if (fin.peek() == EOF) {
      break;
    }
  }
  for (auto &def : defs) {
    std::cout << def.first << " ";
    for (auto &p : def.second.first) {
      std::cout << p.max << " " << p.min << " ";
    }
    std::cout << def.second.second << "\n";
  }
}

#include <llapi/HookAPI.h>
#include <llapi/mc/BiomeRegistry.hpp>
#include <llapi/mc/OverworldBiomeBuilder.hpp>

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
    a1->registerBiome(def.first);
  }
  return original(a1, a2, a3, a4);
}