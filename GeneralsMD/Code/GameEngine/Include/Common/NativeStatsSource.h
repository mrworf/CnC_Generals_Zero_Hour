// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/AsciiString.h"
#include <ctime>
struct NativeStatsIdentity {
  AsciiString map,side,directory="Stats",sessionName;
  Int benchmarkSeconds=0;
};
struct NativeStatsSample {
  UnsignedInt playerUnits=0,aiUnits=0;
  Int money=0;
};
// Borrowed synchronous inputs. The production adapter reads the actual original
// GameLogic/Player owners; generated logging tests do not replace those owners.
class NativeStatsSource {
public:
  virtual ~NativeStatsSource()=default;
  virtual UnsignedInt frame() const=0;
  virtual Int intervalSeconds() const=0;
  virtual Int localPlayerIndex() const=0;
  virtual NativeStatsIdentity identity() const=0;
  virtual NativeStatsSample sample() const=0;
  virtual std::time_t timestamp() const {return std::time(nullptr);}
};
