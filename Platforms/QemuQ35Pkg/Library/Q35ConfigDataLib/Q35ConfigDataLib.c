/* @file Q35ConfigDataLib.c

  Library to supply platform data to OemConfigPolicyCreatorPei.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#define CONFIG_INCLUDE_CACHE
#include <Generated/ConfigClientGenerated.h>
#include <Generated/ConfigServiceGenerated.h>

UINTN gNumKnobs = 2;

// temp, will be data only autogen
VOID * GetKnobValue(KNOB Knob){
  return NULL;
};