/** @file
  Redfish feature driver implementation - common functions

  (C) Copyright 2020-2022 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "RedfishComputerSystemCommon.h"

CHAR8 MemoryEmptyJson[] = "{\"@odata.id\": \"\", \"@odata.type\": \"#ComputerSystem.v1_1_0.ComputerSystem\", \"Id\": \"\", \"Name\": \"\"}";

REDFISH_RESOURCE_COMMON_PRIVATE *mRedfishResourcePrivate = NULL;

/**
  Consume resource from given URI.

  @param[in]   This                Pointer to REDFISH_RESOURCE_COMMON_PRIVATE instance.
  @param[in]   Json                The JSON to consume.
  @param[in]   HeaderEtag          The Etag string returned in HTTP header.

  @retval EFI_SUCCESS              Value is returned successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
RedfishConsumeResourceCommon (
  IN  REDFISH_RESOURCE_COMMON_PRIVATE *Private,
  IN  CHAR8                           *Json,
  IN  CHAR8                           *HeaderEtag OPTIONAL
  )
{
  EFI_STATUS                    Status;
  EFI_REDFISH_COMPUTERSYSTEM_V1_1_0     *ComputerSystem;
  EFI_REDFISH_COMPUTERSYSTEM_V1_1_0_CS  *ComputerSystemCs;
  EFI_STRING                    ConfigureLang;
  CHAR8                         *EtagInDb;

  if (Private == NULL || IS_EMPTY_STRING (Json)) {
    return EFI_INVALID_PARAMETER;
  }

  ComputerSystem = NULL;
  ComputerSystemCs = NULL;
  ConfigureLang = NULL;
  EtagInDb = NULL;

  Status = Private->JsonStructProtocol->ToStructure (
                                          Private->JsonStructProtocol,
                                          NULL,
                                          Json,
                                          (EFI_REST_JSON_STRUCTURE_HEADER **)&ComputerSystem
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
  if (EtagInDb != NULL && HeaderEtag != NULL) {
    if (AsciiStrCmp (EtagInDb, HeaderEtag) == 0) {
      //
      // No change
      //
      DEBUG ((DEBUG_INFO, "%a, ETAG: [%a] no change, ignore consume action\n", __FUNCTION__, EtagInDb));
      goto ON_RELEASE;
    }
  }

  //
  // Handle SYSTEMTYPE
  //
  if (ComputerSystemCs->SystemType != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "SystemType");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->SystemType);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle ASSETTAG
  //
  if (ComputerSystemCs->AssetTag != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "AssetTag");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->AssetTag);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle MANUFACTURER
  //
  if (ComputerSystemCs->Manufacturer != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "Manufacturer");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->Manufacturer);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle MODEL
  //
  if (ComputerSystemCs->Model != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "Model");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->Model);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle SKU
  //
  if (ComputerSystemCs->SKU != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "SKU");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->SKU);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle SERIALNUMBER
  //
  if (ComputerSystemCs->SerialNumber != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "SerialNumber");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->SerialNumber);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle PARTNUMBER
  //
  if (ComputerSystemCs->PartNumber != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "PartNumber");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->PartNumber);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle HOSTNAME
  //
  if (ComputerSystemCs->HostName != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "HostName");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->HostName);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle INDICATORLED
  //
  if (ComputerSystemCs->IndicatorLED != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "IndicatorLED");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->IndicatorLED);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle POWERSTATE
  //
  if (ComputerSystemCs->PowerState != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "PowerState");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->PowerState);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle BOOT->BOOTSOURCEOVERRIDETARGET
  //
  if (ComputerSystemCs->Boot->BootSourceOverrideTarget != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "Boot/BootSourceOverrideTarget");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->Boot->BootSourceOverrideTarget);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle BOOT->BOOTSOURCEOVERRIDEENABLED
  //
  if (ComputerSystemCs->Boot->BootSourceOverrideEnabled != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "Boot/BootSourceOverrideEnabled");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->Boot->BootSourceOverrideEnabled);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle BOOT->UEFITARGETBOOTSOURCEOVERRIDE
  //
  if (ComputerSystemCs->Boot->UefiTargetBootSourceOverride != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "Boot/UefiTargetBootSourceOverride");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->Boot->UefiTargetBootSourceOverride);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle BOOT->BOOTSOURCEOVERRIDEMODE
  //
  if (ComputerSystemCs->Boot->BootSourceOverrideMode != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "Boot/BootSourceOverrideMode");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->Boot->BootSourceOverrideMode);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle BIOSVERSION
  //
  if (ComputerSystemCs->BiosVersion != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "BiosVersion");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->BiosVersion);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle PROCESSORSUMMARY->COUNT
  //
  if (ComputerSystemCs->ProcessorSummary->Count != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "ProcessorSummary/Count");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsNumericType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, (UINTN)*ComputerSystemCs->ProcessorSummary->Count);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle PROCESSORSUMMARY->MODEL
  //
  if (ComputerSystemCs->ProcessorSummary->Model != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "ProcessorSummary/Model");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->ProcessorSummary->Model);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle MEMORYSUMMARY->TOTALSYSTEMMEMORYGIB
  //
  if (ComputerSystemCs->MemorySummary->TotalSystemMemoryGiB != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "MemorySummary/TotalSystemMemoryGiB");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsNumericType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, (UINTN)*ComputerSystemCs->MemorySummary->TotalSystemMemoryGiB);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle MEMORYSUMMARY->MEMORYMIRRORING
  //
  if (ComputerSystemCs->MemorySummary->MemoryMirroring != NULL) {
    //
    // Find corresponding redpath for collection resource.
    //
    ConfigureLang = GetConfigureLang (ComputerSystemCs->odata_id, "MemorySummary/MemoryMirroring");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, ComputerSystemCs->MemorySummary->MemoryMirroring);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
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

  Private->JsonStructProtocol->DestoryStructure (
                                 Private->JsonStructProtocol,
                                 (EFI_REST_JSON_STRUCTURE_HEADER *)ComputerSystem
                                 );

  return EFI_SUCCESS;
}

EFI_STATUS
ProvisioningProperties (
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
  EFI_STATUS                            Status;
  INT64                                 *NumericValue;
  CHAR8                                 *AsciiStringValue;
  BOOLEAN                               PropertyChanged;

  if (JsonStructProtocol == NULL || ResultJson == NULL || IS_EMPTY_STRING (IputJson) || IS_EMPTY_STRING (ConfigureLang)) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a provision for %s with: %s\n", __FUNCTION__, ConfigureLang, (ProvisionMode ? L"Provision all resource" : L"Provision existing resource")));

  *ResultJson = NULL;
  PropertyChanged = FALSE;

  ComputerSystem = NULL;
  Status = JsonStructProtocol->ToStructure (
                                 JsonStructProtocol,
                                 NULL,
                                 IputJson,
                                 (EFI_REST_JSON_STRUCTURE_HEADER **)&ComputerSystem
                                 );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, ToStructure failure: %r\n", __FUNCTION__, Status));
    return Status;
  }

  ComputerSystemCs = ComputerSystem->ComputerSystem;

  //
  // ID
  //
  if (ComputerSystemCs->Id == NULL && !IS_EMPTY_STRING (ResourceId)) {
    ComputerSystemCs->Id = AllocateCopyPool (AsciiStrSize (ResourceId), ResourceId);
  }

  //
  // Handle SYSTEMTYPE
  //
  if (PropertyChecker (ComputerSystemCs->SystemType, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"SystemType", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"AssetTag", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"Manufacturer", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"Model", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"SKU", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"SerialNumber", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"PartNumber", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"HostName", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"IndicatorLED", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"PowerState", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"Boot/BootSourceOverrideTarget", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"Boot/BootSourceOverrideEnabled", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"Boot/UefiTargetBootSourceOverride", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"Boot/BootSourceOverrideMode", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"BiosVersion", ConfigureLang);
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
    NumericValue = GetPropertyNumericValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"ProcessorSummary/Count", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"ProcessorSummary/Model", ConfigureLang);
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
    NumericValue = GetPropertyNumericValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"MemorySummary/TotalSystemMemoryGiB", ConfigureLang);
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
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"MemorySummary/MemoryMirroring", ConfigureLang);
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
                                 (EFI_REST_JSON_STRUCTURE_HEADER *)ComputerSystem,
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
                        (EFI_REST_JSON_STRUCTURE_HEADER *)ComputerSystem
                        );

  return (PropertyChanged ? EFI_SUCCESS : EFI_NOT_FOUND);
}

EFI_STATUS
ProvisioningResource (
  IN  REDFISH_RESOURCE_COMMON_PRIVATE   *Private,
  IN  UINTN                             Index,
  IN  EFI_STRING                        ConfigureLang
  )
{
  CHAR8       *Json;
  EFI_STATUS  Status;
  EFI_STRING  NewResourceLocation;
  CHAR8       *EtagStr;
  CHAR8       ResourceId[16];

  if (IS_EMPTY_STRING (ConfigureLang) || Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  AsciiSPrint (ResourceId, sizeof (ResourceId), "%d", Index);

  Status = ProvisioningProperties (
             Private->JsonStructProtocol,
             MemoryEmptyJson,
             ResourceId,
             ConfigureLang,
             TRUE,
             &Json
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, provisioning resource for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
    return Status;
  }

  Status = CreatePayloadToPostResource (Private->RedfishService, Private->Payload, Json, &NewResourceLocation, &EtagStr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, post memory resource for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
    goto RELEASE_RESOURCE;
  }

  ASSERT (NewResourceLocation != NULL);

  //
  // Keep location of new resource.
  //
  if (NewResourceLocation != NULL) {
    RedfisSetRedfishUri (ConfigureLang, NewResourceLocation);
  }

  //
  // Handle Etag
  //
  if (EtagStr != NULL) {
    SetEtagWithUri (EtagStr, NewResourceLocation);
    FreePool (EtagStr);
  }

RELEASE_RESOURCE:

  if (NewResourceLocation != NULL) {
    FreePool (NewResourceLocation);
  }

  if (Json != NULL) {
    FreePool (Json);
  }

  return Status;
}

EFI_STATUS
ProvisioningResources (
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
    ProvisioningResource (Private, UnifiedConfigureLangList.List[Index].Index, UnifiedConfigureLangList.List[Index].ConfigureLang);
    FreePool (UnifiedConfigureLangList.List[Index].ConfigureLang);
  }

  return EFI_SUCCESS;
}


EFI_STATUS
ProvisioningExistResource (
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

  return (ResourceExist ? ProvisioningExistResource (Private) : ProvisioningResources (Private));
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

    DEBUG ((DEBUG_INFO, "%a, [%d] check resource from: %s\n", __FUNCTION__, Index, Property));

    PropertyAscii = StrUnicodeToAscii (Property);
    if (PropertyAscii == NULL) {
      DEBUG ((DEBUG_ERROR, "%a, StrUnicodeToAscii failed\n", __FUNCTION__));
      continue;
    }

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
  IN     CHAR8                            *InputJson
  )
{
  EFI_STATUS Status;
  CHAR8      *Json;
  EFI_STRING ConfigureLang;
  CHAR8      *EtagStr;

  if (Private == NULL || IS_EMPTY_STRING (InputJson)) {
    return EFI_INVALID_PARAMETER;
  }

  Json = NULL;
  ConfigureLang = NULL;

  ConfigureLang = RedfishGetConfigLanguage (Private->Uri);
  if (ConfigureLang == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = ProvisioningProperties (
             Private->JsonStructProtocol,
             InputJson,
             NULL,
             ConfigureLang,
             FALSE,
             &Json
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
  Status = CreatePayloadToPatchResource (Private->RedfishService, Private->Payload, Json, &EtagStr);
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

  if (Json != NULL) {
    FreePool (Json);
  }

  if (ConfigureLang != NULL) {
    FreePool (ConfigureLang);
  }

  return Status;
}

/**
  Identify resource from given URI.

  @param[in]   This                Pointer to REDFISH_RESOURCE_COMMON_PRIVATE instance.
  @param[in]   Json                The JSON to consume.

  @retval EFI_SUCCESS              Value is returned successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
RedfishIdentifyResourceCommon (
  IN     REDFISH_RESOURCE_COMMON_PRIVATE  *Private,
  IN     CHAR8                            *Json
  )
{
  BOOLEAN     Supported;
  EFI_STATUS  Status;
  EFI_STRING  *ConfigureLangList;
  UINTN       Count;
  EFI_STRING  EndOfChar;

  Supported = RedfishIdentifyResource (Private->Uri, Private->Json);
  if (Supported) {
    Status = RedfishPlatformConfigGetConfigureLang (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, REDPATH_ARRAY_PATTERN, &ConfigureLangList, &Count);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a, BiosConfigToRedfishGetConfigureLangRegex failed: %r\n", __FUNCTION__, Status));
      return Status;
    }

    if (Count == 0) {
      return EFI_SUCCESS;
    }

    EndOfChar = StrStr (ConfigureLangList[0], L"}");
    if (EndOfChar == NULL) {
      ASSERT (FALSE);
      return EFI_DEVICE_ERROR;
    }

    *(++EndOfChar) = '\0';

    //
    // Keep URI and ConfigLang mapping
    //
    RedfisSetRedfishUri (ConfigureLangList[0], Private->Uri);
    FreePool (ConfigureLangList);

    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}