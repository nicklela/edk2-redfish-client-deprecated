/** @file
  Redfish feature driver implementation - common functions

  (C) Copyright 2020-2022 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "BiosCommon.h"

CHAR8 BiosEmptyJson[] = "{\"@odata.id\": \"\", \"@odata.type\": \"#Bios.v1_0_9.Bios\", \"Id\": \"\", \"Name\": \"\"}";

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
  EFI_STATUS                   Status;
  EFI_REDFISH_BIOS_V1_0_9     *Bios;
  EFI_REDFISH_BIOS_V1_0_9_CS  *BiosCs;
  EFI_STRING                   ConfigureLang;
  RedfishCS_Type_EmptyProp_CS_Data   *EmptyPropCs;

  

  if (Private == NULL || IS_EMPTY_STRING (Json)) {
    return EFI_INVALID_PARAMETER;
  }

  Bios = NULL;
  BiosCs = NULL;
  ConfigureLang = NULL;

  Status = Private->JsonStructProtocol->ToStructure (
                                          Private->JsonStructProtocol,
                                          NULL,
                                          Json,
                                          (EFI_REST_JSON_STRUCTURE_HEADER **)&Bios
                                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, ToStructure() failed: %r\n", __FUNCTION__, Status));
    return Status;
  }

  BiosCs = Bios->Bios;

  //
  // Check ETAG to see if we need to consume it
  //
  if (CheckEtag (Private->Uri, HeaderEtag, NULL)) {
    //
    // No change
    //
    DEBUG ((DEBUG_INFO, "%a, ETAG: %s has no change, ignore consume action\n", __FUNCTION__, Private->Uri));
    Status = EFI_ALREADY_STARTED;    
    goto ON_RELEASE;
  }

  //
  // Handle ATTRIBUTEREGISTRY
  //
  if (BiosCs->AttributeRegistry != NULL) {
    //
    // Find corresponding configure language for collection resource.
    //
    ConfigureLang = GetConfigureLang (BiosCs->odata_id, "AttributeRegistry");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsStringType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, BiosCs->AttributeRegistry);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }

  //
  // Handle ATTRIBUTES
  //
  if (BiosCs->Attributes == NULL) {
    BiosCs->Attributes = AllocateZeroPool (sizeof (RedfishBios_V1_0_9_Attributes_CS));
    ASSERT (BiosCs->Attributes != NULL);
  }

  //
  // Handle ATTRIBUTES->EmptyProperty
  //
  if (BiosCs->Attributes != NULL) {
    //
    // Validate empty property.
    //
    if (BiosCs->Attributes->Prop.BackLink == BiosCs->Attributes->Prop.ForwardLink) {
      goto ON_RELEASE;
    }
    EmptyPropCs = (RedfishCS_Type_EmptyProp_CS_Data *)BiosCs->Attributes->Prop.ForwardLink;
    if (EmptyPropCs->Header.ResourceType == RedfishCS_Type_JSON) {
      DEBUG ((DEBUG_ERROR, "%a, Empty property with RedfishCS_Type_JSON type resource is not supported yet. (%s)\n", __FUNCTION__, Private->Uri));
      goto ON_RELEASE;
    }
    //
    // Find corresponding configure language for collection resource.
    //
    ConfigureLang = GetConfigureLang (BiosCs->odata_id, "Attributes");
    if (ConfigureLang != NULL) {
      Status = ApplyFeatureSettingsVagueType (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, EmptyPropCs->KeyValuePtr, EmptyPropCs->NunmOfProperties);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply setting for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
      }

      FreePool (ConfigureLang);
    } else {
      DEBUG ((DEBUG_ERROR, "%a, can not get configure language for URI: %s\n", __FUNCTION__, Private->Uri));
    }
  }


ON_RELEASE:

  //
  // Release resource.
  //
  Private->JsonStructProtocol->DestoryStructure (
                                 Private->JsonStructProtocol,
                                 (EFI_REST_JSON_STRUCTURE_HEADER *)Bios
                                 );

  return EFI_SUCCESS;
}

EFI_STATUS
ProvisioningBiosProperties (
  IN  EFI_REST_JSON_STRUCTURE_PROTOCOL  *JsonStructProtocol,
  IN  CHAR8                             *InputJson,
  IN  CHAR8                             *ResourceId,  OPTIONAL
  IN  EFI_STRING                        ConfigureLang,
  IN  BOOLEAN                           ProvisionMode,
  OUT CHAR8                             **ResultJson
  )
{
  EFI_REDFISH_BIOS_V1_0_9     *Bios;
  EFI_REDFISH_BIOS_V1_0_9_CS  *BiosCs;
  EFI_STATUS                    Status;
  BOOLEAN                       PropertyChanged;  
  CHAR8                         *AsciiStringValue;
  RedfishCS_EmptyProp_KeyValue  *PropertyVagueValues;
  UINT32                        VagueValueNumber;



  if (JsonStructProtocol == NULL || ResultJson == NULL || IS_EMPTY_STRING (InputJson) || IS_EMPTY_STRING (ConfigureLang)) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a provision for %s with: %s\n", __FUNCTION__, ConfigureLang, (ProvisionMode ? L"Provision resource" : L"Update resource")));

  *ResultJson = NULL;
  PropertyChanged = FALSE;

  Bios = NULL;
  Status = JsonStructProtocol->ToStructure (
                                 JsonStructProtocol,
                                 NULL,
                                 InputJson,
                                 (EFI_REST_JSON_STRUCTURE_HEADER **)&Bios
                                 );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, ToStructure failure: %r\n", __FUNCTION__, Status));
    return Status;
  }

  BiosCs = Bios->Bios;

  //
  // ID
  //
  if (BiosCs->Id == NULL && !IS_EMPTY_STRING (ResourceId)) {
    BiosCs->Id = AllocateCopyPool (AsciiStrSize (ResourceId), ResourceId);
  }

  //
  // Handle ATTRIBUTEREGISTRY
  //
  if (PropertyChecker (BiosCs->AttributeRegistry, ProvisionMode)) {
    AsciiStringValue = GetPropertyStringValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"AttributeRegistry", ConfigureLang);
    if (AsciiStringValue != NULL) {
      if (ProvisionMode || AsciiStrCmp (BiosCs->AttributeRegistry, AsciiStringValue) != 0) {
        BiosCs->AttributeRegistry = AsciiStringValue;
        PropertyChanged = TRUE;
      }
    }
  }
  //
  // Handle ATTRIBUTES
  //
  if (BiosCs->Attributes != NULL) {
    //
    // Handle ATTRIBUTES
    //
    if (PropertyChecker (BiosCs->Attributes, ProvisionMode)) {
      PropertyVagueValues = GetPropertyVagueValue (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L"Attributes", ConfigureLang, &VagueValueNumber);
      if (PropertyVagueValues != NULL) {
        if (ProvisionMode || !CompareRedfishPropertyVagueValues (
                               ((RedfishCS_Type_EmptyProp_CS_Data *)BiosCs->Attributes->Prop.ForwardLink)->KeyValuePtr,
                               ((RedfishCS_Type_EmptyProp_CS_Data *)BiosCs->Attributes->Prop.ForwardLink)->NunmOfProperties,
                               PropertyVagueValues,
                               VagueValueNumber)) {
          //
          // Use the properties on system to replace the one on Redfish service.
          //
          FreeEmptyPropKeyValueList (((RedfishCS_Type_EmptyProp_CS_Data *)BiosCs->Attributes->Prop.ForwardLink)->KeyValuePtr);
          ((RedfishCS_Type_EmptyProp_CS_Data *)BiosCs->Attributes->Prop.ForwardLink)->KeyValuePtr = PropertyVagueValues;
          ((RedfishCS_Type_EmptyProp_CS_Data *)BiosCs->Attributes->Prop.ForwardLink)->NunmOfProperties = VagueValueNumber;
          PropertyChanged = TRUE;
        }
      }
    }
  }


  //
  // Convert C structure back to JSON text.
  //
  Status = JsonStructProtocol->ToJson (
                                 JsonStructProtocol,
                                 (EFI_REST_JSON_STRUCTURE_HEADER *)Bios,
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
                        (EFI_REST_JSON_STRUCTURE_HEADER *)Bios
                        );

  return (PropertyChanged ? EFI_SUCCESS : EFI_NOT_FOUND);
}

EFI_STATUS
ProvisioningBiosResource (
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
  
  EtagStr = NULL;
  AsciiSPrint (ResourceId, sizeof (ResourceId), "%d", Index);

  Status = ProvisioningBiosProperties (
             Private->JsonStructProtocol,
             BiosEmptyJson,
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
    DEBUG ((DEBUG_ERROR, "%a, post Bios resource for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
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
ProvisioningBiosResources (
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
    DEBUG ((DEBUG_ERROR, "%a, No HII question found with configure language: %s: %r\n", __FUNCTION__, REDPATH_ARRAY_PATTERN, Status));
    return EFI_NOT_FOUND;
  }

  for (Index = 0; Index < UnifiedConfigureLangList.Count; Index++) {
    DEBUG ((DEBUG_INFO, "[%d] create Bios resource from: %s\n", UnifiedConfigureLangList.List[Index].Index, UnifiedConfigureLangList.List[Index].ConfigureLang));
    ProvisioningBiosResource (Private, UnifiedConfigureLangList.List[Index].Index, UnifiedConfigureLangList.List[Index].ConfigureLang);
    FreePool (UnifiedConfigureLangList.List[Index].ConfigureLang);
  }

  return EFI_SUCCESS;
}


EFI_STATUS
ProvisioningBiosExistResource (
  IN  REDFISH_RESOURCE_COMMON_PRIVATE  *Private
  )
{
  EFI_STATUS Status;
  EFI_STRING ConfigureLang;
  CHAR8      *EtagStr;
  CHAR8      *Json;

  if (Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EtagStr = NULL;
  Json = NULL;
  ConfigureLang = NULL;

  ConfigureLang = RedfishGetConfigLanguage (Private->Uri);
  if (ConfigureLang == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = ProvisioningBiosProperties (
             Private->JsonStructProtocol,
             BiosEmptyJson,
             NULL,
             ConfigureLang,
             TRUE,
             &Json
             );
  if (EFI_ERROR (Status)) {
    if (Status == EFI_NOT_FOUND) {
      DEBUG ((REDFISH_DEBUG_TRACE, "%a, provisioning existing resource for %s ignored. Nothing changed\n", __FUNCTION__, ConfigureLang));
    } else {
      DEBUG ((DEBUG_ERROR, "%a, provisioning existing resource for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
    }
    goto ON_RELEASE;
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a, provisioning existing resource for %s\n", __FUNCTION__, ConfigureLang));
  //
  // PUT back to instance
  //
  Status = CreatePayloadToPatchResource (Private->RedfishService, Private->Payload, Json, &EtagStr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, patch resource for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
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

  return (ResourceExist ? ProvisioningBiosExistResource (Private) : ProvisioningBiosResources (Private));
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

    DEBUG ((DEBUG_INFO, "%a, [%d] check attribute for: %s\n", __FUNCTION__, Index, Property));
    if (!MatchPropertyWithJsonContext (Property, Json)) {
      DEBUG ((DEBUG_INFO, "%a, property is missing: %s\n", __FUNCTION__, Property));
      Status = EFI_NOT_FOUND;
    }
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

  EtagStr = NULL;
  Json = NULL;
  ConfigureLang = NULL;

  ConfigureLang = RedfishGetConfigLanguage (Private->Uri);
  if (ConfigureLang == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = ProvisioningBiosProperties (
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
    DEBUG ((DEBUG_ERROR, "%a, patch resource for %s failed: %r\n", __FUNCTION__, ConfigureLang, Status));
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