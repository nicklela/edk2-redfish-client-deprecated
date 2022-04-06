/** @file
  Redfish feature utility library implementation

  (C) Copyright 2020-2022 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "RedfishFeatureUtilityInternal.h"

EDKII_REDFISH_ETAG_PROTOCOL             *mEtagProtocol = NULL;
EDKII_REDFISH_CONFIG_LANG_MAP_PROTOCOL  *mConfigLangMapProtocol = NULL;


EFI_STATUS
RedfishLocateProtocol (
  IN  VOID      **ProtocolInstance,
  IN  EFI_GUID  *ProtocolGuid
  )
{
  EFI_STATUS  Status;

  if (ProtocolInstance == NULL || ProtocolGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (*ProtocolInstance != NULL) {
    return EFI_SUCCESS;
  }

  Status = gBS->LocateProtocol (
                  ProtocolGuid,
                  NULL,
                  ProtocolInstance
                  );
  return Status;
}


/**

  Get array key by parsing the URI.

  @param[in]  Uri     URI with array key.
  @param[out] ArrayKey  Array key in given URI string.

  @retval     EFI_SUCCESS         Array key is found.
  @retval     Others              Errors occur.

**/
EFI_STATUS
GetArraykeyFromUri (
  IN  CHAR8   *Uri,
  OUT CHAR8   **ArrayKey
  )
{
  CHAR8      *LeftBracket;
  UINTN      Index;

  if (IS_EMPTY_STRING (Uri) || ArrayKey == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *ArrayKey = NULL;

  //
  // Loop through Uri and find last '['
  //
  LeftBracket = NULL;
  for (Index = 0; Uri[Index] != '\0'; Index++) {
    if (Uri[Index] == '[') {
      LeftBracket = &Uri[Index];
    }
  }

  if (LeftBracket == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // skip '/'
  //
  ++LeftBracket;

  *ArrayKey = AllocateCopyPool (AsciiStrSize (LeftBracket), LeftBracket);
  if (*ArrayKey == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // remove ']'
  //
  *(*ArrayKey + AsciiStrLen (*ArrayKey) - 1) = '\0';

  return EFI_SUCCESS;
}

/**

  Keep ETAG string and URI string in database.

  @param[in]  EtagStr   ETAG string.
  @param[in]  Uri       URI string.

  @retval     EFI_SUCCESS     ETAG and URI are applied successfully.
  @retval     Others          Errors occur.

**/
EFI_STATUS
SetEtagWithUri (
  IN  CHAR8       *EtagStr,
  IN  EFI_STRING  Uri
  )
{
  EFI_STATUS  Status;
  CHAR8       *AsciiUri;

  if (IS_EMPTY_STRING (EtagStr) || IS_EMPTY_STRING (Uri)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = RedfishLocateProtocol ((VOID **)&mEtagProtocol, &gEdkIIRedfishETagProtocolGuid);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, fail to locate gEdkIIRedfishETagProtocolGuid: %r\n", __FUNCTION__, Status));
    return Status;
  }

  AsciiUri = StrUnicodeToAscii (Uri);
  if (AsciiUri == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mEtagProtocol->Set (mEtagProtocol, AsciiUri, EtagStr);
  mEtagProtocol->Flush (mEtagProtocol);

  FreePool (AsciiUri);

  return EFI_SUCCESS;
}

/**

  Find ETAG string that refers to given URI.

  @param[in]  Uri       Target URI string.

  @retval     CHAR8 *   ETAG string
  @retval     NULL      No ETAG is found.

**/
CHAR8 *
GetEtagWithUri (
  IN  EFI_STRING  Uri
  )
{
  EFI_STATUS  Status;
  CHAR8       *AsciiUri;
  CHAR8       *EtagStr;

  if (IS_EMPTY_STRING (Uri)) {
    return NULL;
  }

  Status = RedfishLocateProtocol ((VOID **)&mEtagProtocol, &gEdkIIRedfishETagProtocolGuid);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, fail to locate gEdkIIRedfishETagProtocolGuid: %r\n", __FUNCTION__, Status));
    return NULL;
  }

  AsciiUri = StrUnicodeToAscii (Uri);
  if (AsciiUri == NULL) {
    return NULL;
  }

  Status = mEtagProtocol->Get (mEtagProtocol, AsciiUri, &EtagStr);

  FreePool (AsciiUri);

  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return EtagStr;
}

/**

  Convert Unicode string to ASCII string. It's call responsibility to release returned buffer.

  @param[in]  UnicodeStr      Unicode string to convert.

  @retval     CHAR8 *         ASCII string returned.
  @retval     NULL            Errors occur.

**/
CHAR8 *
StrUnicodeToAscii (
  IN EFI_STRING   UnicodeStr
  )
{
  CHAR8 *AsciiStr;
  UINTN AsciiStrSize;
  EFI_STATUS Status;

  if (IS_EMPTY_STRING (UnicodeStr)) {
    return NULL;
  }

  AsciiStrSize = StrLen (UnicodeStr) + 1;
  AsciiStr = AllocatePool (AsciiStrSize);
  if (AsciiStr == NULL) {
    return NULL;
  }

  Status = UnicodeStrToAsciiStrS (UnicodeStr, AsciiStr, AsciiStrSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UnicodeStrToAsciiStrS failed: %r\n", Status));
    FreePool (AsciiStr);
    return NULL;
  }

  return AsciiStr;
}

/**

  Convert ASCII string to Unicode string. It's call responsibility to release returned buffer.

  @param[in]  AsciiStr        ASCII string to convert.

  @retval     EFI_STRING      Unicode string returned.
  @retval     NULL            Errors occur.

**/
EFI_STRING
StrAsciiToUnicode (
  IN CHAR8  *AsciiStr
  )
{
  EFI_STRING  UnicodeStr;
  UINTN       UnicodeStrSize;
  EFI_STATUS  Status;

  if (IS_EMPTY_STRING (AsciiStr)) {
    return NULL;
  }

  UnicodeStrSize = (AsciiStrLen (AsciiStr) + 1) * sizeof (CHAR16);
  UnicodeStr = AllocatePool (UnicodeStrSize);
  if (UnicodeStr == NULL) {
    return NULL;
  }

  Status = AsciiStrToUnicodeStrS (AsciiStr, UnicodeStr, UnicodeStrSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "t failed: %r\n", Status));
    FreePool (UnicodeStr);
    return NULL;
  }

  return UnicodeStr;
}

/**

  Apply property value to UEFI HII database in string type.

  @param[in]  Schema        Property schema.
  @param[in]  Version       Property schema version.
  @param[in]  ConfigureLang Configure language refers to this property.
  @param[in]  FeatureValue  New value to set.

  @retval     EFI_SUCCESS     New value is applied successfully.
  @retval     Others          Errors occur.

**/
EFI_STATUS
ApplyFeatureSettingsStringType (
  IN  CHAR8      *Schema,
  IN  CHAR8      *Version,
  IN  EFI_STRING ConfigureLang,
  IN  CHAR8      *FeatureValue
  )
{
  EFI_STATUS          Status;
  EDKII_REDFISH_VALUE RedfishValue;

  if (IS_EMPTY_STRING (Schema) || IS_EMPTY_STRING (Version) || IS_EMPTY_STRING (ConfigureLang) || FeatureValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get the current value from HII
  //
  Status = RedfishPlatformConfigGetValue (Schema, Version, ConfigureLang, &RedfishValue);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, %a.%a %s failed: %r\n", __FUNCTION__, Schema, Version, ConfigureLang, Status));
  } else {

    if (RedfishValue.Type != REDFISH_VALUE_TYPE_STRING) {
       DEBUG ((DEBUG_ERROR, "%a, %a.%a %s value is not string type\n", __FUNCTION__, Schema, Version, ConfigureLang));
      return EFI_DEVICE_ERROR;
    }

    if (AsciiStrCmp (FeatureValue, RedfishValue.Value.Buffer) != 0) {
      //
      // Apply settings from redfish
      //
      DEBUG ((DEBUG_INFO, "%a, %a.%a apply %s from %a to %a\n", __FUNCTION__, Schema, Version, ConfigureLang, RedfishValue.Value.Buffer, FeatureValue));

      FreePool (RedfishValue.Value.Buffer);
      RedfishValue.Value.Buffer = FeatureValue;

      Status = RedfishPlatformConfigSetValue (Schema, Version, ConfigureLang, RedfishValue);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply %s to %s failed: %r\n", __FUNCTION__, ConfigureLang, FeatureValue, Status));
      }
    } else {
      DEBUG ((DEBUG_ERROR, "%a, %a.%a %s value is: %s\n", __FUNCTION__, Schema, Version, ConfigureLang, RedfishValue.Value.Buffer, Status));
    }
  }

  return Status;
}

/**

  Apply property value to UEFI HII database in numric type.

  @param[in]  Schema        Property schema.
  @param[in]  Version       Property schema version.
  @param[in]  ConfigureLang Configure language refers to this property.
  @param[in]  FeatureValue  New value to set.

  @retval     EFI_SUCCESS     New value is applied successfully.
  @retval     Others          Errors occur.

**/
EFI_STATUS
ApplyFeatureSettingsNumericType (
  IN  CHAR8      *Schema,
  IN  CHAR8      *Version,
  IN  EFI_STRING ConfigureLang,
  IN  INTN       FeatureValue
  )
{
  EFI_STATUS          Status;
  EDKII_REDFISH_VALUE RedfishValue;

  if (IS_EMPTY_STRING (Schema) || IS_EMPTY_STRING (Version) || IS_EMPTY_STRING (ConfigureLang)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get the current value from HII
  //
  Status = RedfishPlatformConfigGetValue (Schema, Version, ConfigureLang, &RedfishValue);
  if (EFI_ERROR (Status)) {
   DEBUG ((DEBUG_ERROR, "%a, %a.%a %s failed: %r\n", __FUNCTION__, Schema, Version, ConfigureLang, Status));
  } else {

    if (RedfishValue.Type != REDFISH_VALUE_TYPE_INTEGER) {
       DEBUG ((DEBUG_ERROR, "%a, %a.%a %s value is not numeric type\n", __FUNCTION__, Schema, Version, ConfigureLang));
      return EFI_DEVICE_ERROR;
    }

    if (RedfishValue.Value.Integer != FeatureValue) {
      //
      // Apply settings from redfish
      //
      DEBUG ((DEBUG_INFO, "%a, %a.%a apply %s from 0x%x to 0x%x\n", __FUNCTION__, Schema, Version, ConfigureLang, RedfishValue.Value.Integer, FeatureValue));

      RedfishValue.Value.Integer = (INT64)FeatureValue;

      Status = RedfishPlatformConfigSetValue (Schema, Version, ConfigureLang, RedfishValue);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply %s to 0x%x failed: %r\n", __FUNCTION__, ConfigureLang, FeatureValue, Status));
      }
    } else {
      DEBUG ((DEBUG_ERROR, "%a, %a.%a %s value is: 0x%x\n", __FUNCTION__, Schema, Version, ConfigureLang, RedfishValue.Value.Integer, Status));
    }
  }

  return Status;
}

/**

  Apply property value to UEFI HII database in boolean type.

  @param[in]  Schema        Property schema.
  @param[in]  Version       Property schema version.
  @param[in]  ConfigureLang Configure language refers to this property.
  @param[in]  FeatureValue  New value to set.

  @retval     EFI_SUCCESS     New value is applied successfully.
  @retval     Others          Errors occur.

**/
EFI_STATUS
ApplyFeatureSettingsBooleanType (
  IN  CHAR8      *Schema,
  IN  CHAR8      *Version,
  IN  EFI_STRING ConfigureLang,
  IN  BOOLEAN    FeatureValue
  )
{
  EFI_STATUS          Status;
  EDKII_REDFISH_VALUE RedfishValue;

  if (IS_EMPTY_STRING (Schema) || IS_EMPTY_STRING (Version) || IS_EMPTY_STRING (ConfigureLang)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get the current value from HII
  //
  Status = RedfishPlatformConfigGetValue (Schema, Version, ConfigureLang, &RedfishValue);
  if (EFI_ERROR (Status)) {
   DEBUG ((DEBUG_ERROR, "%a, %a.%a %s failed: %r\n", __FUNCTION__, Schema, Version, ConfigureLang, Status));
  } else {

    if (RedfishValue.Type != REDFISH_VALUE_TYPE_BOOLEAN) {
       DEBUG ((DEBUG_ERROR, "%a, %a.%a %s value is not boolean type\n", __FUNCTION__, Schema, Version, ConfigureLang));
      return EFI_DEVICE_ERROR;
    }

    if (RedfishValue.Value.Boolean != FeatureValue) {
      //
      // Apply settings from redfish
      //
      DEBUG ((DEBUG_INFO, "%a, %a.%a apply %s from %a to %a\n", __FUNCTION__, Schema, Version, ConfigureLang, (RedfishValue.Value.Boolean ? "True" : "False"), (FeatureValue ? "True" : "False")));

      RedfishValue.Value.Boolean = FeatureValue;

      Status = RedfishPlatformConfigSetValue (Schema, Version, ConfigureLang, RedfishValue);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, apply %s to %a failed: %r\n", __FUNCTION__, ConfigureLang, (FeatureValue ? "True" : "False"), Status));
      }
    } else {
      DEBUG ((DEBUG_ERROR, "%a, %a.%a %s value is: %a\n", __FUNCTION__, Schema, Version, ConfigureLang, (RedfishValue.Value.Boolean ? "True" : "False"), Status));
    }
  }

  return Status;
}

/**

  Read redfish resource by given resource URI.

  @param[in]  Service       Redfish srvice instacne to make query.
  @param[in]  ResourceUri   Target resource URI.
  @param[out] Response      HTTP response from redfish service.

  @retval     EFI_SUCCESS     Resrouce is returned successfully.
  @retval     Others          Errors occur.

**/
EFI_STATUS
GetResourceByUri (
  IN  REDFISH_SERVICE           *Service,
  IN  EFI_STRING                ResourceUri,
  OUT REDFISH_RESPONSE          *Response
  )
{
  EFI_STATUS  Status;
  CHAR8       *AsciiResourceUri;

  if (Service == NULL || Response == NULL || IS_EMPTY_STRING (ResourceUri)) {
    return EFI_INVALID_PARAMETER;
  }

  AsciiResourceUri = StrUnicodeToAscii (ResourceUri);
  if (AsciiResourceUri == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Get resource from redfish service.
  //
  Status = RedfishGetByUri (
             Service,
             AsciiResourceUri,
             Response
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, RedfishGetByUri to %a failed: %r\n", __FUNCTION__, AsciiResourceUri, Status));
    if (Response->Payload != NULL) {
      RedfishDumpPayload (Response->Payload);
      RedfishFreeResponse (
        NULL,
        0,
        NULL,
        Response->Payload
        );
      Response->Payload = NULL;
    }
  }

  if (AsciiResourceUri != NULL) {
    FreePool (AsciiResourceUri);
  }

  return Status;
}

/**

  Find array index from given configure language string.

  @param[in]  ConfigureLang         Configure language string to parse.
  @param[out] UnifiedConfigureLang  The configure language in array.
  @param[out] Index                 The array index number.

  @retval     EFI_SUCCESS     Index is found.
  @retval     Others          Errors occur.

**/
EFI_STATUS
GetArrayIndexFromArrayTypeConfigureLang (
  IN  CHAR16 *ConfigureLang,
  OUT CHAR16 **UnifiedConfigureLang,
  OUT UINTN  *Index
  )
{
  CHAR16  *TmpConfigureLang;
  CHAR16  *IndexString;
  CHAR16  *TmpString;

  if (ConfigureLang == NULL || UnifiedConfigureLang == NULL || Index == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TmpConfigureLang = AllocateCopyPool (StrSize (ConfigureLang), ConfigureLang);
  if (TmpConfigureLang == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // looking for index signature "{""
  //
  IndexString = StrStr (TmpConfigureLang, BIOS_CONFIG_TO_REDFISH_REDPATH_ARRAY_START_SIGNATURE);
  if (IndexString == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // Skip "{"
  //
  TmpString = IndexString + StrLen (BIOS_CONFIG_TO_REDFISH_REDPATH_ARRAY_START_SIGNATURE);

  //
  // Looking for "}"
  //
  TmpString = StrStr (TmpString, BIOS_CONFIG_TO_REDFISH_REDPATH_ARRAY_END_SIGNATURE);
  if (TmpString == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // Append '\0' for converting decimal string to integer.
  //
  TmpString[0] = '\0';

  //
  // Convert decimal string to integer
  //
  *Index = StrDecimalToUintn (IndexString + StrLen (BIOS_CONFIG_TO_REDFISH_REDPATH_ARRAY_START_SIGNATURE));

  //
  // Resotre the '}' character and remove rest of string.
  //
  TmpString[0] = L'}';
  TmpString[1] = '\0';

  *UnifiedConfigureLang = TmpConfigureLang;

  return EFI_SUCCESS;
}

/**

  Search HII database with given Configure Language pattern. Data is handled and
  returned in array.

  @param[in]  Schema                    The schema to search.
  @param[in]  Version                   The schema version.
  @param[in]  Pattern                   Configure Language pattern to search.
  @param[out] UnifiedConfigureLangList  The data returned by HII database.

  @retval     EFI_SUCCESS     Data is found and returned.
  @retval     Others          Errors occur.

**/
EFI_STATUS
RedfishFeatureGetUnifiedArrayTypeConfigureLang (
  IN     CHAR8                                        *Schema,
  IN     CHAR8                                        *Version,
  IN     EFI_STRING                                   Pattern,  OPTIONAL
  OUT    REDFISH_FEATURE_ARRAY_TYPE_CONFIG_LANG_LIST  *UnifiedConfigureLangList
  )
{
  EFI_STATUS Status;
  EFI_STRING *ConfigureLangList;
  UINTN      Count;
  UINTN      Index;
  UINTN      Index2;
  UINTN      ArrayIndex;
  EFI_STRING UnifiedConfigureLang;
  BOOLEAN    Duplicated;
  REDFISH_FEATURE_ARRAY_TYPE_CONFIG_LANG UnifiedConfigureLangPool[BIOS_CONFIG_TO_REDFISH_REDPATH_POOL_SIZE];

  if (IS_EMPTY_STRING (Schema) || IS_EMPTY_STRING (Version) || UnifiedConfigureLangList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  UnifiedConfigureLangList->Count = 0;
  UnifiedConfigureLangList->List = NULL;
  ZeroMem (UnifiedConfigureLangPool, sizeof (UnifiedConfigureLangPool));

  Status = RedfishPlatformConfigGetConfigureLang (Schema, Version, Pattern, &ConfigureLangList, &Count);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, RedfishFeatureGetConfigureLangRegex failed: %r\n", __FUNCTION__, Status));
    return Status;
  }

  if (Count == 0) {
    return EFI_NOT_FOUND;
  }

  for (Index = 0; Index < Count; Index++) {
    Status = GetArrayIndexFromArrayTypeConfigureLang (ConfigureLangList[Index], &UnifiedConfigureLang, &ArrayIndex);
    if (EFI_ERROR (Status)) {
      ASSERT (FALSE);
      continue;
    }

    //
    // Check if this configure language is duplicated.
    //
    Duplicated = FALSE;
    for (Index2 = 0; Index2 < BIOS_CONFIG_TO_REDFISH_REDPATH_POOL_SIZE; Index2++) {
      if (UnifiedConfigureLangPool[Index2].ConfigureLang == NULL) {
        break;
      }

      if (StrCmp (UnifiedConfigureLangPool[Index2].ConfigureLang, UnifiedConfigureLang) == 0) {
        Duplicated = TRUE;
        break;
      }
    }

    if (Duplicated) {
      FreePool (UnifiedConfigureLang);
      continue;
    }

    if (UnifiedConfigureLangList->Count >= BIOS_CONFIG_TO_REDFISH_REDPATH_POOL_SIZE) {
      FreePool (UnifiedConfigureLang);
      Status = EFI_BUFFER_TOO_SMALL;
      break;
    }

    //
    // New configure language. Keep it in Pool
    //

    UnifiedConfigureLangPool[UnifiedConfigureLangList->Count].ConfigureLang = UnifiedConfigureLang;
    UnifiedConfigureLangPool[UnifiedConfigureLangList->Count].Index = ArrayIndex;
    ++UnifiedConfigureLangList->Count;
  }

  FreePool (ConfigureLangList);

  //
  // Prepare the result to caller.
  //
  UnifiedConfigureLangList->List = AllocateCopyPool (sizeof (REDFISH_FEATURE_ARRAY_TYPE_CONFIG_LANG) * UnifiedConfigureLangList->Count, UnifiedConfigureLangPool);

  return Status;
}

/**

  Create HTTP payload and send them to redfish service with PATCH method.

  @param[in]  Service         Redfish service.
  @param[in]  TargetPayload   Target payload
  @param[in]  Json            Data in JSON format.
  @param[out] Etag            Returned ETAG string from Redfish service.

  @retval     EFI_SUCCESS     Data is sent to redfish service successfully.
  @retval     Others          Errors occur.

**/
EFI_STATUS
CreatePayloadToPatchResource (
  IN  REDFISH_SERVICE *Service,
  IN  REDFISH_PAYLOAD *TargetPayload,
  IN  CHAR8           *Json,
  OUT CHAR8           **Etag
  )
{
  REDFISH_PAYLOAD    Payload;
  EDKII_JSON_VALUE   ResourceJsonValue;
  REDFISH_RESPONSE   PostResponse;
  EFI_STATUS         Status;
  UINTN              Index;
  EDKII_JSON_VALUE   JsonValue;
  EDKII_JSON_VALUE   OdataIdValue;
  CHAR8              *OdataIdString;

  if (Service == NULL || TargetPayload == NULL || IS_EMPTY_STRING (Json) || Etag == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ResourceJsonValue = JsonLoadString (Json, 0, NULL);
  Payload = RedfishCreatePayload (ResourceJsonValue, Service);
  if (Payload == NULL) {
    DEBUG ((DEBUG_ERROR, "%a:%d Failed to create JSON payload from JSON value!\n",__FUNCTION__, __LINE__));
    Status =  EFI_DEVICE_ERROR;
    goto EXIT_FREE_JSON_VALUE;
  }

  ZeroMem (&PostResponse, sizeof (REDFISH_RESPONSE));
  Status = RedfishPatchToPayload (TargetPayload, Payload, &PostResponse);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a:%d Failed to PATCH payload to Redfish service.\n",__FUNCTION__, __LINE__));
    goto EXIT_FREE_JSON_VALUE;
  }


  //
  // Keep etag.
  //
  *Etag = NULL;
  if (*PostResponse.StatusCode == HTTP_STATUS_200_OK) {
    if (PostResponse.HeaderCount != 0) {
      for (Index = 0; Index < PostResponse.HeaderCount; Index++) {
        if (AsciiStrnCmp (PostResponse.Headers[Index].FieldName, "ETag", 4) == 0) {
          *Etag = AllocateCopyPool (AsciiStrSize (PostResponse.Headers[Index].FieldValue), PostResponse.Headers[Index].FieldValue);
        }
      }
    } else if (PostResponse.Payload != NULL) {
      //
      // No header is returned. Search payload for location.
      //
      JsonValue = RedfishJsonInPayload (PostResponse.Payload);
      if (JsonValue != NULL) {
        OdataIdValue = JsonObjectGetValue (JsonValueGetObject (JsonValue), "@odata.etag");
        if (OdataIdValue != NULL) {
          OdataIdString = (CHAR8 *)JsonValueGetAsciiString (OdataIdValue);
          if (OdataIdString != NULL) {
            *Etag = AllocateCopyPool (AsciiStrSize (OdataIdString), OdataIdString);
          }
        }
      }
    }
  }

  RedfishFreeResponse (
    PostResponse.StatusCode,
    PostResponse.HeaderCount,
    PostResponse.Headers,
    PostResponse.Payload
    );

EXIT_FREE_JSON_VALUE:
  if (Payload != NULL) {
    RedfishCleanupPayload (Payload);
  }

  JsonValueFree (ResourceJsonValue);

  return Status;
}

/**

  Create HTTP payload and send them to redfish service with POST method.

  @param[in]  Service         Redfish service.
  @param[in]  TargetPayload   Target payload
  @param[in]  Json            Data in JSON format.
  @param[out] Location        Returned location string from Redfish service.
  @param[out] Etag            Returned ETAG string from Redfish service.

  @retval     EFI_SUCCESS     Data is sent to redfish service successfully.
  @retval     Others          Errors occur.

**/
EFI_STATUS
CreatePayloadToPostResource (
  IN  REDFISH_SERVICE *Service,
  IN  REDFISH_PAYLOAD *TargetPayload,
  IN  CHAR8           *Json,
  OUT EFI_STRING      *Location,
  OUT CHAR8           **Etag
  )
{
  REDFISH_PAYLOAD    Payload;
  EDKII_JSON_VALUE   ResourceJsonValue;
  REDFISH_RESPONSE   PostResponse;
  EFI_STATUS         Status;
  UINTN              Index;
  EDKII_JSON_VALUE   JsonValue;
  EDKII_JSON_VALUE   OdataIdValue;
  CHAR8              *OdataIdString;
  CHAR8              *AsciiLocation;
  UINTN              StringSize;

  if (Service == NULL || TargetPayload == NULL || IS_EMPTY_STRING (Json) || Location == NULL || Etag == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ResourceJsonValue = JsonLoadString (Json, 0, NULL);
  Payload = RedfishCreatePayload (ResourceJsonValue, Service);
  if (Payload == NULL) {
    DEBUG ((DEBUG_ERROR, "%a:%d Failed to create JSON payload from JSON value!\n",__FUNCTION__, __LINE__));
    Status =  EFI_DEVICE_ERROR;
    goto EXIT_FREE_JSON_VALUE;
  }

  ZeroMem (&PostResponse, sizeof (REDFISH_RESPONSE));
  Status = RedfishPostToPayload (TargetPayload, Payload, &PostResponse);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a:%d Failed to POST Attribute Registry to Redfish service.\n",__FUNCTION__, __LINE__));
    goto EXIT_FREE_JSON_VALUE;
  }

  //
  // per Redfish spec. the URL of new eresource will be returned in "Location" header.
  //
  *Location = NULL;
  AsciiLocation = NULL;
  *Etag = NULL;
  if (*PostResponse.StatusCode == HTTP_STATUS_200_OK) {
    if (PostResponse.HeaderCount != 0) {
      for (Index = 0; Index < PostResponse.HeaderCount; Index++) {
        if (AsciiStrnCmp (PostResponse.Headers[Index].FieldName, "Location", 8) == 0) {
          AsciiLocation = AllocateCopyPool (AsciiStrSize (PostResponse.Headers[Index].FieldValue), PostResponse.Headers[Index].FieldValue);
        } else if (AsciiStrnCmp (PostResponse.Headers[Index].FieldName, "ETag", 4) == 0) {
          *Etag = AllocateCopyPool (AsciiStrSize (PostResponse.Headers[Index].FieldValue), PostResponse.Headers[Index].FieldValue);
        }
      }
    } else if (PostResponse.Payload != NULL) {
      //
      // No header is returned. Search payload for location.
      //
      JsonValue = RedfishJsonInPayload (PostResponse.Payload);
      if (JsonValue != NULL) {
        OdataIdValue = JsonObjectGetValue (JsonValueGetObject (JsonValue), "@odata.id");
        if (OdataIdValue != NULL) {
          OdataIdString = (CHAR8 *)JsonValueGetAsciiString (OdataIdValue);
          if (OdataIdString != NULL) {
            AsciiLocation = AllocateCopyPool (AsciiStrSize (OdataIdString), OdataIdString);
          }
        }

        OdataIdValue = JsonObjectGetValue (JsonValueGetObject (JsonValue), "@odata.etag");
        if (OdataIdValue != NULL) {
          OdataIdString = (CHAR8 *)JsonValueGetAsciiString (OdataIdValue);
          if (OdataIdString != NULL) {
            *Etag = AllocateCopyPool (AsciiStrSize (OdataIdString), OdataIdString);
          }
        }
      }
    }
  }

  //
  // This is not expected as service does not follow spec.
  //
  if (AsciiLocation == NULL) {
    Status = EFI_DEVICE_ERROR;
  }

  RedfishFreeResponse (
    PostResponse.StatusCode,
    PostResponse.HeaderCount,
    PostResponse.Headers,
    PostResponse.Payload
    );

  RedfishCleanupPayload (Payload);

EXIT_FREE_JSON_VALUE:
  JsonValueFree (JsonValue);
  JsonValueFree (ResourceJsonValue);

  if (AsciiLocation != NULL) {
    StringSize = (AsciiStrLen (AsciiLocation) + 1);
    *Location = AllocatePool (StringSize * sizeof (CHAR16));
    if (*Location == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      Status = AsciiStrToUnicodeStrS (AsciiLocation, *Location, StringSize);
    }

    FreePool (AsciiLocation);
  }

  return Status;
}

/**

  Return redfish URI by given config language. It's call responsibility to release returned buffer.

  @param[in]  ConfigLang    ConfigLang to search.

  @retval  NULL     Can not find redfish uri.
  @retval  Other    redfish uri is returned.

**/
EFI_STRING
RedfishGetUri (
  IN  EFI_STRING ConfigLang
  )
{
  EFI_STATUS Status;
  EFI_STRING Target;
  EFI_STRING Found;
  EFI_STRING TempStr;
  EFI_STRING ResultStr;
  EFI_STRING Head;
  EFI_STRING CloseBracket;
  UINTN      TempStrSize;
  UINTN      RemainingLen;
  UINTN      ConfigLangLen;

  Status = RedfishLocateProtocol ((VOID **)&mConfigLangMapProtocol, &gEdkIIRedfishConfigLangMapProtocolGuid);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, fail to locate gEdkIIRedfishConfigLangMapProtocolGuid: %r\n", __FUNCTION__, Status));
    return NULL;
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a, Get: %s\n", __FUNCTION__, ConfigLang));

  CloseBracket = StrStr (ConfigLang, L"{");
  if (CloseBracket == NULL) {
    return AllocateCopyPool (StrSize (ConfigLang), ConfigLang);
  }

  //
  // Remove leading "/v1" or "/redfish/v1" because we don't code
  // configure language in this way.
  //
  Head = StrStr (ConfigLang, REDFISH_ROOT_PATH_UNICODE);
  if (Head == NULL) {
    Head = ConfigLang;
  } else {
    Head += 3;
  }

  ResultStr = AllocateZeroPool (sizeof (CHAR16) * MAX_REDFISH_URL_LEN);
  if (ResultStr == NULL) {
    return NULL;
  }

  //
  // Go though ConfigLang and replace each {} with URL
  //
  do {
    ConfigLangLen = StrLen (Head);
    Target = CloseBracket;

    //
    // Look for next ConfigLang
    //
    do {
      Target += 1;
    } while (*Target != '\0' && *Target != '}');

    //
    // Invalid format. No '}' found
    //
    if (*Target == '\0') {
      DEBUG ((DEBUG_ERROR, "%a, invalid format: %s\n", __FUNCTION__, ConfigLang));
      return NULL;
    }

    //
    // Copy current ConfigLang to temporary string and do a query
    //
    Target += 1;
    RemainingLen = StrLen (Target);
    TempStrSize = (ConfigLangLen - RemainingLen + 1) * sizeof (CHAR16);
    TempStr = AllocateCopyPool (TempStrSize, Head);
    if (TempStr == NULL) {
      return NULL;
    }
    TempStr[ConfigLangLen - RemainingLen] = '\0';

    Status = mConfigLangMapProtocol->Get (
                                       mConfigLangMapProtocol,
                                       RedfishGetTypeConfigLang,
                                       TempStr,
                                       &Found
                                       );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a, Can not find: %s\n", __FUNCTION__, TempStr));
      return NULL;
    }

    DEBUG ((REDFISH_DEBUG_TRACE, "%a, Found: %s\n", __FUNCTION__, Found));

    //
    // Keep result in final string pool
    //
    StrCatS (ResultStr, MAX_REDFISH_URL_LEN, Found);
    FreePool (TempStr);

    //
    // Prepare for next ConfigLang
    //
    Head = Target;
    CloseBracket = StrStr (Head, L"{");
  } while (CloseBracket != NULL);

  //
  // String which has no ConfigLang remaining
  //
  if (Head != '\0') {
    StrCatS (ResultStr, MAX_REDFISH_URL_LEN, Head);
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a, return: %s\n", __FUNCTION__, ResultStr));

  return ResultStr;
}

/**

  Return config language by given URI. It's call responsibility to release returned buffer.

  @param[in]  Uri   Uri to search.

  @retval  NULL     Can not find redfish uri.
  @retval  Other    redfish uri is returned.

**/
EFI_STRING
RedfishGetConfigLanguage (
  IN  EFI_STRING Uri
  )
{
  EFI_STATUS  Status;
  EFI_STRING  ConfigLang;

  if (IS_EMPTY_STRING (Uri)) {
    return NULL;
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a, search config lang for URI: %s\n", __FUNCTION__, Uri));

  Status = RedfishLocateProtocol ((VOID **)&mConfigLangMapProtocol, &gEdkIIRedfishConfigLangMapProtocolGuid);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, fail to locate gEdkIIRedfishConfigLangMapProtocolGuid: %r\n", __FUNCTION__, Status));
    return NULL;
  }

  ConfigLang = NULL;
  Status = mConfigLangMapProtocol->Get (
                                     mConfigLangMapProtocol,
                                     RedfishGetTypeUri,
                                     Uri,
                                     &ConfigLang
                                     );


  return ConfigLang;
}

/**

  Return config language from given URI and prperty name. It's call responsibility to release returned buffer.

  @param[in] Uri            The URI to match
  @param[in] PropertyName   The property name of resource. This is optional.

  @retval  NULL     Can not find redfish uri.
  @retval  Other    redfish uri is returned.

**/
EFI_STRING
GetConfigureLang (
  IN  CHAR8 *Uri,
  IN  CHAR8 *PropertyName   OPTIONAL
  )
{
  EFI_STRING  ConfigLang;
  UINTN       StringSize;
  EFI_STRING  ResultStr;
  EFI_STRING  UnicodeUri;
  EFI_STATUS  Status;

  if (IS_EMPTY_STRING (Uri)) {
    return NULL;
  }

  StringSize = AsciiStrSize (Uri);
  UnicodeUri = AllocatePool (StringSize * sizeof (CHAR16));
  if (UnicodeUri == NULL) {
    return NULL;
  }

  Status = AsciiStrToUnicodeStrS (Uri, UnicodeUri, StringSize);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  ConfigLang = RedfishGetConfigLanguage (UnicodeUri);
  if (ConfigLang == NULL) {
    return NULL;
  }

  if (IS_EMPTY_STRING (PropertyName)) {
    return ConfigLang;
  }

  StringSize = StrSize (ConfigLang) + (AsciiStrLen (PropertyName) * sizeof (CHAR16));
  ResultStr = AllocatePool (StringSize);
  if (ResultStr == NULL) {
    return NULL;
  }

  UnicodeSPrint (ResultStr, StringSize, L"%s/%a", ConfigLang, PropertyName);

  return ResultStr;
}

/**

  Save Redfish URI in database for further use.

  @param[in]    ConfigLang        ConfigLang to save
  @param[in]    Uri               Redfish Uri to save

  @retval  EFI_INVALID_PARAMETR   SystemId is NULL or EMPTY
  @retval  EFI_SUCCESS            Redfish uri is saved

**/
EFI_STATUS
RedfisSetRedfishUri (
  IN    EFI_STRING  ConfigLang,
  IN    EFI_STRING  Uri
  )
{
  EFI_STATUS Status;

  if (IS_EMPTY_STRING (ConfigLang) || IS_EMPTY_STRING (Uri)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = RedfishLocateProtocol ((VOID **)&mConfigLangMapProtocol, &gEdkIIRedfishConfigLangMapProtocolGuid);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, fail to locate gEdkIIRedfishConfigLangMapProtocolGuid: %r\n", __FUNCTION__, Status));
    return Status;
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a, Saved: %s -> %s\n", __FUNCTION__, ConfigLang, Uri));

  return mConfigLangMapProtocol->Set (mConfigLangMapProtocol, ConfigLang, Uri);
}

/**

  Get @odata.id from give HTTP payload. It's call responsibility to release returned buffer.

  @param[in]  Payload             HTTP payload

  @retval     NULL                Can not find @odata.id from given payload.
  @retval     Others              odata.id string is returned.

**/
EFI_STRING
GetOdataId (
  IN  REDFISH_PAYLOAD *Payload
  )
{
  EDKII_JSON_VALUE *JsonValue;
  EDKII_JSON_VALUE *OdataId;
  EFI_STRING       OdataIdString;

  if (Payload == NULL) {
    return NULL;
  }

  JsonValue = RedfishJsonInPayload (Payload);
  if (!JsonValueIsObject (JsonValue)) {
    return NULL;
  }

  OdataId = JsonObjectGetValue (JsonValueGetObject (JsonValue), "@odata.id");
  if (!JsonValueIsString (OdataId)) {
    return NULL;
  }

  OdataIdString = JsonValueGetUnicodeString (OdataId);
  if (OdataIdString == NULL) {
    return NULL;
  }

  return AllocateCopyPool (StrSize (OdataIdString), OdataIdString);
}


/**

  Get the property name by given Configure Langauge.

  @param[in]  ConfigureLang   Configure Language string.

  @retval     EFI_STRING      Pointer to property name.
  @retval     NULL            There is error.

**/
EFI_STRING
GetPropertyFromConfigureLang (
  IN EFI_STRING ConfigureLang
  )
{
  EFI_STRING Property;
  UINTN      Index;

  if (ConfigureLang == NULL) {
    return NULL;
  }

  Index = 0;
  Property = ConfigureLang;

  while (ConfigureLang[Index] != '\0') {
    if (ConfigureLang[Index] == L'/') {
      Property = &ConfigureLang[Index];
    }

    ++Index;
  }

  ++Property;

  return Property;
}

/**

  Get the property value in string type.

  @param[in]  Schema        Schema of this property.
  @param[in]  Version       Schema version.
  @param[in]  PropertyName  Property name.
  @param[in]  ConfigureLang Configure Language of this property.

  @retval     CHAR8*        Pointer to the CHAR8 buffer.
  @retval     NULL          There is error.

**/
CHAR8 *
GetPropertyStringValue (
  IN CHAR8      *Schema,
  IN CHAR8      *Version,
  IN EFI_STRING PropertyName,
  IN EFI_STRING ConfigureLang
  )
{
  EFI_STATUS          Status;
  EDKII_REDFISH_VALUE RedfishValue;
  EFI_STRING          ConfigureLangBuffer;
  UINTN               BufferSize;
  CHAR8               *AsciiStringValue;

  if (IS_EMPTY_STRING (Schema) || IS_EMPTY_STRING (Version) || IS_EMPTY_STRING (ConfigureLang) || IS_EMPTY_STRING (PropertyName)) {
    return NULL;
  }

  //
  // Configure Language buffer.
  //
  BufferSize = sizeof (CHAR16) * MAX_CONF_LANG_LEN;
  ConfigureLangBuffer = AllocatePool (BufferSize);
  if (ConfigureLangBuffer == NULL) {
    return NULL;
  }

  UnicodeSPrint (ConfigureLangBuffer, BufferSize, L"%s/%s", ConfigureLang, PropertyName);
  Status = RedfishPlatformConfigGetValue (Schema, Version, ConfigureLangBuffer, &RedfishValue);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, %a.%a query current setting for %s failed: %r\n", __FUNCTION__, Schema, Version, ConfigureLangBuffer, Status));
    return NULL;
  }

  if (RedfishValue.Type != REDFISH_VALUE_TYPE_STRING) {
    DEBUG ((DEBUG_ERROR, "%a, %a.%a %s value is not string type\n", __FUNCTION__, Schema, Version, ConfigureLang));
    return NULL;
  }

  AsciiStringValue = AllocateCopyPool (AsciiStrSize (RedfishValue.Value.Buffer), RedfishValue.Value.Buffer);
  ASSERT (AsciiStringValue != NULL);

  return AsciiStringValue;
}

/**

  Get the property value in numeric type.

  @param[in]  Schema        Schema of this property.
  @param[in]  Version       Schema version.
  @param[in]  PropertyName  Property name.
  @param[in]  ConfigureLang Configure Language of this property.

  @retval     INT64*        Pointer to the INT64 value.
  @retval     NULL          There is error.

**/
INT64 *
GetPropertyNumericValue (
  IN CHAR8      *Schema,
  IN CHAR8      *Version,
  IN EFI_STRING PropertyName,
  IN EFI_STRING ConfigureLang
  )
{
  EFI_STATUS          Status;
  EDKII_REDFISH_VALUE RedfishValue;
  EFI_STRING          ConfigureLangBuffer;
  UINTN               BufferSize;
  INT64               *ResultValue;

  if (IS_EMPTY_STRING (Schema) || IS_EMPTY_STRING (Version) || IS_EMPTY_STRING (ConfigureLang) || IS_EMPTY_STRING (PropertyName)) {
    return NULL;
  }

  //
  // Configure Language buffer.
  //
  BufferSize = sizeof (CHAR16) * MAX_CONF_LANG_LEN;
  ConfigureLangBuffer = AllocatePool (BufferSize);
  if (ConfigureLangBuffer == NULL) {
    return NULL;
  }

  UnicodeSPrint (ConfigureLangBuffer, BufferSize, L"%s/%s", ConfigureLang, PropertyName);
  Status = RedfishPlatformConfigGetValue (Schema, Version, ConfigureLangBuffer, &RedfishValue);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, %a.%a query current setting for %s failed: %r\n", __FUNCTION__, Schema, Version, ConfigureLangBuffer, Status));
    return NULL;
  }

  if (RedfishValue.Type != REDFISH_VALUE_TYPE_INTEGER) {
    DEBUG ((DEBUG_ERROR, "%a, %a.%a %s value is not numeric type\n", __FUNCTION__, Schema, Version, ConfigureLang));
    return NULL;
  }

  ResultValue = AllocatePool (sizeof (INT64));
  ASSERT (ResultValue != NULL);
  if (ResultValue == NULL) {
    return NULL;
  }

  *ResultValue = RedfishValue.Value.Integer;

  return ResultValue;
}

/**

  Get the property value in Boolean type.

  @param[in]  Schema        Schema of this property.
  @param[in]  Version       Schema version.
  @param[in]  PropertyName  Property name.
  @param[in]  ConfigureLang Configure Language of this property.

  @retval     BOOLEAN       Boolean value returned by this property.

**/
BOOLEAN *
GetPropertyBooleanValue (
  IN CHAR8      *Schema,
  IN CHAR8      *Version,
  IN EFI_STRING PropertyName,
  IN EFI_STRING ConfigureLang
  )
{
  EFI_STATUS          Status;
  EDKII_REDFISH_VALUE RedfishValue;
  EFI_STRING          ConfigureLangBuffer;
  UINTN               BufferSize;
  BOOLEAN             *ResultValue;

  if (IS_EMPTY_STRING (Schema) || IS_EMPTY_STRING (Version) || IS_EMPTY_STRING (ConfigureLang) || IS_EMPTY_STRING (PropertyName)) {
    return NULL;
  }

  //
  // Configure Language buffer.
  //
  BufferSize = sizeof (CHAR16) * MAX_CONF_LANG_LEN;
  ConfigureLangBuffer = AllocatePool (BufferSize);
  if (ConfigureLangBuffer == NULL) {
    return NULL;
  }

  UnicodeSPrint (ConfigureLangBuffer, BufferSize, L"%s/%s", ConfigureLang, PropertyName);
  Status = RedfishPlatformConfigGetValue (Schema, Version, ConfigureLangBuffer, &RedfishValue);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, %a.%a query current setting for %s failed: %r\n", __FUNCTION__, Schema, Version, ConfigureLangBuffer, Status));
    return NULL;
  }

  if (RedfishValue.Type != REDFISH_VALUE_TYPE_BOOLEAN) {
    DEBUG ((DEBUG_ERROR, "%a, %a.%a %s value is not boolean type\n", __FUNCTION__, Schema, Version, ConfigureLang));
    return NULL;
  }

  ResultValue = AllocatePool (sizeof (BOOLEAN));
  ASSERT (ResultValue != NULL);
  if (ResultValue == NULL) {
    return NULL;
  }

  *ResultValue = RedfishValue.Value.Boolean;

  return ResultValue;
}

/**

  Check and see if we need to do provisioning for this property.

  @param[in]  PropertyBuffer   Pointer to property instance.
  @param[in]  ProvisionMode    TRUE if we are in provision mode. FALSE otherwise.

  @retval     TRUE             Provision is required.
  @retval     FALSE            Provision is not required.

**/
BOOLEAN
PropertyChecker (
  IN VOID         *PropertyBuffer,
  IN BOOLEAN      ProvisionMode
  )
{
  if (ProvisionMode && PropertyBuffer == NULL) {
    return TRUE;
  }

  if (!ProvisionMode && PropertyBuffer != NULL) {
    return TRUE;
  }

  return FALSE;
}

/**

  Check and see if we need to do provisioning for this two properties.

  @param[in]  PropertyBuffer1   Pointer to property instance 1.
  @param[in]  PropertyBuffer2   Pointer to property instance 2.
  @param[in]  ProvisionMode     TRUE if we are in provision mode. FALSE otherwise.

  @retval     TRUE             Provision is required.
  @retval     FALSE            Provision is not required.

**/
BOOLEAN
PropertyChecker2Parm (
  IN VOID         *PropertyBuffer1,
  IN VOID         *PropertyBuffer2,
  IN BOOLEAN      ProvisionMode
  )
{
  if (ProvisionMode && (PropertyBuffer1 == NULL || PropertyBuffer2 == NULL)) {
    return TRUE;
  }

  if (!ProvisionMode && PropertyBuffer1 != NULL && PropertyBuffer2 != NULL) {
    return TRUE;
  }

  return FALSE;
}

/**

  Check and see if ETAG is identical to what we keep in system.

  @param[in]  Uri           URI requested
  @param[in]  EtagInHeader  ETAG string returned from HTTP request.
  @param[in]  EtagInJson    ETAG string in JSON body.

  @retval     TRUE          ETAG is identical.
  @retval     FALSE         ETAG is changed.

**/
BOOLEAN
CheckEtag (
  IN EFI_STRING Uri,
  IN CHAR8      *EtagInHeader,
  IN CHAR8      *EtagInJson
  )
{
  CHAR8 *EtagInDb;

  if (IS_EMPTY_STRING (Uri)) {
    return FALSE;
  }

  if (IS_EMPTY_STRING (EtagInHeader) && IS_EMPTY_STRING (EtagInJson)) {
    return FALSE;
  }

  //
  // Check ETAG to see if we need to consume it
  //
  EtagInDb = NULL;
  EtagInDb = GetEtagWithUri (Uri);
  if (EtagInDb == NULL) {
    DEBUG ((REDFISH_DEBUG_TRACE, "%a, no ETAG record cound be found for: %s\n", __FUNCTION__, Uri));
    return FALSE;
  }

  if (EtagInHeader != NULL) {
    if (AsciiStrCmp (EtagInDb, EtagInHeader) == 0) {
      FreePool (EtagInDb);
      return TRUE;
    }
  }

  if (EtagInJson != NULL) {
    if (AsciiStrCmp (EtagInDb, EtagInJson) == 0) {
      FreePool (EtagInDb);
      return TRUE;
    }
  }

  FreePool (EtagInDb);

  return FALSE;
}

/**

  Install Boot Maintenance Manager Menu driver.

  @param[in] ImageHandle     The image handle.
  @param[in] SystemTable     The system table.

  @retval  EFI_SUCEESS  Install Boot manager menu success.
  @retval  Other        Return error status.

**/
EFI_STATUS
EFIAPI
RedfishFeatureUtilityLibConstructor (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{

  return EFI_SUCCESS;
}

/**
  Unloads the application and its installed protocol.

  @param[in] ImageHandle       Handle that identifies the image to be unloaded.
  @param[in] SystemTable      The system table.

  @retval EFI_SUCCESS      The image has been unloaded.

**/
EFI_STATUS
EFIAPI
RedfishFeatureUtilityLibDestructor (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  return EFI_SUCCESS;
}
