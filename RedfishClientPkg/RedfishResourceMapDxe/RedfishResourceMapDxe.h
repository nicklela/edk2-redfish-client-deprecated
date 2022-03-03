/** @file
  Common header file for RedfishResourceMapDxe driver.

  (C) Copyright 2022 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef REDFISH_RESOURCE_MAP_DXE_H_
#define REDFISH_RESOURCE_MAP_DXE_H_

#include <Uefi.h>
#include <RedfishBase.h>

//
// Libraries
//
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/EdkIIRedfishResourceMapProtocol.h>

#include <Guid/VariableFormat.h>

#define RESOURCE_MAP_VARIABLE_NAME    L"RedfishResourceMap"
#define RESOURCE_MAP_DEBUG_ENABLED    0x01

//
// Definition of REDFISH_RESOURCE_MAP_RECORD
//
typedef struct {
  LIST_ENTRY  List;
  CHAR8       *Uri;
  CHAR8       *ConfigLang;
  UINTN       Size;
} REDFISH_RESOURCE_MAP_RECORD;

#define REDFISH_RESOURCE_MAP_RECORD_FROM_LIST(a)  BASE_CR (a, REDFISH_RESOURCE_MAP_RECORD, List)

//
// Definition of REDFISH_RESOURCE_MAP_LIST
//
typedef struct {
  LIST_ENTRY    Listheader;
  UINTN         TotalSize;
  UINTN         Count;
} REDFISH_RESOURCE_MAP_LIST;

//
// Definition of REDFISH_RESOURCE_MAP_PRIVATE_DATA
//
typedef struct {
  EFI_HANDLE                          ImageHandle;
  REDFISH_RESOURCE_MAP_LIST           ResourceList;
  EDKII_REDFISH_RESOURCE_MAP_PROTOCOL Protocol;
  EFI_STRING                          VariableName;
  EFI_EVENT                           Event;
} REDFISH_RESOURCE_MAP_PRIVATE_DATA;

#define REDFISH_RESOURCE_MAP_PRIVATE_FROM_THIS(a)  BASE_CR (a, REDFISH_RESOURCE_MAP_PRIVATE_DATA, Protocol)

#endif
