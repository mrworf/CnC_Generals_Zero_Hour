// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/GameCommon.h"
#include "Common/SparseMatchFinder.h"
#include "GameClient/ViewFilters.h"
#include "GameClient/Keyboard.h"
#include "GameLogic/GameModes.h"
#include "GameClient/WindowMessageData.h"
#include "Common/XferCRC.h"
#include "Common/XferLoad.h"
#include "Common/XferSave.h"
#include <array>
#include <cstdio>
#include <stdexcept>
#include <type_traits>

static_assert(std::is_abstract_v<Xfer> && std::is_abstract_v<XferBase>);
static_assert(std::is_base_of_v<Xfer, XferCRC> && !std::is_abstract_v<XferCRC>);
static_assert(std::is_base_of_v<Xfer, XferLoad> && !std::is_abstract_v<XferLoad>);
static_assert(std::is_base_of_v<Xfer, XferSave> && !std::is_abstract_v<XferSave>);

template<> const char* BitFlags<4>::s_bitNameList[] = {"ONE", "TWO", "THREE", "FOUR", nullptr};
static_assert(FT_NULL_FILTER == 0 && FT_VIEW_BW_FILTER == 1 && FT_MAX == 5);
static_assert(FM_NULL_MODE == 0 && FM_VIEW_DEFAULT == 13 && FM_VIEW_MB_PAN_ALPHA == 14);
static_assert(sizeof(FilterTypes) == 4 && sizeof(FilterModes) == 4);
static_assert(sizeof(KeyDefType) == 4 && KEY_NONE == 0 && KEY_LOST == 255);
static_assert(KEY_KPENTER == 0x9c && KEY_RCTRL == 0x9d && KEY_RALT == 0xb8);
static_assert(sizeof(KeyboardIO) == 8 && offsetof(KeyboardIO,key) == 0 &&
    offsetof(KeyboardIO,state) == 2 && offsetof(KeyboardIO,sequence) == 4);
namespace {
void require(bool yes, const char* message) {
    if (!yes) throw std::runtime_error(message);
}
using Bits = BitFlags<4>;
struct Match {
    Bits flags;
    Int getConditionsYesCount() const { return 1; }
    const Bits& getNthConditionsYes(Int) const { return flags; }
    AsciiString getDescription() const { return AsciiString("generated-match"); }
};
void matching() {
    for (Int mode : {GAME_SINGLE_PLAYER,GAME_LAN,GAME_SKIRMISH,GAME_REPLAY,GAME_SHELL,GAME_NONE})
        require(nativeGameModeSupported(mode), "retained native mode identity");
    for (Int mode : std::array<Int,4>{-1,GAME_INTERNET,7,2147483647})
        require(!nativeGameModeSupported(mode), "unsupported/raw native mode admission");
    require(Bits::getSingleBitFromName("tWo") == 1 &&
            Bits::getSingleBitFromName("unknown") == -1, "source case-insensitive bit names");
    std::vector<Match> values{{Bits(Bits::kInit,0,1)}, {Bits(Bits::kInit,0)},
                              {Bits(Bits::kInit,0)}, {Bits(Bits::kInit,2)}};
    SparseMatchFinder<Match,Bits> finder;
    const Bits one(Bits::kInit,0), two(Bits::kInit,0,1), three(Bits::kInit,2);
    require(finder.findBestInfo(values,one) == &values[1], "fewer extraneous bits, first tie wins");
    require(finder.findBestInfo(values,two) == &values[0], "maximum intersection");
    require(finder.findBestInfo(values,three) == &values[3], "independent match");
    AllocationFault::arm(0);
    auto* cached = finder.findBestInfo(values,one);
    AllocationFault::disarm();
    require(cached == &values[1] && !AllocationFault::triggered(), "accepted cached lookup allocates nothing");
    finder.clear(); // Withdraw borrowed identities before changing vector backing.
    values.reserve(4096);
    require(finder.findBestInfo(values,one) == &values[1], "explicit cache withdrawal and new backing");
    finder.clear();
}
void faults() {
    const Bits one(Bits::kInit,0), two(Bits::kInit,1);
    std::vector<Match> source{{one},{two}}, copied=source;
    SparseMatchFinder<Match,Bits> original;
    require(original.findBestInfo(source,one)==&source[0], "source cache warmed");
    AllocationFault::arm(0);
    SparseMatchFinder<Match,Bits> clone(original);
    AllocationFault::disarm();
    require(!AllocationFault::triggered(), "clone does not copy borrowed cache backing");
    require(clone.findBestInfo(copied,one)==&copied[0], "copy resolves against exact new vector owner");
    AllocationFault::arm(0); clone=clone; AllocationFault::disarm();
    require(!AllocationFault::triggered() && clone.findBestInfo(copied,one)==&copied[0], "self-copy preserves cache");
    AllocationFault::arm(0); clone=original; AllocationFault::disarm();
    require(!AllocationFault::triggered(), "assignment withdraws derived cache allocation-free");
    const auto live=AllocationFault::live();
    AllocationFault::arm(0); bool rejected=false;
    try { (void)clone.findBestInfo(copied,one); }
    catch(const std::bad_alloc&) { rejected=true; }
    catch(...) { AllocationFault::disarm(); throw; }
    AllocationFault::disarm();
    require(rejected && AllocationFault::triggered() && AllocationFault::live()==live, "copied cold-cache exact rollback");
    require(clone.findBestInfo(copied,one)==&copied[0], "same copied-owner retry");
    original.clear(); source.clear(); source.shrink_to_fit();
    require(clone.findBestInfo(copied,one)==&copied[0], "source retirement preserves clone cache");
}
void insertionFaults() {
    std::vector<Match> values{{Bits(Bits::kInit,0)}, {Bits(Bits::kInit,1)}};
    const Bits one(Bits::kInit,0), two(Bits::kInit,1);
    for (std::size_t ordinal = 0; ordinal <= 1; ++ordinal) {
        SparseMatchFinder<Match,Bits> finder;
        require(finder.findBestInfo(values,one) == &values[0], "accepted pre-fault entry");
        const auto live = AllocationFault::live();
        bool rejected = false;
        AllocationFault::arm(ordinal);
        try { require(finder.findBestInfo(values,two) == &values[1], "candidate selection"); }
        catch (const std::bad_alloc&) { rejected = true; }
        catch (...) { AllocationFault::disarm(); throw; }
        AllocationFault::disarm();
        if (ordinal == 1) {
            require(!rejected && !AllocationFault::triggered(), "exact one-node terminal");
        } else {
            require(rejected && AllocationFault::triggered() && AllocationFault::live() == live,
                    "failed cache insertion leaves no backing");
            AllocationFault::arm(0);
            const auto* prior = finder.findBestInfo(values,one);
            AllocationFault::disarm();
            require(prior == &values[0] && !AllocationFault::triggered(), "accepted cache remains intact");
            require(finder.findBestInfo(values,two) == &values[1], "same-owner corrected retry");
        }
        finder.clear();
    }
}
struct Node {
    explicit Node(Int number) : value(number) {}
    Int value;
    MAKE_DLINK(Node, items)
};
struct Owner {
    MAKE_DLINK_HEAD(Node, items)
};
Int drained = 0;
void observeDrain(Node* node) {
    require(node->dlink_prev_items() == nullptr && node->dlink_next_items() == nullptr,
            "callback receives withdrawn node");
    drained = drained * 10 + node->value;
}
void lists() {
    Int scalar = 17;
    const auto address = reinterpret_cast<WindowMsgData>(&scalar);
    require(reinterpret_cast<Int*>(address) == &scalar, "full native callback address");
    const WindowMsgData packed = (WindowMsgData(0xabcd)<<16) | 0x9876;
    require((packed>>16) == 0xabcd && (packed&0xffff) == 0x9876,
            "packed callback scalar fields are unchanged");
    Node a(1), b(2), c(3);
    Owner owner;
    auto empty = owner.iterate_items();
    empty.advance();
    require(empty.done() && empty.cur() == nullptr, "empty iteration/advance");
    owner.removeFrom_items(&a); // Absent removal is the original no-op.
    owner.prependTo_items(&a); owner.prependTo_items(&b); owner.prependTo_items(&c);
    owner.prependTo_items(&b); // Same-owner duplicate prepend preserves order.
    Int sequence = 0;
    for (auto it = owner.iterate_items(); !it.done(); it.advance()) sequence = sequence*10+it.cur()->value;
    require(sequence == 321 && b.dlink_prev_items() == &c && b.dlink_next_items() == &a,
            "native member-function pointer and bidirectional insertion order");
    owner.reverse_items();
    require(owner.getFirstItemIn_items() == &a && a.dlink_prev_items() == nullptr &&
            a.dlink_next_items() == &b && c.dlink_next_items() == nullptr, "reverse links");
    owner.removeFrom_items(&b);
    require(!owner.isInList_items(&b) && b.dlink_prev_items() == nullptr &&
            b.dlink_next_items() == nullptr && a.dlink_next_items() == &c &&
            c.dlink_prev_items() == &a, "middle removal repairs both neighbors");
    owner.prependTo_items(&b);
    drained = 0;
    AllocationFault::arm(0);
    owner.removeAll_items(observeDrain);
    AllocationFault::disarm();
    require(drained == 213 && owner.getFirstItemIn_items() == nullptr &&
            !AllocationFault::triggered(), "drain order/withdrawal is allocation-free");
    owner.reverse_items();
    require(!BOGUSPTR(&a) && BOGUSPTR(reinterpret_cast<Node*>(std::uintptr_t(1))),
            "native-width diagnostic examines identity without dereference");
}
}
int main(int argc, char** argv) {
    if (argc != 2) return 2;
    try {
        for (int repeat=0; repeat<3; ++repeat) {
            initMemoryManager();
            try {
                const std::string_view family(argv[1]);
                if (family == "matching") matching();
                else if (family == "faults") faults();
                else if (family == "insertion-faults") insertionFaults();
                else if (family == "lists") lists();
                else throw std::runtime_error("unknown family");
            } catch (...) { AllocationFault::disarm(); shutdownMemoryManager(); throw; }
            shutdownMemoryManager();
        }
        std::puts("original-graph-header-contract: PASS");
        return 0;
    } catch (const std::exception& error) { std::fprintf(stderr,"%s\n",error.what()); return 1; }
}
