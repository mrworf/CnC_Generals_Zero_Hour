// SPDX-License-Identifier: GPL-3.0-or-later
// Actual base creator and shared creator-macro ownership support, not gameplay
// or whole ModuleFactory/ThingFactory acceptance.
#include "AllocationFault.h"
#include "Common/FileSystem.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/Module.h"
#include "Common/NativeModuleData.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>
#include <unistd.h>

namespace {
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
struct CandidateData final : ModuleData {
  static inline unsigned live = 0;
  std::vector<Int> constructorBacking = std::vector<Int>(32, 7);
  AsciiString label;
  Int count = 0;
  CandidateData() { ++live; }
  ~CandidateData() override { --live; }
  static void parseLabel(INI* ini, void* instance, void*, const void*) {
    auto* value = static_cast<CandidateData*>(instance);
    INI::parseAsciiString(ini, instance, &value->label, nullptr);
  }
  static void parseCount(INI* ini, void* instance, void*, const void*) {
    auto* value = static_cast<CandidateData*>(instance);
    INI::parseInt(ini, instance, &value->count, nullptr);
  }
  static void buildFieldParse(MultiIniFieldParse& fields) {
    ModuleData::buildFieldParse(fields);
    static const FieldParse table[] = {
      {"Label", parseLabel, nullptr, 0},
      {"Count", parseCount, nullptr, 0},
      {nullptr, nullptr, nullptr, 0}
    };
    fields.add(table);
  }
};
// Executes the real shared production macro; this is deliberately a generated
// data shape, not a replacement gameplay module or factory.
struct CandidateCreator {
  const ModuleData* getModuleData() const { return nullptr; }
  MAKE_STANDARD_MODULE_DATA_MACRO_ABC(CandidateCreator, CandidateData)
};
thread_local std::unique_ptr<ModuleData>* publication = nullptr;
void parseBase(INI* ini) {
  std::unique_ptr<ModuleData> value(Module::friend_newModuleData(ini));
  publication->swap(value);
}
void parseCandidate(INI* ini) {
  std::unique_ptr<ModuleData> value(CandidateCreator::friend_newModuleData(ini));
  publication->swap(value);
}
struct Context {
  std::filesystem::path root;
  FileSystem files;
  std::unique_ptr<ModuleData> accepted;
  Context() {
    char pattern[] = "/tmp/zh-module-data-XXXXXX";
    const char* directory = ::mkdtemp(pattern);
    require(directory, "generated module-data root");
    root = directory;
    try {
      put("base.ini", "Base\nEnd\n");
      put("base-bad.ini", "Base\nUnknown = late\nEnd\n");
      put("candidate.ini", "Candidate\nLabel = generated-owned-module-label-with-backing\nCount = 19\nEnd\n");
      put("bad.ini", "Candidate\nLabel = generated-owned-module-label-with-backing\nCount = 19\nUnknown = late\nEnd\n");
      files.mountReadOnly({root.string()});
      TheFileSystem = &files;
      publication = &accepted;
    } catch (...) {
      std::error_code ignored;
      std::filesystem::remove_all(root, ignored);
      throw;
    }
  }
  ~Context() {
    publication = nullptr;
    TheFileSystem = nullptr;
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
  }
  void put(const char* filename, const char* bytes) {
    std::ofstream stream(root / filename, std::ios::binary);
    stream << bytes;
    require(bool(stream), "generated module input");
  }
  void load(const char* filename) {
    const INIBlockDefinition blocks[] = {{"Base", parseBase}, {"Candidate", parseCandidate}};
    INI ini;
    ini.loadBlocks(filename, INI_LOAD_OVERWRITE, blocks);
  }
};
void values() {
  std::unique_ptr<ModuleData> base(Module::friend_newModuleData(nullptr));
  require(base->getModuleTagNameKey() == NAMEKEY_INVALID, "actual base default tag");
  base->setModuleTagNameKey(NameKeyType(17));
  require(base->getModuleTagNameKey() == NameKeyType(17), "source tag publication");
  std::unique_ptr<ModuleData> generated(CandidateCreator::friend_newModuleData(nullptr));
  require(generated->getModuleTagNameKey() == NAMEKEY_INVALID && CandidateData::live == 1,
    "shared creator returns initialized ordinary owner");
}
void parsing(bool base) {
  Context context;
  context.load(base ? "base.ini" : "candidate.ini");
  require(context.accepted && context.accepted->getModuleTagNameKey() == NAMEKEY_INVALID,
    "actual creator completes parsing before publication");
  if (!base) {
    auto* value = dynamic_cast<CandidateData*>(context.accepted.get());
    require(value && value->count == 19 && value->label == "generated-owned-module-label-with-backing",
      "shared macro executes complete actual INI fields");
  }
  const auto* accepted = context.accepted.get();
  for (int repeat = 0; repeat < 3; ++repeat) {
    const auto live = AllocationFault::live();
    const auto candidates = CandidateData::live;
    bool rejected = false;
    try { context.load(base ? "base-bad.ini" : "bad.ini"); }
    catch (ErrorCode) { rejected = true; }
    catch (const INIException&) { rejected = true; }
    require(rejected && context.accepted.get() == accepted &&
      CandidateData::live == candidates && AllocationFault::live() == live,
      "repeated late rejection preserves accepted owner and retires candidate");
  }
  context.load(base ? "base.ini" : "candidate.ini");
}
void faults(bool base) {
  Context context;
  const char* filename = base ? "base.ini" : "candidate.ini";
  context.load(filename); // initialize source pools before exact baselines
  context.accepted.reset();
  AllocationFault::arm(std::numeric_limits<std::size_t>::max());
  context.load(filename);
  const auto census = AllocationFault::attempts();
  AllocationFault::disarm();
  context.accepted.reset();
  require(census > 0 && census < 256, "bounded complete module-data census");
  for (std::size_t ordinal = 0; ordinal <= census; ++ordinal) {
    context.load(filename);
    const auto* accepted = context.accepted.get();
    const auto live = AllocationFault::live();
    const auto candidates = CandidateData::live;
    AllocationFault::arm(ordinal);
    bool rejected = false;
    try { context.load(filename); }
    catch (const std::bad_alloc&) { rejected = true; }
    catch (...) { AllocationFault::disarm(); throw; }
    AllocationFault::disarm();
    if (ordinal < census) {
      require(rejected && AllocationFault::triggered() && context.accepted.get() == accepted &&
        CandidateData::live == candidates && AllocationFault::live() == live,
        "every acquisition rejects with exact candidate rollback");
      context.load(filename); // same source owner retry
    } else {
      require(!rejected && !AllocationFault::triggered() && AllocationFault::attempts() == census,
        "exact independent terminal proof");
    }
    context.accepted.reset();
  }
  std::cout << (base ? "base" : "macro") << " module-data ordinals [0," << census
    << ") complete; terminal " << census << '\n';
}
void decoratedKeys(bool fault) {
  NameKeyGenerator names;
  names.init();
  const AsciiString name(std::string(600, 'N').c_str());
  auto acquire = [&] {
    NameKeyTransaction keys(names);
    const auto key = nativeModuleNameKey(names, name, MODULETYPE_CLIENT_UPDATE);
    require(key == 1, "source decorated acquisition ordinal");
    keys.commit();
  };
  acquire(); // warm process pools
  names.reset();
  auto* pool = TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");
  require(pool, "actual module-key pool");
  if (!fault) {
    const auto live = AllocationFault::live();
    const auto used = pool->getUsedBlockCount();
    for (Int raw : {std::numeric_limits<Int>::min(), Int(-1), Int(NUM_MODULE_TYPES),
                    std::numeric_limits<Int>::max()}) {
      bool rejected = false;
      try { (void)nativeModuleNameKey(names, name, raw); }
      catch (ErrorCode) { rejected = true; }
      require(rejected && names.keyToName(NameKeyType(1)).isEmpty() &&
        AllocationFault::live() == live && pool->getUsedBlockCount() == used,
        "defined raw invalid buckets rejected before any key acquisition");
    }
    for (Int bucket = 0; bucket < NUM_MODULE_TYPES; ++bucket) {
      const auto key = nativeModuleNameKey(names, name, bucket);
      std::string expected(1, char('0' + bucket)); expected += name.str();
      require(names.keyToName(key) == expected.c_str(), "unchanged bucket prefix and long source name");
    }
    return;
  }
  AllocationFault::arm(std::numeric_limits<std::size_t>::max());
  acquire();
  const auto census = AllocationFault::attempts();
  AllocationFault::disarm();
  names.reset();
  require(census > 0 && census < 64, "complete bounded decorated-key manifest");
  for (std::size_t ordinal = 0; ordinal <= census; ++ordinal) {
    const auto live = AllocationFault::live();
    const auto used = pool->getUsedBlockCount();
    AllocationFault::arm(ordinal);
    bool rejected = false;
    try { acquire(); }
    catch (const std::bad_alloc&) { rejected = true; }
    catch (...) { AllocationFault::disarm(); throw; }
    AllocationFault::disarm();
    if (ordinal < census) {
      require(rejected && AllocationFault::triggered() && AllocationFault::live() == live &&
        pool->getUsedBlockCount() == used && names.keyToName(NameKeyType(1)).isEmpty(),
        "decorated key failed candidate restores exact namespace and allocations");
      acquire();
    } else {
      require(!rejected && !AllocationFault::triggered() && AllocationFault::attempts() == census,
        "decorated-key exact terminal boundary");
    }
    names.reset();
  }
  std::cout << "decorated-key ordinals [0," << census << ") complete; terminal " << census << '\n';
}
void dataOwner(bool fault) {
  NativeModuleDataList owner;
  std::unique_ptr<ModuleData> accepted(Module::friend_newModuleData(nullptr));
  accepted->setModuleTagNameKey(NameKeyType(17));
  owner.push_back(accepted.get());
  auto append = [&] {
    std::unique_ptr<ModuleData> value(Module::friend_newModuleData(nullptr));
    value->setModuleTagNameKey(NameKeyType(23));
    owner.push_back(value.get());
    value.release();
  };
  auto candidate = [&] {
    NativeModuleDataTransaction transaction(owner);
    append(); append(); append();
    // This is a fully commit-ready graph, abandoned by the enclosing definition.
    require(owner.size()==4 && owner.front()==accepted.get(), "complete candidate graph before rollback");
  };
  const auto* backing=owner.data();
  const auto capacity=owner.capacity();
  auto unchanged = [&] {
    require(owner.size()==1 && owner.front()==accepted.get() && owner.data()==backing &&
      owner.capacity()==capacity && accepted->getModuleTagNameKey()==NameKeyType(17),
      "definition rollback restores exact accepted owner, vector backing and tags");
  };
  if (!fault) {
    const auto live=AllocationFault::live();
    candidate(); unchanged();
    require(AllocationFault::live()==live, "abandoned complete data graph exact residuals");
    {
      NativeModuleDataTransaction outer(owner);
      append();
      const auto* outerBacking=owner.data();
      const auto outerLive=AllocationFault::live();
      {
        NativeModuleDataTransaction inner(owner);
        append(); append();
      }
      require(owner.size()==2 && owner.data()==outerBacking && AllocationFault::live()==outerLive,
        "inner rejection preserves outer-acquired data and backing");
      {
        NativeModuleDataTransaction inner(owner);
        append(); inner.commit();
      }
      require(owner.size()==3, "inner success publishes complete suffix");
    }
    unchanged(); require(AllocationFault::live()==live, "outer rejection owns committed inner units");
    {
      NativeModuleDataTransaction transaction(owner);
      append(); transaction.commit();
    }
    require(owner.size()==2 && owner.front()==accepted.get(), "accepted definition data commit");
    delete owner.back(); owner.pop_back();
    return;
  }
  AllocationFault::arm(std::numeric_limits<std::size_t>::max());
  candidate();
  const auto census=AllocationFault::attempts();
  AllocationFault::disarm(); unchanged();
  require(census>0 && census<64, "independent complete definition-data census");
  for (std::size_t ordinal=0; ordinal<=census; ++ordinal) {
    const auto live=AllocationFault::live();
    AllocationFault::arm(ordinal); bool rejected=false;
    try { candidate(); }
    catch(const std::bad_alloc&) { rejected=true; }
    catch(...) { AllocationFault::disarm(); throw; }
    AllocationFault::disarm(); unchanged();
    require(AllocationFault::live()==live, "all data-owner rejection paths restore exact allocations");
    if (ordinal<census) {
      require(rejected && AllocationFault::triggered(), "each data-owner ordinal rejects");
      {
        NativeModuleDataTransaction outer(owner);
        NativeModuleDataTransaction retry(owner);
        append(); append(); append(); retry.commit();
        require(owner.size()==4, "same-owner retry reaches accepted inner commit");
      }
      unchanged(); require(AllocationFault::live()==live, "retry and enclosing rejection retire all suffix units");
    } else require(!rejected && !AllocationFault::triggered() && AllocationFault::attempts()==census,
      "exact data-owner terminal boundary");
  }
  std::cout<<"definition data ordinals [0,"<<census<<") complete; terminal "<<census<<'\n';
}
}
int main(int argc, char** argv) {
  bool initialized = false;
  try {
    require(argc == 2, "module-data family");
    initMemoryManager(); initialized = true;
    const std::string family = argv[1];
    for (int repeat = 0; repeat < 3; ++repeat) {
      if (family == "values") values();
      else if (family == "base") parsing(true);
      else if (family == "macro") parsing(false);
      else if (family == "fault-base") faults(true);
      else if (family == "fault-macro") faults(false);
      else if (family == "keys") decoratedKeys(false);
      else if (family == "fault-keys") decoratedKeys(true);
      else if (family == "owner-values") dataOwner(false);
      else if (family == "owner-faults") dataOwner(true);
      else throw std::runtime_error("unknown module-data family");
      require(CandidateData::live == 0, "complete creator lifecycle retirement");
    }
    shutdownMemoryManager(); initialized = false;
    std::cout << "PASS: base/shared module-data creator support (whole factory acceptance pending)\n";
    return 0;
  } catch (ErrorCode error) { std::cerr << "FAIL: source error " << unsigned(error) << '\n'; }
  catch (const std::exception& error) { std::cerr << "FAIL: " << error.what() << '\n'; }
  AllocationFault::disarm(); publication = nullptr; TheFileSystem = nullptr;
  if (initialized) shutdownMemoryManager();
  return 1;
}
