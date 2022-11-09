/** @file

  Copyright (c) 2019, Linaro Limited. All rights reserved.
  Copyright (c) Microsoft Corporation.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Base.h>
#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <libfdt.h>

#include <Library/HobLib.h>
#include <Guid/DxeMemoryProtectionSettings.h> // MU_CHANGE
#include <Guid/MmMemoryProtectionSettings.h>  // MU_CHANGE

// Number of Virtual Memory Map Descriptors
#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS          5

RETURN_STATUS
EFIAPI
SbsaQemuLibConstructor (
  VOID
  )
{
  VOID          *DeviceTreeBase;
  INT32         Node, Prev;
  UINT64        NewBase, CurBase;
  UINT64        NewSize, CurSize;
  CONST CHAR8   *Type;
  INT32         Len;
  CONST UINT64  *RegProp;
  RETURN_STATUS PcdStatus;

  NewBase = 0;
  NewSize = 0;

  DeviceTreeBase = (VOID *)(UINTN)PcdGet64 (PcdDeviceTreeInitialBaseAddress);
  ASSERT (DeviceTreeBase != NULL);

  // Make sure we have a valid device tree blob
  ASSERT (fdt_check_header (DeviceTreeBase) == 0);

  // Look for the lowest memory node
  for (Prev = 0;; Prev = Node) {
    Node = fdt_next_node (DeviceTreeBase, Prev, NULL);
    if (Node < 0) {
      break;
    }

    // Check for memory node
    Type = fdt_getprop (DeviceTreeBase, Node, "device_type", &Len);
    if (Type && AsciiStrnCmp (Type, "memory", Len) == 0) {
      // Get the 'reg' property of this node. For now, we will assume
      // two 8 byte quantities for base and size, respectively.
      RegProp = fdt_getprop (DeviceTreeBase, Node, "reg", &Len);
      if (RegProp != 0 && Len == (2 * sizeof (UINT64))) {

        CurBase = fdt64_to_cpu (ReadUnaligned64 (RegProp));
        CurSize = fdt64_to_cpu (ReadUnaligned64 (RegProp + 1));

        DEBUG ((DEBUG_INFO, "%a: System RAM @ 0x%lx - 0x%lx\n",
          __FUNCTION__, CurBase, CurBase + CurSize - 1));

        if (NewBase > CurBase || NewBase == 0) {
          NewBase = CurBase;
          NewSize = CurSize;
        }
      } else {
        DEBUG ((DEBUG_ERROR, "%a: Failed to parse FDT memory node\n",
          __FUNCTION__));
      }
    }
  }

  // Make sure the start of DRAM matches our expectation
  ASSERT (FixedPcdGet64 (PcdSystemMemoryBase) == NewBase);
  // TODO: This is carved out by the BL31 during DT build up.
  PcdStatus = PcdSet64S (PcdSystemMemorySize, NewSize - PcdGet64 (PcdMmBufferSize));
  ASSERT_RETURN_ERROR (PcdStatus);

  return RETURN_SUCCESS;
}

#define DXE_MEMORY_PROTECTION_SETTINGS_OSDDEBUG                    \
          {                                                     \
            DXE_MEMORY_PROTECTION_SETTINGS_CURRENT_VERSION,     \
            TRUE,   /* Stack Guard On */                        \
            {                                                   \
              .Fields.UefiNullDetection               = 1,      \
              .Fields.DisableEndOfDxe                 = 0,      \
              .Fields.DisableReadyToBoot              = 0       \
            },                                                  \
            {                                                   \
              .Fields.UefiPageGuard                   = 1,      \
              .Fields.UefiPoolGuard                   = 1,      \
              .Fields.UefiFreedMemoryGuard            = 0,      \
              .Fields.Direction                       = 0       \
            },                                                  \
            {                                                   \
              .Fields.ProtectImageFromUnknown         = 1,      \
              .Fields.ProtectImageFromFv              = 1,      \
              .Fields.RaiseErrorIfProtectionFails     = 1,      \
              .Fields.BlockImagesWithoutNxFlag        = 0       \
            },                                                  \
            {                                                   \
              .Fields.EfiReservedMemoryType           = 1,      \
              .Fields.EfiLoaderCode                   = 1,      \
              .Fields.EfiLoaderData                   = 1,      \
              .Fields.EfiBootServicesCode             = 1,      \
              .Fields.EfiBootServicesData             = 1,      \
              .Fields.EfiRuntimeServicesCode          = 1,      \
              .Fields.EfiRuntimeServicesData          = 1,      \
              .Fields.EfiConventionalMemory           = 0,      \
              .Fields.EfiUnusableMemory               = 1,      \
              .Fields.EfiACPIReclaimMemory            = 1,      \
              .Fields.EfiACPIMemoryNVS                = 1,      \
              .Fields.EfiMemoryMappedIO               = 1,      \
              .Fields.EfiMemoryMappedIOPortSpace      = 1,      \
              .Fields.EfiPalCode                      = 1,      \
              .Fields.EfiPersistentMemory             = 0,      \
              .Fields.OEMReserved                     = 1,      \
              .Fields.OSReserved                      = 1       \
            },                                                  \
            {                                                   \
              .Fields.EfiReservedMemoryType           = 1,      \
              .Fields.EfiLoaderCode                   = 1,      \
              .Fields.EfiLoaderData                   = 1,      \
              .Fields.EfiBootServicesCode             = 1,      \
              .Fields.EfiBootServicesData             = 1,      \
              .Fields.EfiRuntimeServicesCode          = 1,      \
              .Fields.EfiRuntimeServicesData          = 1,      \
              .Fields.EfiConventionalMemory           = 0,      \
              .Fields.EfiUnusableMemory               = 1,      \
              .Fields.EfiACPIReclaimMemory            = 1,      \
              .Fields.EfiACPIMemoryNVS                = 1,      \
              .Fields.EfiMemoryMappedIO               = 1,      \
              .Fields.EfiMemoryMappedIOPortSpace      = 1,      \
              .Fields.EfiPalCode                      = 1,      \
              .Fields.EfiPersistentMemory             = 0,      \
              .Fields.OEMReserved                     = 1,      \
              .Fields.OSReserved                      = 1       \
            },                                                  \
            {                                                   \
              .Fields.EfiReservedMemoryType           = 1,      \
              .Fields.EfiLoaderCode                   = 1,      \
              .Fields.EfiLoaderData                   = 1,      \
              .Fields.EfiBootServicesCode             = 1,      \
              .Fields.EfiBootServicesData             = 1,      \
              .Fields.EfiRuntimeServicesCode          = 1,      \
              .Fields.EfiRuntimeServicesData          = 1,      \
              .Fields.EfiConventionalMemory           = 1,      \
              .Fields.EfiUnusableMemory               = 1,      \
              .Fields.EfiACPIReclaimMemory            = 1,      \
              .Fields.EfiACPIMemoryNVS                = 1,      \
              .Fields.EfiMemoryMappedIO               = 1,      \
              .Fields.EfiMemoryMappedIOPortSpace      = 1,      \
              .Fields.EfiPalCode                      = 1,      \
              .Fields.EfiPersistentMemory             = 0,      \
              .Fields.OEMReserved                     = 1,      \
              .Fields.OSReserved                      = 1       \
            }                                                   \
          }

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU
  on your platform.

  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR
                                    describing a Physical-to-Virtual Memory
                                    mapping. This array must be ended by a
                                    zero-filled entry. The allocated memory
                                    will not be freed.

**/
VOID
ArmPlatformGetVirtualMemoryMap (
  OUT ARM_MEMORY_REGION_DESCRIPTOR   **VirtualMemoryMap
  )
{
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;
  DXE_MEMORY_PROTECTION_SETTINGS  DxeSettings;
  MM_MEMORY_PROTECTION_SETTINGS   MmSettings;

  DxeSettings = (DXE_MEMORY_PROTECTION_SETTINGS)DXE_MEMORY_PROTECTION_SETTINGS_OSDDEBUG;
  MmSettings  = (MM_MEMORY_PROTECTION_SETTINGS)MM_MEMORY_PROTECTION_SETTINGS_DEBUG;

  MmSettings.HeapGuardPolicy.Fields.MmPageGuard                    = 1;
  MmSettings.HeapGuardPolicy.Fields.MmPoolGuard                    = 1;
  //DxeSettings.ImageProtectionPolicy.Fields.ProtectImageFromUnknown = 1;
  // THE /NXCOMPAT DLL flag cannot be set using non MinGW GCC
 #ifdef __GNUC__
  //DxeSettings.ImageProtectionPolicy.Fields.BlockImagesWithoutNxFlag = 0;
 #endif

  DEBUG ((DEBUG_ERROR, "%a: OSDDEBUG GetVirtualMemoryMap\n",
          __FUNCTION__));

  BuildGuidDataHob (
    &gDxeMemoryProtectionSettingsGuid,
    &DxeSettings,
    sizeof (DxeSettings)
    );

  BuildGuidDataHob (
    &gMmMemoryProtectionSettingsGuid,
    &MmSettings,
    sizeof (MmSettings)
    );

  ASSERT (VirtualMemoryMap != NULL);

  VirtualMemoryTable = AllocatePool (sizeof (ARM_MEMORY_REGION_DESCRIPTOR) *
                                     MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  if (VirtualMemoryTable == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Error: Failed AllocatePool()\n", __FUNCTION__));
    return;
  }

  // System DRAM
  VirtualMemoryTable[0].PhysicalBase = PcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[0].VirtualBase  = VirtualMemoryTable[0].PhysicalBase;
  VirtualMemoryTable[0].Length       = PcdGet64 (PcdSystemMemorySize);
  VirtualMemoryTable[0].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  DEBUG ((DEBUG_INFO, "%a: Dumping System DRAM Memory Map:\n"
          "\tPhysicalBase: 0x%lX\n"
          "\tVirtualBase: 0x%lX\n"
          "\tLength: 0x%lX\n",
          __FUNCTION__,
          VirtualMemoryTable[0].PhysicalBase,
          VirtualMemoryTable[0].VirtualBase,
          VirtualMemoryTable[0].Length));

  // Peripheral space before DRAM
  VirtualMemoryTable[1].PhysicalBase = 0x0;
  VirtualMemoryTable[1].VirtualBase  = 0x0;
  VirtualMemoryTable[1].Length       = VirtualMemoryTable[0].PhysicalBase;
  VirtualMemoryTable[1].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // Remap the FD region as normal executable memory
  VirtualMemoryTable[2].PhysicalBase = PcdGet64 (PcdFdBaseAddress);
  VirtualMemoryTable[2].VirtualBase  = VirtualMemoryTable[2].PhysicalBase;
  VirtualMemoryTable[2].Length       = FixedPcdGet32 (PcdFdSize);
  VirtualMemoryTable[2].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  // MM Memory Space
  VirtualMemoryTable[3].PhysicalBase = PcdGet64 (PcdMmBufferBase);
  VirtualMemoryTable[3].VirtualBase    = PcdGet64 (PcdMmBufferBase);
  VirtualMemoryTable[3].Length         = PcdGet64 (PcdMmBufferSize);
  VirtualMemoryTable[3].Attributes     = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // End of Table
  ZeroMem (&VirtualMemoryTable[4], sizeof (ARM_MEMORY_REGION_DESCRIPTOR));

  *VirtualMemoryMap = VirtualMemoryTable;
}
