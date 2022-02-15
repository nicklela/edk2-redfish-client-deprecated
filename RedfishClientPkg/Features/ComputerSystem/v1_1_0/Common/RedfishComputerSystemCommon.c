/** @file
  Redfish feature driver implementation - common functions

  (C) Copyright 2020-2021 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "RedfishComputerSystemCommon.h"

CHAR8 MemoryEmptyJson[] = "{\"@odata.id\": \"\", \"@odata.type\": \"#ComputerSystem.v1_1_0.ComputerSystem\", \"Id\": \"\", \"Name\": \"\"}";

REDFISH_RESOURCE_COMMON_PRIVATE *mRedfishResourcePrivate = NULL;

EFI_STATUS
RedfishConsumeResourceCommon (
  IN  REDFISH_RESOURCE_COMMON_PRIVATE *Private,
  IN  CHAR8                         *MemoryJson
  )
{
  EFI_STATUS                    Status;
  EFI_REDFISH_COMPUTERSYSTEM_V1_1_0     *ComputerSystem;
  EFI_REDFISH_COMPUTERSYSTEM_V1_1_0_CS  *ComputerSystemCs;
  EFI_STRING                    ConfigureLang;
  CHAR8                         *Arraykey;
  CHAR8                         *EtagInDb;

  if (Private == NULL || IS_EMPTY_STRING (MemoryJson)) {
    return EFI_INVALID_PARAMETER;
  }

  Memory= NULL;
  MemoryCs = NULL;
  ConfigureLang = NULL;
  Arraykey = NULL;
  EtagInDb = NULL;

  Status = Private->JsonStructProtocol->ToStructure (
                                          Private->JsonStructProtocol,
                                          NULL,
                                          MemoryJson,
                                          (EFI_REST_JSON_STRUCTURE_HEADER **)&Memory
                                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, ToStructure() failed: %r\n", __FUNCTION__, Status));
    return Status;
  }

  ComputerSystemCs = ComputerSystem->ComputerSystem;

  //
  // Check ETAG to see if we need to consume it
  //
  EtagInDb = GetEtagWithUri (Private->Uri);
  if (EtagInDb != NULL && MemoryCs->odata_etag != NULL) {
    if (AsciiStrCmp (EtagInDb, MemoryCs->odata_etag) == 0) {
      //
      // No change
      //
      DEBUG ((DEBUG_INFO, "%a, ETAG: [%a] no change, ignore consume action\n", __FUNCTION__, EtagInDb));
      goto ON_RELEASE;
    }
  }

  //
  // Find array key from URI
  //
  Status = GetArraykeyFromUri (Private->Uri, &Arraykey);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return Status;
  }

  //
  // Handle SYSTEMTYPE
  //
  if (ComputerSystemCs->SystemType != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "SystemType", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->SystemType);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle ASSETTAG
  //
  if (ComputerSystemCs->AssetTag != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "AssetTag", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->AssetTag);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle MANUFACTURER
  //
  if (ComputerSystemCs->Manufacturer != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "Manufacturer", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->Manufacturer);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle MODEL
  //
  if (ComputerSystemCs->Model != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "Model", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->Model);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle SKU
  //
  if (ComputerSystemCs->SKU != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "SKU", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->SKU);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle SERIALNUMBER
  //
  if (ComputerSystemCs->SerialNumber != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "SerialNumber", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->SerialNumber);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle PARTNUMBER
  //
  if (ComputerSystemCs->PartNumber != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "PartNumber", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->PartNumber);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle HOSTNAME
  //
  if (ComputerSystemCs->HostName != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "HostName", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->HostName);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle INDICATORLED
  //
  if (ComputerSystemCs->IndicatorLED != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "IndicatorLED", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->IndicatorLED);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle POWERSTATE
  //
  if (ComputerSystemCs->PowerState != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "PowerState", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->PowerState);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle BOOT->BOOTSOURCEOVERRIDETARGET
  //
  if (ComputerSystemCs->Boot->BootSourceOverrideTarget != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "Boot/BootSourceOverrideTarget", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->Boot->BootSourceOverrideTarget);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle BOOT->BOOTSOURCEOVERRIDEENABLED
  //
  if (ComputerSystemCs->Boot->BootSourceOverrideEnabled != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "Boot/BootSourceOverrideEnabled", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->Boot->BootSourceOverrideEnabled);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle BOOT->UEFITARGETBOOTSOURCEOVERRIDE
  //
  if (ComputerSystemCs->Boot->UefiTargetBootSourceOverride != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "Boot/UefiTargetBootSourceOverride", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->Boot->UefiTargetBootSourceOverride);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle BOOT->BOOTSOURCEOVERRIDEMODE
  //
  if (ComputerSystemCs->Boot->BootSourceOverrideMode != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "Boot/BootSourceOverrideMode", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->Boot->BootSourceOverrideMode);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle BIOSVERSION
  //
  if (ComputerSystemCs->BiosVersion != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "BiosVersion", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->BiosVersion);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle PROCESSORSUMMARY->COUNT
  //
  if (ComputerSystemCs->ProcessorSummary->Count != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "ProcessorSummary/Count", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsNumericType (RESOURCE_SCHEMA_FULL, ConfigureLang, (UINTN)*ComputerSystemCs->ProcessorSummary->Count);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle PROCESSORSUMMARY->MODEL
  //
  if (ComputerSystemCs->ProcessorSummary->Model != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "ProcessorSummary/Model", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->ProcessorSummary->Model);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle MEMORYSUMMARY->TOTALSYSTEMMEMORYGIB
  //
  if (ComputerSystemCs->MemorySummary->TotalSystemMemoryGiB != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "MemorySummary/TotalSystemMemoryGiB", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsNumericType (RESOURCE_SCHEMA_FULL, ConfigureLang, (UINTN)*ComputerSystemCs->MemorySummary->TotalSystemMemoryGiB);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }

  //
  // Handle MEMORYSUMMARY->MEMORYMIRRORING
  //
  if (ComputerSystemCs->MemorySummary->MemoryMirroring != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "MemorySummary/MemoryMirroring", Arraykey);
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettings (RESOURCE_SCHEMA_FULL, ConfigureLang, ComputerSystemCs->MemorySummary->MemoryMirroring);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for key: %a\n", __FUNCTION__, Arraykey));
    }
  }



  //
ON_RELEASE:

  //
  // Release resource.
  //
  if (EtagInDb != NULL) {
    FreePool (EtagInDb);
  }

  if (Arraykey != NULL) {
    FreePool (Arraykey);
  }

  Private->JsonStructProtocol->DestoryStructure (
                                 Private->JsonStructProtocol,
                                 (EFI_REST_JSON_STRUCTURE_HEADER *)Memory
                                 );

  return EFI_SUCCESS;
}

EFI_STATUS
ProvisioningMemoryProperties (
  IN  EFI_REST_JSON_STRUCTURE_PROTOCOL  *JsonStructProtocol,
  IN  CHAR8                             *IputJson,
  IN  CHAR8                             *ResourceId,  OPTIONAL
  IN  EFI_STRING                        ConfigureLang,
  IN  BOOLEAN                           ProvisionMode,
  OUT CHAR8                             **ResultJson
  )
{
  EFI_REDFISH_COMPUTERSYSTEM_V1_1_0     *ComputerSystem;
  EFI_REDFISH_COMPUTERSYSTEM_V1_1_0_CS  *ComputerSystemCs;
  EFI_STATUS                    Status;
  INT64                         *NumericValue;
  INT32                         *IntegerValue;
  BOOLEAN                       *BooleanValue;
  CHAR8                         *AsciiStringValue;
  BOOLEAN                       PropertyChanged;

  if (JsonStructProtocol == NULL || ResultJson == NULL || IS_EMPTY_STRING (IputJson) || IS_EMPTY_STRING (ConfigureLang)) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a provision for %s with: %s\n", __FUNCTION__, ConfigureLang, (ProvisionMode ? L"Provision all resource" : L"Provision existing resource")));

  *ResultJson = NULL;
  PropertyChanged = FALSE;

  Memory = NULL;
  Status = JsonStructProtocol->ToStructure (
                                 JsonStructProtocol,
                                 NULL,
                                 IputJson,
                                 (EFI_REST_JSON_STRUCTURE_HEADER **)&Memory
                                 );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, ToStructure failure: %r\n", __FUNCTION__, Status));
    return Status;
  }

  ComputerSystemCs = ComputerSystem->ComputerSystem;

  //
  // ID
  //
  if (MemoryCs->Id == NULL && !IS_EMPTY_STRING (ResourceId)) {
    MemoryCs->Id = AllocateCopyPool (AsciiStrSize (ResourceId), ResourceId);
  }

  //
  // Handle SYSTEMTYPE
  //
  if (PropertyChecker (ComputerSystemCs->SystemType, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "SystemType", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->SystemType, AsciiStringValue) != 0) {
        ComputerSystemCs->SystemType = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle ASSETTAG
  //
  if (PropertyChecker (ComputerSystemCs->AssetTag, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "AssetTag", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->AssetTag, AsciiStringValue) != 0) {
        ComputerSystemCs->AssetTag = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle MANUFACTURER
  //
  if (PropertyChecker (ComputerSystemCs->Manufacturer, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "Manufacturer", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->Manufacturer, AsciiStringValue) != 0) {
        ComputerSystemCs->Manufacturer = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle MODEL
  //
  if (PropertyChecker (ComputerSystemCs->Model, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "Model", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->Model, AsciiStringValue) != 0) {
        ComputerSystemCs->Model = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle SKU
  //
  if (PropertyChecker (ComputerSystemCs->SKU, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "SKU", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->SKU, AsciiStringValue) != 0) {
        ComputerSystemCs->SKU = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle SERIALNUMBER
  //
  if (PropertyChecker (ComputerSystemCs->SerialNumber, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "SerialNumber", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->SerialNumber, AsciiStringValue) != 0) {
        ComputerSystemCs->SerialNumber = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle PARTNUMBER
  //
  if (PropertyChecker (ComputerSystemCs->PartNumber, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "PartNumber", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->PartNumber, AsciiStringValue) != 0) {
        ComputerSystemCs->PartNumber = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle HOSTNAME
  //
  if (PropertyChecker (ComputerSystemCs->HostName, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "HostName", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->HostName, AsciiStringValue) != 0) {
        ComputerSystemCs->HostName = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle INDICATORLED
  //
  if (PropertyChecker (ComputerSystemCs->IndicatorLED, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "IndicatorLED", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->IndicatorLED, AsciiStringValue) != 0) {
        ComputerSystemCs->IndicatorLED = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle POWERSTATE
  //
  if (PropertyChecker (ComputerSystemCs->PowerState, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "PowerState", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->PowerState, AsciiStringValue) != 0) {
        ComputerSystemCs->PowerState = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle BOOT->BOOTSOURCEOVERRIDETARGET
  //
  if (PropertyChecker (ComputerSystemCs->Boot->BootSourceOverrideTarget, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "Boot/BootSourceOverrideTarget", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->Boot->BootSourceOverrideTarget, AsciiStringValue) != 0) {
        ComputerSystemCs->Boot->BootSourceOverrideTarget = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle BOOT->BOOTSOURCEOVERRIDEENABLED
  //
  if (PropertyChecker (ComputerSystemCs->Boot->BootSourceOverrideEnabled, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "Boot/BootSourceOverrideEnabled", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->Boot->BootSourceOverrideEnabled, AsciiStringValue) != 0) {
        ComputerSystemCs->Boot->BootSourceOverrideEnabled = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle BOOT->UEFITARGETBOOTSOURCEOVERRIDE
  //
  if (PropertyChecker (ComputerSystemCs->Boot->UefiTargetBootSourceOverride, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "Boot/UefiTargetBootSourceOverride", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->Boot->UefiTargetBootSourceOverride, AsciiStringValue) != 0) {
        ComputerSystemCs->Boot->UefiTargetBootSourceOverride = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle BOOT->BOOTSOURCEOVERRIDEMODE
  //
  if (PropertyChecker (ComputerSystemCs->Boot->BootSourceOverrideMode, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "Boot/BootSourceOverrideMode", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->Boot->BootSourceOverrideMode, AsciiStringValue) != 0) {
        ComputerSystemCs->Boot->BootSourceOverrideMode = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle BIOSVERSION
  //
  if (PropertyChecker (ComputerSystemCs->BiosVersion, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "BiosVersion", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->BiosVersion, AsciiStringValue) != 0) {
        ComputerSystemCs->BiosVersion = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle PROCESSORSUMMARY->COUNT
  //
  if (PropertyChecker (ComputerSystemCs->ProcessorSummary->Count, ProvisionMode)) {
    NumericValue = GetPropertyNumericValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "ProcessorSummary/Count", ConfigureLang);
    if (NumericValue != NULL) {
      if (ProvisionMode || *ComputerSystemCs->ProcessorSummary->Count != *NumericValue) {
        ComputerSystemCs->ProcessorSummary->Count = NumericValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle PROCESSORSUMMARY->MODEL
  //
  if (PropertyChecker (ComputerSystemCs->ProcessorSummary->Model, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "ProcessorSummary/Model", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->ProcessorSummary->Model, AsciiStringValue) != 0) {
        ComputerSystemCs->ProcessorSummary->Model = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle MEMORYSUMMARY->TOTALSYSTEMMEMORYGIB
  //
  if (PropertyChecker (ComputerSystemCs->MemorySummary->TotalSystemMemoryGiB, ProvisionMode)) {
    NumericValue = GetPropertyNumericValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "MemorySummary/TotalSystemMemoryGiB", ConfigureLang);
    if (NumericValue != NULL) {
      if (ProvisionMode || *ComputerSystemCs->MemorySummary->TotalSystemMemoryGiB != *NumericValue) {
        ComputerSystemCs->MemorySummary->TotalSystemMemoryGiB = NumericValue;
        PropertyChanged = TRUE;
      }
    }
  }

  //
  // Handle MEMORYSUMMARY->MEMORYMIRRORING
  //
  if (PropertyChecker (ComputerSystemCs->MemorySummary->MemoryMirroring, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, "MemorySummary/MemoryMirroring", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (ComputerSystemCs->MemorySummary->MemoryMirroring, AsciiStringValue) != 0) {
        ComputerSystemCs->MemorySummary->MemoryMirroring = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }



  //
  // Convert C structure back to JSON text.
  //
  Status = JsonStructProtocol->ToJson (
                                 JsonStructProtocol,
                                 (EFI_REST_JSON_STRUCTURE_HEADER *)Memory,
                                 ResultJson
                                 );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, ToJson() failed: %r\n", __FUNCTION__, Status));
    return Status;
  }

  //
  // Release resource.
  //
  JsonStructProtocol->DestoryStructure (
                        JsonStructProtocol,
                        (EFI_REST_JSON_STRUCTURE_HEADER *)Memory
                        );

  return (PropertyChanged ? EFI_SUCCESS : EFI_NOT_FOUND);
}

EFI_STATUS
ProvisioningMemoryResource (
  IN  REDFISH_RESOURCE_COMMON_PRIVATE   *Private,
  IN  UINTN                             Index,
  IN  EFI_STRING                        ConfigureLang
  )
{
  CHAR8       *MemoryJson;
  EFI_STATUS  Status;
  CHAR8       *NewResourceLocation;
  CHAR8       *NewKey;
  CHAR8       *EtagStr;
  CHAR8       ResourceId[16];
  CHAR8       NewUri[255];

  if (IS_EMPTY_STRING (ConfigureLang) || Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  AsciiSPrint (ResourceId, sizeof (ResourceId), "%d", Index);

  Status = ProvisioningMemoryProperties (
             Private->JsonStructProtocol,
             MemoryEmptyJson,
             ResourceId,
             ConfigureLang,
             TRUE,
             &MemoryJson
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, provisioning resource for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
    return Status;
  }

  Status = CreatePayloadToPostResource (Private->RedfishService, Private->Payload, MemoryJson, &NewResourceLocation, &EtagStr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, post memory resource for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
    goto RELEASE_RESOURCE;
  }

  NewUri[0] = '\0';

  //
  // Keep location of new resource.
  //
  if (NewResourceLocation != NULL) {
    //
    // Find key
    //
    NewKey = AsciiStrStr (NewResourceLocation, RESOURCE_SCHEMA);
    if (NewKey != NULL) {
      NewKey += 6;
      //
      // skip '/'
      //
      if (NewKey[0] == '/') {
        NewKey++;
      }
      SetConfigureLangWithkey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, NewKey, Index);
      AsciiSPrint (NewUri, sizeof (NewUri), "%a[%a]", Private->Uri, NewKey);
      FreePool (NewResourceLocation);
    }
  }

  //
  // Handle Etag
  //
  if (EtagStr != NULL) {
    SetEtagWithUri (EtagStr, NewUri);
    FreePool (EtagStr);
  }

RELEASE_RESOURCE:

  FreePool (MemoryJson);

  return Status;
}

EFI_STATUS
ProvisioningMemoryResources (
  IN  REDFISH_RESOURCE_COMMON_PRIVATE  *Private
  )
{
  UINTN                                        Index;
  EFI_STATUS                                   Status;
  REDFISH_FEATURE_ARRAY_TYPE_CONFIG_LANG_LIST  UnifiedConfigureLangList;

  if (Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = RedfishFeatureGetUnifiedArrayTypeConfigureLang (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, REDPATH_ARRAY_PATTERN, &UnifiedConfigureLangList);
  if (EFI_ERROR (Status) || UnifiedConfigureLangList.Count == 0) {
    DEBUG ((DEBUG_ERROR, "%a, No HII question found with redpath: %s: %r\n", __FUNCTION__, REDPATH_ARRAY_PATTERN, Status));
    return EFI_NOT_FOUND;
  }

  for (Index = 0; Index < UnifiedConfigureLangList.Count; Index++) {
    DEBUG ((DEBUG_INFO, "[%d] create memory resource from: %s\n", UnifiedConfigureLangList.List[Index].Index, UnifiedConfigureLangList.List[Index].ConfigureLang));
    ProvisioningMemoryResource (Private, UnifiedConfigureLangList.List[Index].Index, UnifiedConfigureLangList.List[Index].ConfigureLang);
    FreePool (UnifiedConfigureLangList.List[Index].ConfigureLang);
  }

  return EFI_SUCCESS;
}


EFI_STATUS
ProvisioningMemoryExistResource (
  IN  REDFISH_RESOURCE_COMMON_PRIVATE  *Private
  )
{
  if (Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private->Json = JsonDumpString (RedfishJsonInPayload (Private->Payload), EDKII_JSON_COMPACT);
  ASSERT (Private->Json != NULL);

  return RedfishUpdateResourceCommon (Private, Private->Json);
}

/**
  Provisioning redfish resource by given URI.

  @param[in]   This                Pointer to EFI_HP_REDFISH_HII_PROTOCOL instance.
  @param[in]   ResourceExist       TRUE if resource exists, PUT method will be used.
                                   FALSE if resource does not exist POST method is used.

  @retval EFI_SUCCESS              Value is returned successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
RedfishProvisioningResourceCommon (
  IN     REDFISH_RESOURCE_COMMON_PRIVATE  *Private,
  IN     BOOLEAN                          ResourceExist
  )
{
  if (Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  return (ResourceExist ? ProvisioningMemoryExistResource (Private) : ProvisioningMemoryResources (Private));
}

/**
  Check resource from given URI.

  @param[in]   This                Pointer to REDFISH_RESOURCE_COMMON_PRIVATE instance.
  @param[in]   Json                The JSON to consume.

  @retval EFI_SUCCESS              Value is returned successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
RedfishCheckResourceCommon (
  IN     REDFISH_RESOURCE_COMMON_PRIVATE  *Private,
  IN     CHAR8                            *Json
  )
{
  UINTN      Index;
  EFI_STATUS Status;
  EFI_STRING *ConfigureLangList;
  UINTN      Count;
  EFI_STRING Property;
  CHAR8      *PropertyAscii;
  UINTN      BuffSize;
  CHAR8      *Match;

  if (Private == NULL || IS_EMPTY_STRING (Json)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = RedfishPlatformConfigGetConfigureLang (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, REDPATH_ARRAY_PATTERN, &ConfigureLangList, &Count);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, BiosConfigToRedfishGetConfigureLangRegex failed: %r\n", __FUNCTION__, Status));
    return Status;
  }

  if (Count == 0) {
    return EFI_NOT_FOUND;
  }

  Status = EFI_SUCCESS;
  for (Index = 0; Index < Count; Index++) {

    Property = GetPropertyFromConfigureLang (ConfigureLangList[Index]);
    if (Property == NULL) {
      continue;
    }

    DEBUG ((DEBUG_INFO, "[%d] check resource from: %s\n", Index, Property));

    BuffSize = StrLen (Property) + 1;
    PropertyAscii = AllocatePool (BuffSize);
    UnicodeStrToAsciiStrS (Property, PropertyAscii, BuffSize);

    //
    // check to see if it is partial match.
    //
    Match = AsciiStrStr (Json, PropertyAscii);
    if (Match == NULL || AsciiStrnCmp (Match, PropertyAscii, AsciiStrLen (PropertyAscii)) != 0) {
      Status = EFI_NOT_FOUND;
      DEBUG ((DEBUG_ERROR, "%a, property %a is missing\n", __FUNCTION__, PropertyAscii));
    }

    FreePool (PropertyAscii);
  }

  FreePool (ConfigureLangList);

  return Status;
}


/**
  Update resource to given URI.

  @param[in]   This                Pointer to REDFISH_RESOURCE_COMMON_PRIVATE instance.
  @param[in]   Json                The JSON to consume.

  @retval EFI_SUCCESS              Value is returned successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
RedfishUpdateResourceCommon (
  IN     REDFISH_RESOURCE_COMMON_PRIVATE  *Private,
  IN     CHAR8                            *Json
  )
{
  EFI_STATUS Status;
  CHAR8      *MemoryJson;
  CHAR8      *ArrayKey;
  EFI_STRING ConfigureLang;
  CHAR8      *EtagStr;

  if (Private == NULL || IS_EMPTY_STRING (Json)) {
    return EFI_INVALID_PARAMETER;
  }

  MemoryJson = NULL;
  ConfigureLang = NULL;
  ArrayKey = NULL;

  Status = GetArraykeyFromUri (Private->Uri, &ArrayKey);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ConfigureLang = GetConfigureLangByKey (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, NULL, ArrayKey);
  if (ConfigureLang == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = ProvisioningMemoryProperties (
             Private->JsonStructProtocol,
             Json,
             NULL,
             ConfigureLang,
             FALSE,
             &MemoryJson
             );
  if (EFI_ERROR (Status)) {
    if (Status == EFI_NOT_FOUND) {
      DEBUG ((REDFISH_DEBUG_TRACE, "%a, update resource for %s ignored. Nothing changed\n", __FUNCTION__, ConfigureLang));
    } else {
      DEBUG ((DEBUG_ERROR, "%a, update resource for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
    }
    goto ON_RELEASE;
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a, update resource for %s\n", __FUNCTION__, ConfigureLang));
  //
  // PUT back to instance
  //
  Status = CreatePayloadToPatchResource (Private->RedfishService, Private->Payload, MemoryJson, &EtagStr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, post memory resource for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
  }

  //
  // Handle Etag
  //
  if (EtagStr != NULL) {
    SetEtagWithUri (EtagStr, Private->Uri);
    FreePool (EtagStr);
  }

ON_RELEASE:

  if (MemoryJson != NULL) {
    FreePool (MemoryJson);
  }

  if (ConfigureLang != NULL) {
    FreePool (ConfigureLang);
  }

  if (ArrayKey != NULL) {
    FreePool (ArrayKey);
  }

  return Status;
}