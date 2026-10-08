// SPDX-License-Identifier: GPL-3.0-or-later
// Supporting original startup owners. Not GameLogic acceptance until
// registered.
#include "AllocationFault.h"
#include "Common/Dict.h"
#include "Common/NameKeyGenerator.h"
#include "GameLogic/FPUControl.h"
#include <array>
#include <cfenv>
#include <fpu_control.h>
#include <iostream>
#include <new>
#include <stdexcept>
namespace {
void require(bool value, const char *message) {
  if (!value)
    throw std::runtime_error(message);
}
void names() {
  static_assert(sizeof(NameKeyType) == sizeof(Int) &&
                alignof(NameKeyType) == alignof(Int));
  StaticNameKey cached("cached");
  alignas(NameKeyGenerator) std::array<std::byte, sizeof(NameKeyGenerator)>
      storage;
  for (int lifecycle = 0; lifecycle < 3; ++lifecycle) {
    auto *owner = new (storage.data()) NameKeyGenerator;
    TheNameKeyGenerator = owner;
    try {
      owner->init();
      require(owner->nameToKey("first") == 1 && cached.key() == 2,
              "source namespace ordinal order");
      const auto original = owner->getNamespaceGeneration();
      owner->reset();
      require(owner->getNamespaceGeneration() != original && cached.key() == 1,
              "static key reset generation");
      require(owner->nameToLowercaseKey("Mixed") == 2 &&
                  owner->nameToLowercaseKey("MIXED") == 2,
              "original case-independent key lookup");
      require(owner->nameToKey("Mixed") == 3 && owner->nameToKey("Mixed") == 3,
              "original distinct exact-case bucket semantics");
      require(owner->keyToName(NameKeyType(2)) == "Mixed",
              "source first admitted spelling retained");
      bool rejected = false;
      try {
        owner->nameToKey(nullptr);
      } catch (ErrorCode) {
        rejected = true;
      }
      require(rejected && owner->nameToKey("after-invalid") == 4,
              "null admission preserves ordinal");
    } catch (...) {
      owner->~NameKeyGenerator();
      TheNameKeyGenerator = nullptr;
      throw;
    }
    owner->~NameKeyGenerator();
    TheNameKeyGenerator = nullptr;
    require(cached.key() == NAMEKEY_INVALID,
            "static cache has no retired namespace");
  }
  // Same storage address with an intervening changed ordinal order: raw
  // address equality cannot establish namespace identity.
  auto *owner = new (storage.data()) NameKeyGenerator;
  TheNameKeyGenerator = owner;
  try {
    owner->init();
    require(cached.key() == 1, "fresh replacement namespace rebind");
  } catch (...) {
    owner->~NameKeyGenerator();
    TheNameKeyGenerator = nullptr;
    throw;
  }
  owner->~NameKeyGenerator();
  TheNameKeyGenerator = nullptr;
  NameKeyGenerator bounded(2), independent(2);
  bounded.init();
  independent.init();
  require(bounded.nameToKey("one") == 1 && bounded.nameToKey("two") == 2,
          "exact bounded namespace maximum");
  require(independent.nameToKey("other") == 1, "independent source namespace");
  for (bool lower : {false, true}) {
    bool rejected = false;
    try {
      if (lower)
        bounded.nameToLowercaseKey("three");
      else
        bounded.nameToKey("three");
    } catch (ErrorCode) {
      rejected = true;
    }
    require(rejected && bounded.keyToName(NameKeyType(2)) == "two",
            "capacity rejection preserves admitted bucket graph");
  }
  bounded.reset();
  require(bounded.nameToKey("retry") == 1 &&
              independent.keyToName(NameKeyType(1)) == "other",
          "bounded reset/retry preserves independent shared-pool owner");
}
void nameFaults(bool lower) {
  NameKeyGenerator owner;
  owner.init();
  TheNameKeyGenerator = &owner;
  const char *name = "generated-fallible-name-with-real-string-backing";
  auto acquire = [&] {
    return lower ? owner.nameToLowercaseKey(name) : owner.nameToKey(name);
  };
  try {
    (void)acquire();
    owner.reset();
    auto *pool = TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");
    require(pool, "source name-key pool reachable");
    for (std::size_t ordinal = 0; ordinal < 64; ++ordinal) {
      const auto standard = AllocationFault::live();
      const Int blocks = pool->getUsedBlockCount();
      AllocationFault::arm(ordinal);
      bool failed = false;
      NameKeyType result = NAMEKEY_INVALID;
      try {
        result = acquire();
      } catch (const std::bad_alloc &) {
        failed = true;
      } catch (...) {
        AllocationFault::disarm();
        throw;
      }
      AllocationFault::disarm();
      if (!failed) {
        require(!AllocationFault::triggered() && result == 1,
                "name-key terminal admission");
        require(ordinal > 0, "name-key census contains actual allocations");
        std::cout << "name-key fault ordinals [0," << ordinal
                  << ") complete; terminal " << ordinal << '\n';
        TheNameKeyGenerator = nullptr;
        return;
      }
      require(AllocationFault::live() == standard &&
                  pool->getUsedBlockCount() == blocks,
              "name-key exact rollback ownership");
      require(owner.keyToName(NameKeyType(1)).isEmpty(),
              "failed name key remains unpublished");
      require(acquire() == 1 && owner.keyToName(NameKeyType(1)) == name,
              "same namespace retry preserves next ordinal");
      owner.reset();
    }
    throw std::runtime_error("name-key fault manifest exceeded");
  } catch (...) {
    AllocationFault::disarm();
    TheNameKeyGenerator = nullptr;
    throw;
  }
}
void nameTransactions(bool faults,bool lower=false) {
  NameKeyGenerator owner(8),peer(8);owner.init();peer.init();TheNameKeyGenerator=&owner;
  const char* names[]={"transaction-long-first-name-with-owned-backing",
    "transaction-long-second-name-with-owned-backing","transaction-long-third-name-with-owned-backing"};
  auto acquire=[&] {for(const char* name:names) {
    if(lower)owner.nameToLowercaseKey(name);else owner.nameToKey(name);
  }};
  {NameKeyTransaction warm(owner);acquire();}
  auto* pool=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");
  require(pool,"real name bucket pool");
  try {
    require(owner.nameToKey("accepted")==1 && peer.nameToKey("independent")==1,"independent accepted owner units");
    if(faults) {
      auto action=[&] {NameKeyTransaction transaction(owner);acquire();};
      AllocationFault::arm(SIZE_MAX);action();const auto census=AllocationFault::attempts();AllocationFault::disarm();
      require(census>0 && census<32,"bounded complete scope census");
      for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
        const auto live=AllocationFault::live();const auto used=pool->getUsedBlockCount();
        AllocationFault::arm(ordinal);bool failed=false;
        try {action();}catch(const std::bad_alloc&){failed=true;}
        catch(...){AllocationFault::disarm();throw;}
        AllocationFault::disarm();
        require(failed==(ordinal<census) && AllocationFault::triggered()==failed && AllocationFault::live()==live &&
          pool->getUsedBlockCount()==used,"whole name scope exact failure/terminal/std/pool retirement");
        require(owner.keyToName(NameKeyType(1))=="accepted" && owner.keyToName(NameKeyType(2)).isEmpty() &&
          peer.keyToName(NameKeyType(1))=="independent","rollback preserves accepted and independent namespaces");
        if(failed)action();
      }
      require(owner.nameToKey("headroom")==2,"scope restores exact ordinal headroom");
      std::cout<<"name transaction "<<(lower?"folded":"exact")<<" ordinals [0,"<<census<<"); terminal "<<census<<'\n';
    } else {
      StaticNameKey cached("static-candidate");
      {NameKeyTransaction outer(owner);
        require(owner.nameToKey("outer")==2,"outer acquired ordinal");
        {NameKeyTransaction inner(owner);require(cached.key()==3,"inner lazy static acquisition");inner.commit();}
        AllocationFault::arm(0);
      }
      AllocationFault::disarm();require(!AllocationFault::triggered(),"rollback allocation-free");
      require(owner.keyToName(NameKeyType(2)).isEmpty() && owner.nameToKey("changed-order")==2 && cached.key()==3,
        "outer rejection retires inner success and invalidates lazy cache");
      require(owner.keyToName(cached.key())=="static-candidate","static key has correct surviving identity");
      {NameKeyTransaction outer(owner);
        require(owner.nameToKey("outer-kept")==4,"second outer acquired ordinal");
        {NameKeyTransaction inner(owner);require(owner.nameToLowercaseKey("Discard")==5,"folded nested candidate");}
        require(owner.keyToName(NameKeyType(4))=="outer-kept" && owner.keyToName(NameKeyType(5)).isEmpty(),
          "inner rollback preserves outer units");
        bool resetRejected=false,initRejected=false;
        try {owner.reset();}catch(ErrorCode error){resetRejected=error==ERROR_BAD_ARG;}
        try {owner.init();}catch(ErrorCode error){initRejected=error==ERROR_BAD_ARG;}
        require(resetRejected && initRejected,"active owner cannot reset/init");outer.commit();
      }
      require(owner.nameToKey("accepted-after-commit")==5 && peer.keyToName(NameKeyType(1))=="independent",
        "nested accepted order and peer preserved");
      NameKeyGenerator inactive;bool rejected=false;
      try {NameKeyTransaction invalid(inactive);}catch(ErrorCode error){rejected=error==ERROR_BAD_ARG;}
      require(rejected,"uninitialized scope rejected");
      NameKeyGenerator bounded(2);bounded.init();
      {NameKeyTransaction pending(bounded);
        require(bounded.nameToKey("maximum-first")==1 && bounded.nameToLowercaseKey("maximum-second")==2,
          "transaction admits exact namespace maximum");
        bool full=false;try {bounded.nameToKey("representable-third");}catch(ErrorCode error){full=error==ERROR_OUT_OF_MEMORY;}
        require(full,"maximum plus one rejects inside transaction");
      }
      require(bounded.nameToKey("retry-first")==1 && bounded.nameToKey("retry-second")==2,
        "rollback restores complete bounded ordinal capacity");
    }
  }catch(...){AllocationFault::disarm();TheNameKeyGenerator=nullptr;throw;}
  TheNameKeyGenerator=nullptr;
}
void floatingPoint() {
  const auto prior = std::fegetround();
  for (int mode : {FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO, FE_TONEAREST}) {
    require(std::fesetround(mode) == 0, "generated prior rounding setup");
    std::feraiseexcept(FE_OVERFLOW | FE_INVALID);
    setFPMode();
    require(std::fegetround() == FE_TONEAREST &&
                std::fetestexcept(FE_ALL_EXCEPT) == 0,
            "source reset and active nearest rounding");
    fpu_control_t control;
    _FPU_GETCW(control);
    require((control & _FPU_EXTENDED) == _FPU_SINGLE &&
                (control & _FPU_RC_ZERO) == _FPU_RC_NEAREST,
            "source x87 single precision and rounding");
    volatile float positive = 2.5f, negative = -1.5f;
    require(fast_float2long_round(positive) == 2 &&
                fast_float2long_round(negative) == -2,
            "native conversion follows active ties-to-even mode");
  }
  std::fesetround(prior);
}
NameKeyType key(Int n) { return static_cast<NameKeyType>(n); }
void dictionary() {
  Dict owner(8);
  owner.setUnicodeString(key(5), UnicodeString(L"wide \u03a9 \U0001f680"));
  owner.setReal(key(3), -1.25f);
  owner.setBool(key(1), true);
  owner.setAsciiString(key(4), AsciiString("source-string"));
  owner.setInt(key(2), -17);
  require(owner.getPairCount() == 5, "all dictionary value families admitted");
  for (Int n = 0; n < 5; ++n)
    require(owner.getNthKey(n) == key(n + 1) &&
                owner.getNthType(n) == static_cast<Dict::DataType>(n),
            "source sorted key/type enumeration");
  require(owner.getNthBool(0) && owner.getNthInt(1) == -17 &&
              owner.getNthReal(2) == -1.25f &&
              owner.getNthAsciiString(3) == "source-string" &&
              owner.getNthUnicodeString(4) ==
                  UnicodeString(L"wide \u03a9 \U0001f680"),
          "typed nth dictionary values");
  Bool exists = true;
  require(owner.getInt(key(1), &exists) == 0 && !exists,
          "wrong-type default and existence");
  require(owner.getAsciiString(key(6), &exists).isEmpty() && !exists,
          "missing typed default and existence");
  require(owner.getNthKey(-1) == NAMEKEY_INVALID &&
              owner.getNthType(5) == Dict::DICT_NONE &&
              owner.getNthUnicodeString(5).isEmpty(),
          "bounded nth access");
  Dict alias = owner;
  owner.setInt(key(4), 29);
  require(alias.getAsciiString(key(4)) == "source-string" &&
              owner.getInt(key(4)) == 29,
          "typed COW replacement preserves alias");
  owner.copyPairFrom(alias, key(4));
  owner.copyPairFrom(owner, key(5));
  require(owner.getAsciiString(key(4)) == "source-string" &&
              owner.getUnicodeString(key(5)) == alias.getUnicodeString(key(5)),
          "borrowed backing and self-copy lifetime");
  require(owner.remove(key(3)) && !owner.remove(key(3)) &&
              alias.getReal(key(3)) == -1.25f,
          "remove missing and independent alias");
  Dict empty;
  owner.copyPairFrom(empty, key(4));
  require(owner.getType(key(4)) == Dict::DICT_NONE,
          "copy absent removes prior pair");
  owner.setInt(key(NAMEKEY_MAX), 73);
  require(owner.getInt(key(NAMEKEY_MAX)) == 73,
          "full declared name-key maximum");
  for (Int invalid : {-1, 0, NAMEKEY_MAX + 1}) {
    const Int count = owner.getPairCount();
    bool rejected = false;
    try {
      owner.setInt(key(invalid), 1);
    } catch (ErrorCode) {
      rejected = true;
    }
    require(rejected && owner.getPairCount() == count,
            "invalid packed-name admission has no mutation");
  }
  for (Int reserve : {-1, Dict::MAX_LEN + 1}) {
    bool rejected = false;
    try {
      Dict bad(reserve);
    } catch (ErrorCode) {
      rejected = true;
    }
    require(rejected, "dictionary reserve bound");
  }
  std::vector<Dict> aliases(70000, alias);
  alias.clear();
  require(aliases.back().getInt(key(2)) == -17,
          "shared backing reference count exceeds legacy uint16");
  Dict full(Dict::MAX_LEN);
  for (Int n = 1; n <= Dict::MAX_LEN; ++n)
    full.setInt(key(n), n);
  require(full.getPairCount() == Dict::MAX_LEN &&
              full.getInt(key(Dict::MAX_LEN)) == Dict::MAX_LEN,
          "exact dictionary cardinality maximum");
  bool rejected = false;
  try {
    full.setInt(key(Dict::MAX_LEN + 1), 0);
  } catch (ErrorCode) {
    rejected = true;
  }
  require(rejected && full.getPairCount() == Dict::MAX_LEN,
          "dictionary maximum plus one preserves graph");
  full.setInt(key(Dict::MAX_LEN), -9);
  require(full.getInt(key(Dict::MAX_LEN)) == -9,
          "full dictionary existing-key replacement");
  require(full.remove(key(1)), "full dictionary removal");
  full.setBool(key(Dict::MAX_LEN + 1), true);
  require(full.getPairCount() == Dict::MAX_LEN, "cardinality same-owner retry");
}
void dictionaryFaults(const std::string &operation) {
  const AsciiString ascii("generated-original-dictionary-string");
  const UnicodeString wide(L"generated dictionary \u03a9 \U0001f680");
  Dict source;
  source.setAsciiString(key(1), ascii);
  source.setUnicodeString(key(3), wide);
  source.setInt(key(5), -17);
  for (std::size_t ordinal = 0; ordinal < 32; ++ordinal) {
    Dict owner = source;
    const bool unique = operation.rfind("unique-", 0) == 0;
    const std::string action = unique ? operation.substr(7) : operation;
    if (unique)
      owner.setInt(key(5),
                   -17); // Fully admitted independent graph before injection.
    auto mutate = [&] {
      if (action == "insert")
        owner.setBool(key(2), true);
      else if (action == "replace")
        owner.setUnicodeString(key(1), wide);
      else if (action == "remove")
        owner.remove(key(3));
      else if (action == "copy")
        owner.copyPairFrom(owner, key(3));
      else
        throw std::runtime_error("unknown dictionary fault operation");
    };
    auto verify = [&] {
      require(source.getPairCount() == 3 &&
                  source.getAsciiString(key(1)) == ascii &&
                  source.getUnicodeString(key(3)) == wide &&
                  source.getInt(key(5)) == -17,
              "dictionary borrowed source unchanged");
      if (action == "insert")
        require(owner.getPairCount() == 4 && owner.getBool(key(2)),
                "inserted dictionary candidate");
      else if (action == "replace")
        require(owner.getPairCount() == 3 &&
                    owner.getUnicodeString(key(1)) == wide,
                "replaced dictionary candidate");
      else if (action == "remove")
        require(owner.getPairCount() == 2 &&
                    owner.getType(key(3)) == Dict::DICT_NONE,
                "removed dictionary candidate");
      else
        require(owner.getPairCount() == 3 &&
                    owner.getUnicodeString(key(3)) == wide,
                "self-copied dictionary candidate");
    };
    const auto baseline = AllocationFault::live();
    AllocationFault::arm(ordinal);
    bool failed = false;
    try {
      mutate();
    } catch (const std::bad_alloc &) {
      failed = true;
    } catch (...) {
      AllocationFault::disarm();
      throw;
    }
    AllocationFault::disarm();
    if (!failed) {
      require(!AllocationFault::triggered() && ordinal > 0,
              "dictionary exact terminal proof");
      verify();
      std::cout << "dictionary " << operation << " fault ordinals [0,"
                << ordinal << ") complete; terminal " << ordinal << '\n';
      return;
    }
    require(AllocationFault::live() == baseline && owner.getPairCount() == 3 &&
                owner.getAsciiString(key(1)) == ascii &&
                owner.getUnicodeString(key(3)) == wide &&
                owner.getInt(key(5)) == -17,
            "dictionary immediate rollback graph");
    mutate();
    verify();
  }
  throw std::runtime_error("dictionary fault manifest exceeded");
}
} // namespace
int main(int argc, char **argv) {
  bool initialized = false;
  try {
    require(argc == 2, "original runtime family required");
    initMemoryManager();
    initialized = true;
    const std::string family = argv[1];
    if (family == "names")
      names();
    else if (family == "floating-point")
      floatingPoint();
    else if (family == "dictionary")
      dictionary();
    else if (family.rfind("dictionary-fault-", 0) == 0)
      for (int repeat = 0; repeat < 3; ++repeat)
        dictionaryFaults(family.substr(17));
    else if (family == "names-exact-fault" || family == "names-lower-fault")
      for (int repeat = 0; repeat < 3; ++repeat)
        nameFaults(family == "names-lower-fault");
    else if(family=="names-transactions" || family=="names-transaction-exact-fault" || family=="names-transaction-lower-fault")
      for(int repeat=0;repeat<3;++repeat)nameTransactions(family!="names-transactions",family=="names-transaction-lower-fault");
    else
      throw std::runtime_error("unknown original runtime family");
    shutdownMemoryManager();
    initialized = false;
    std::cout << "PASS: original startup owner " << family
              << " (GameLogic acceptance pending)\n";
    return 0;
  } catch (ErrorCode) {
    std::cerr << "FAIL: original startup ownership admission\n";
  } catch (const std::exception &error) {
    std::cerr << "FAIL: generated startup fixture: " << error.what() << '\n';
  }
  TheNameKeyGenerator = nullptr;
  if (initialized)
    shutdownMemoryManager();
  return 1;
}
