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

#include "ConfigKnobs.h"

// temp test of autogen change
typedef struct {
  UINTN       NameSize;
  UINTN       DataSize;
  EFI_GUID    NamespaceGuid;
} CONF_POLICY_ENTRY;

VOID *CachedPolicy = NULL;
BOOLEAN CachedPolicyInitialized = FALSE;
UINT16 CachedPolicySize = 0x38;

STATIC
BOOLEAN
TempGetPowerOnPort0 (
  )
{
  POLICY_PPI  *PolPpi        = NULL;
  EFI_STATUS Status;
  // autogen'ed offset, past the first header and length of the name
  UINTN      Offset = sizeof(CONF_POLICY_ENTRY) + 26;

  if (!CachedPolicyInitialized) {
    // fetch policy
    Status = PeiServicesLocatePpi (
             &gPeiPolicyPpiGuid,
             0,
             NULL,
             (VOID **)&PolPpi
             );

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a failed to locate policy service!\n", __FUNCTION__));
      ASSERT (FALSE);
      return FALSE;
    }

    CachedPolicy = AllocatePool (CachedPolicySize);
    if (CachedPolicy == NULL) {
      DEBUG ((DEBUG_ERROR, "%a failed to allocate memory for cached policy\n", __FUNCTION__));
      ASSERT(FALSE);
      return FALSE;
    }

    Status = PolPpi->GetPolicy (&gOemConfigPolicyGuid, NULL, CachedPolicy, &CachedPolicySize);
    if (EFI_ERROR (Status)) {
      ASSERT (FALSE);
      return FALSE;
    }
    CachedPolicyInitialized = TRUE;
  }

  DEBUG ((DEBUG_ERROR, "%a cached policy size %x\n", __FUNCTION__, CachedPolicySize));

  DUMP_HEX(DEBUG_ERROR, 0, CachedPolicy, CachedPolicySize, "OSDDEBUG: ");

  return (BOOLEAN)(*((UINT8 *)CachedPolicy + Offset));
}

/**
  Module entry point that will check configuration data and publish them to policy database.

  @param FileHandle                     The image handle.
  @param PeiServices                    The PEI services table.

  @retval Status                        From internal routine or boot object, should not fail
**/
EFI_STATUS
EFIAPI
ConfigKnobsEntry (
  IN EFI_PEI_FILE_HANDLE     FileHandle,
  IN CONST EFI_PEI_SERVICES  **PeiServices
  )
{
  EFI_STATUS  Status;
  POLICY_PPI  *PolPpi = NULL;

  BOOLEAN   GfxEnablePort0;

  DEBUG ((DEBUG_INFO, "%a - Entry.\n", __FUNCTION__));

  // First locate policy ppi, should not fail as we populate the 
  Status = PeiServicesLocatePpi (&gPeiPolicyPpiGuid, 0, NULL, (VOID *)&PolPpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to locate Policy PPI - %r\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    goto Done;
  }

  Status = PublishDefaultPolicyGfx (PolPpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to locate Policy PPI - %r\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    goto Done;
  }

  GfxEnablePort0  = TempGetPowerOnPort0 ();
  Status          = ApplyGfxConfigToPolicy (PolPpi, &GfxEnablePort0);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to apply configuration data to the GFX silicon policy - %r\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    goto Done;
  }

Done:

  if (CachedPolicy != NULL) {
    FreePool (CachedPolicy);
  }
  return Status;
}
