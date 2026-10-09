// SPDX-License-Identifier: GPL-3.0-or-later
// Link-only diagnostic: default GUI construction must still require real providers.
#include "Common/FunctionLexicon.h"
int main(){FunctionLexicon owner;owner.init();return 0;}
