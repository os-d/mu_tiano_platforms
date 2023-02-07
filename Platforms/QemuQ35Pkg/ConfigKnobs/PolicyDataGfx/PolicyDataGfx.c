/** @file
  Consumer module to locate conf data from variable storage, initialize
  the GFX policy data and override the policy based on configuration values.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <PolicyDataStructGFX.h>
#include <Protocol/Policy.h>

#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/Policy.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>

// XML autogen definitions
#include <Generated/ConfigClientGenerated.h>

// Statically define policy initialization for 2 GFX ports
GFX_POLICY_DATA  DefaultQemuGfxPolicy[GFX_PORT_MAX_CNT] = {
  {
    .Power_State_Port = TRUE
  },
  {
    .Power_State_Port = TRUE
  }
};

/**
  Publish default silicon policy for GFX to policy database.

  @param FileHandle                     The image handle.
  @param PeiServices                    The PEI services table.

  @retval Status                        From internal routine or boot object, should not fail
**/
EFI_STATUS
EFIAPI
PublishDefaultPolicyGfx (
  IN POLICY_PPI   *PolPpi
  )
{
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "%a - Entry.\n", __FUNCTION__));

  // First check policy ppi.
  if (PolPpi == NULL) {
    DEBUG ((DEBUG_ERROR, "%a Policy PPI is NULL!\n", __FUNCTION__));
    ASSERT (FALSE);
    return EFI_INVALID_PARAMETER;
  }

  // Publish GFX policy
  Status = PolPpi->SetPolicy (&gPolicyDataGFXGuid, 0, DefaultQemuGfxPolicy, sizeof (DefaultQemuGfxPolicy));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to set GFX policy - %r\n", __FUNCTION__, Status));
  }

  return Status;
}
