#include "PreRTS.h"
#include "Common/SubsystemInterface.h"
#include "zh/original_process.h"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::vector<std::string> events;
int live_subsystems = 0;
int fail_loader = 0;

int fail(const char* message)
{
  std::fprintf(stderr, "M20 lifecycle failure: %s\n", message);
  return 1;
}

class TestSubsystem final : public SubsystemInterface {
 public:
  TestSubsystem(const char* label, bool fail_init = false, bool fail_post = false,
      bool fail_reset = false)
      : label_(label), fail_init_(fail_init), fail_post_(fail_post), fail_reset_(fail_reset)
  {
    ++live_subsystems;
    events.emplace_back(std::string("construct:") + label_);
  }

  ~TestSubsystem() override
  {
    events.emplace_back(std::string("destroy:") + label_);
    --live_subsystems;
  }

  void init() override
  {
    events.emplace_back(std::string("init:") + label_);
    if (fail_init_) throw ERROR_BAD_ARG;
  }

  void postProcessLoad() override
  {
    events.emplace_back(std::string("post:") + label_);
    if (fail_post_) throw ERROR_BAD_INI;
  }

  void reset() override
  {
    events.emplace_back(std::string("reset:") + label_);
    if (fail_reset_) throw ERROR_BUG;
  }

  void update() override { events.emplace_back(std::string("update:") + label_); }

 private:
  const char* label_;
  bool fail_init_;
  bool fail_post_;
  bool fail_reset_;
};

void injected_loader(const char*, const char*, const char*, Xfer*)
{
  events.emplace_back("data");
  if (fail_loader) throw ERROR_BAD_INI;
}

bool suffix(std::initializer_list<const char*> expected)
{
  if (events.size() < expected.size()) return false;
  auto at = events.end() - static_cast<std::ptrdiff_t>(expected.size());
  for (const char* value : expected)
    if (*at++ != value) return false;
  return true;
}

bool released()
{
  const auto services = zh::original_process::service_counts();
  return live_subsystems == 0 && services.workers == 0;
}

}  // namespace

int main()
{
  char diagnostic[96]{};
  if (!zh::original_process::initialize_services(0, diagnostic, sizeof(diagnostic)))
    return fail(diagnostic);

  {
    SubsystemInterfaceList list;
    TheSubsystemList = &list;
    list.initSubsystem(new TestSubsystem("one"), nullptr, nullptr, nullptr, nullptr, "one");
    list.initSubsystem(new TestSubsystem("two"), nullptr, nullptr, nullptr, nullptr, "two");
    list.postProcessLoadAll();
    list.resetAll();
    list.shutdownAll();
    if (!suffix({"post:one", "post:two", "reset:two", "reset:one", "destroy:two", "destroy:one"}))
      return fail("normal lifecycle did not preserve source order and reverse ownership");
    if (!released()) return fail("normal lifecycle retained subsystem ownership");
    TheSubsystemList = nullptr;
  }

  {
    events.clear();
    SubsystemInterfaceList list;
    TheSubsystemList = &list;
    bool rejected = false;
    try { list.initSubsystem(new TestSubsystem("init", true), nullptr, nullptr, nullptr, nullptr, "init"); }
    catch (ErrorCode error) { rejected = error == ERROR_BAD_ARG; }
    if (!rejected || !suffix({"construct:init", "init:init", "destroy:init"}) || !released())
      return fail("pending init failure was not released exactly once");
    TheSubsystemList = nullptr;
  }

  {
    events.clear();
    SubsystemInterfaceList list;
    TheSubsystemList = &list;
    bool rejected = false;
    try { list.initSubsystem(new TestSubsystem("missing-loader"), "owned.ini", nullptr, nullptr, nullptr, "data"); }
    catch (ErrorCode error) { rejected = error == ERROR_BAD_INI; }
    if (!rejected || !suffix({"construct:missing-loader", "init:missing-loader", "destroy:missing-loader"}) || !released())
      return fail("missing production data provider did not fail closed");

    installSubsystemINIDataLoader(injected_loader);
    fail_loader = 1;
    rejected = false;
    try { list.initSubsystem(new TestSubsystem("data"), "owned.ini", nullptr, nullptr, nullptr, "data"); }
    catch (ErrorCode error) { rejected = error == ERROR_BAD_INI; }
    if (!rejected || !suffix({"construct:data", "init:data", "data", "destroy:data"}) || !released())
      return fail("data-load failure did not release pending subsystem");
    fail_loader = 0;
    installSubsystemINIDataLoader(nullptr);
    TheSubsystemList = nullptr;
  }

  {
    events.clear();
    SubsystemInterfaceList list;
    TheSubsystemList = &list;
    list.initSubsystem(new TestSubsystem("one"), nullptr, nullptr, nullptr, nullptr, "one");
    list.initSubsystem(new TestSubsystem("two", false, true), nullptr, nullptr, nullptr, nullptr, "two");
    bool rejected = false;
    try { list.postProcessLoadAll(); }
    catch (ErrorCode error) { rejected = error == ERROR_BAD_INI; }
    if (!rejected || !suffix({"post:one", "post:two", "destroy:two", "destroy:one"}) || !released())
      return fail("post-load failure did not unwind registered subsystems in reverse");
    TheSubsystemList = nullptr;
  }

  {
    events.clear();
    SubsystemInterfaceList list;
    TheSubsystemList = &list;
    list.initSubsystem(new TestSubsystem("one"), nullptr, nullptr, nullptr, nullptr, "one");
    list.initSubsystem(new TestSubsystem("two", false, false, true), nullptr, nullptr, nullptr, nullptr, "two");
    bool rejected = false;
    try { list.resetAll(); }
    catch (ErrorCode error) { rejected = error == ERROR_BUG; }
    if (!rejected || !suffix({"reset:two", "destroy:two", "destroy:one"}) || !released())
      return fail("reset failure did not unwind registered subsystems in reverse");
    list.shutdownAll();
    TheSubsystemList = nullptr;
  }

  zh::original_process::shutdown_services();
  if (TheSubsystemList || !released()) return fail("process shutdown retained lifecycle state");
  std::printf("original-lifecycle subsystem: ok provider=SubsystemInterface.cpp live=0 workers=0\n");
  return 0;
}
