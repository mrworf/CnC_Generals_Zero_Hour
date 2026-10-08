// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/STLTypedefs.h"
#include <array>
#include <cstdio>
#include <stdexcept>

namespace {
using Names = std::unordered_map<AsciiString, Int, rts::hash<AsciiString>,
                                 rts::equal_to<AsciiString>>;
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
void functional() {
    AsciiString a("independently-owned"), b("independently-owned");
    require(a.str() != b.str(), "independent string backing");
    require(rts::equal_to<AsciiString>{}(a,b) &&
            rts::hash<AsciiString>{}(a) == rts::hash<AsciiString>{}(b),
            "content equality/hash agreement");
    Names names;
    names.emplace(a, 17);
    require(names.find(b) != names.end() && names.at(b) == 17,
            "independent backing lookup");
    require(!names.emplace(b, 99).second && names.size() == 1,
            "duplicate content retains node");
    names.emplace(AsciiString("Independently-owned"), 29);
    require(names.size() == 2, "case-sensitive source keys");
    Int* accepted = &names.at(a);
    names.rehash(4096);
    require(accepted == &names.at(b) && *accepted == 17, "rehash reference stability");
    names.erase(b);
    require(names.find(a) == names.end(), "independent key erase");
    names.emplace(b, 31);
    require(names.at(a) == 31, "same-name reinsertion");
    std::array<char, 9> first{'b','o','r','r','o','w','e','d',0};
    auto second = first;
    std::unordered_map<const char*, Int, rts::hash<const char*>,
                       rts::equal_to<const char*>> speech;
    speech.emplace(first.data(), 7);
    require(speech.at(second.data()) == 7 &&
            rts::hash<const char*>{}(first.data()) ==
            rts::hash<const char*>{}(second.data()), "speech content hash");
    speech.clear(); // Borrowed names stay alive until after node withdrawal.
    int identity = 19;
    std::unordered_map<const int*, int> pointers;
    pointers.emplace(&identity, 1);
    require(pointers.at(&identity) == 1, "native-width pointer identity");
}
void ordered() {
    Names left, right;
    left.emplace(AsciiString("z"), 1); left.emplace(AsciiString("A"), 2);
    left.emplace(AsciiString("a"), 3);
    right.rehash(4096);
    right.emplace(AsciiString("a"), 3); right.emplace(AsciiString("z"), 1);
    right.emplace(AsciiString("A"), 2);
    const auto a = rts::keysInOrder(left), b = rts::keysInOrder(right);
    require(a == b && a.size() == 3 && a[0] == "A" && a[1] == "a" && a[2] == "z",
            "named order independent of buckets/insertion");
    std::unordered_map<UnsignedInt, Int> numeric;
    for (UnsignedInt key : {0xffffffffu, 9u, 0u, 0x80000000u}) numeric.emplace(key, 1);
    const auto keys = rts::keysInOrder(numeric);
    require(keys == std::vector<UnsignedInt>{0,9,0x80000000u,0xffffffffu},
            "unsigned relation key order");
    std::unordered_map<Int, Int> signedKeys;
    signedKeys.emplace(-1, 1); signedKeys.emplace(9, 1); signedKeys.emplace(0, 1);
    require(rts::keysInOrder(signedKeys) == std::vector<Int>{-1,0,9},
            "signed player-index order");
}
struct Pools {
    Int count = TheDynamicMemoryAllocator->getDmaMemoryPoolCount();
    std::array<Int, 64> used{};
    Pools() {
        require(count <= Int(used.size()), "DMA pool census capacity");
        for (Int i = 0; i < count; ++i)
            used[i] = TheDynamicMemoryAllocator->getNthDmaMemoryPool(i)->getUsedBlockCount();
    }
    bool unchanged() const {
        if (count != TheDynamicMemoryAllocator->getDmaMemoryPoolCount()) return false;
        for (Int i = 0; i < count; ++i)
            if (used[i] != TheDynamicMemoryAllocator->getNthDmaMemoryPool(i)->getUsedBlockCount())
                return false;
        return true;
    }
};
void faults(int operation) {
    AsciiString one("one"), two("two"), three("three");
    const std::size_t expected = operation == 1 ? 1 : operation == 2 ? 3 : 2;
    for (std::size_t ordinal = 0; ordinal <= expected; ++ordinal) {
        Names names;
        names.max_load_factor(1);
        names.rehash(2);
        names.emplace(one, 1); names.emplace(two, 2);
        require(names.bucket_count() == 2, "manifest two-bucket fixture");
        Int* first = &names.at(one);
        const auto live = AllocationFault::live();
        const Pools pools;
        bool rejected = false;
        AllocationFault::arm(ordinal);
        try {
            if (operation == 1) { const auto keys = rts::keysInOrder(names); (void)keys; }
            else if (operation == 2) { Names copy(names); (void)copy; }
            else names.emplace(three, 3);
        } catch (const std::bad_alloc&) { rejected = true; }
        catch (...) { AllocationFault::disarm(); throw; }
        AllocationFault::disarm();
        if (ordinal == expected) {
            require(!rejected && !AllocationFault::triggered(), "exact terminal boundary");
            return;
        }
        require(rejected && AllocationFault::triggered() &&
                AllocationFault::live() == live && pools.unchanged() && names.size() == 2 &&
                names.bucket_count() == 2 && first == &names.at(one) &&
                *first == 1 && names.at(two) == 2 && names.find(three) == names.end(),
                "immediate node/bucket/reference rollback");
        if (operation == 1) {
            const auto keys = rts::keysInOrder(names);
            require(keys.size() == 2 && keys[0] == one && keys[1] == two,
                    "same-owner canonical-order retry");
        } else if (operation == 2) {
            Names copy(names);
            require(copy.size() == 2 && copy.at(one) == 1 && copy.at(two) == 2 &&
                    &copy.at(one) != first, "same-owner copy retry");
            copy.at(one) = 9;
            require(*first == 1, "independent copied nodes");
        } else {
            names.emplace(three, 3);
            require(names.at(three) == 3 && &names.at(one) == first,
                    "same-owner growth retry");
        }
    }
    throw std::runtime_error("missing terminal");
}
}
int main(int argc, char** argv) {
    if (argc != 2) return 2;
    try {
        for (int repeat = 0; repeat < 3; ++repeat) {
            initMemoryManager();
            try {
                const std::string_view family(argv[1]);
                if (family == "functional") functional();
                else if (family == "ordered") ordered();
                else if (family == "fault-insert") faults(0);
                else if (family == "fault-order") faults(1);
                else if (family == "fault-copy") faults(2);
                else throw std::runtime_error("unknown family");
            } catch (...) { shutdownMemoryManager(); throw; }
            shutdownMemoryManager();
        }
        std::puts("original-container-contract: PASS");
        return 0;
    } catch (const std::exception& error) {
        AllocationFault::disarm();
        std::fprintf(stderr, "original-container-contract: FAIL: %s\n", error.what());
        return 1;
    }
}
