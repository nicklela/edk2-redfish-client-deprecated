/** @file
  This file defines the Redfish Feature Utility Library interface.

  (C) Copyright 2021-2022 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef REDFISH_FEATURE_UTILITY_LIB_H_
#define REDFISH_FEATURE_UTILITY_LIB_H_

#include <Protocol/EdkIIRedfishPlatformConfig.h>

//
// Definition of REDFISH_FEATURE_ARRAY_TYPE_CONFIG_LANG
//
typedef struct {
  UINTN       Index;
  EFI_STRING  ConfigureLang;
} REDFISH_FEATURE_ARRAY_TYPE_CONFIG_LANG;

//
// Definition of REDFISH_FEATURE_ARRAY_TYPE_CONFIG_LANG_LIST
//
typedef struct {
  UINTN                                   Count;
  REDFISH_FEATURE_ARRAY_TYPE_CONFIG_LANG  *List;
} REDFISH_FEATURE_ARRAY_TYPE_CONFIG_LANG_LIST;

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
  );

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
  IN     EFI_STRING                                   Pattern,
  OUT    REDFISH_FEATURE_ARRAY_TYPE_CONFIG_LANG_LIST  *UnifiedConfigureLangList
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**

  Get the property name by given Configure Langauge.

  @param[in]  ConfigureLang   Configure Language string.

  @retval     EFI_STRING      Pointer to property name.
  @retval     NULL            There is error.

**/
EFI_STRING
GetPropertyFromConfigureLang (
  IN EFI_STRING ConfigureLang
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**

  Find ETAG string that refers to given URI.

  @param[in]  Uri       Target URI string.

  @retval     CHAR8 *   ETAG string
  @retval     NULL      No ETAG is found.

**/
CHAR8 *
GetEtagWithUri (
  IN  EFI_STRING  Uri
  );

/**

  Get @odata.id from give HTTP payload. It's call responsibility to release returned buffer.

  @param[in]  Payload             HTTP payload

  @retval     NULL                Can not find @odata.id from given payload.
  @retval     Others              odata.id string is returned.

**/
EFI_STRING
GetOdataId (
  IN  REDFISH_PAYLOAD *Payload
  );

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
  );

/**

  Return redfish URI by given config language. It's call responsibility to release returned buffer.

  @param[in]  ConfigLang    ConfigLang to search.

  @retval  NULL     Can not find redfish uri.
  @retval  Other    redfish uri is returned.

**/
EFI_STRING
RedfishGetUri (
  IN  EFI_STRING ConfigLang
  );

/**

  Return config language by given URI. It's call responsibility to release returned buffer.

  @param[in]  Uri   Uri to search.

  @retval  NULL     Can not find redfish uri.
  @retval  Other    redfish uri is returned.

**/
EFI_STRING
RedfishGetConfigLanguage (
  IN  EFI_STRING Uri
  );

/**

  Convert Unicode string to ASCII string. It's call responsibility to release returned buffer.

  @param[in]  UnicodeStr      Unicode string to convert.

  @retval     CHAR8 *         ASCII string returned.
  @retval     NULL            Errors occur.

**/
CHAR8 *
StrUnicodeToAscii (
  IN EFI_STRING   UnicodeStr
  );

/**

  Convert ASCII string to Unicode string. It's call responsibility to release returned buffer.

  @param[in]  AsciiStr        ASCII string to convert.

  @retval     EFI_STRING      Unicode string returned.
  @retval     NULL            Errors occur.

**/
EFI_STRING
StrAsciiToUnicode (
  IN CHAR8  *AsciiStr
  );

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
  );

/**

  Get the property value in array type.

  @param[in]  Schema        Schema of this property.
  @param[in]  Version       Schema version.
  @param[in]  PropertyName  Property name.
  @param[in]  ConfigureLang Configure Language of this property.
  @param[out] ArraySize     The size of returned array.

  @retval     CHAR8 **      Returned string array. NULL while error happens.

**/
CHAR8 **
GetPropertyArrayValue (
  IN  CHAR8               *Schema,
  IN  CHAR8               *Version,
  IN  EFI_STRING          PropertyName,
  IN  EFI_STRING          ConfigureLang,
  OUT UINTN               *ArraySize
  );

/**

  Check and see if given property is in JSON context or not

  @param[in]  Property      Property name string
  @param[in]  Json          The JSON context to search.

  @retval     TRUE          Property is found in JSON context
  @retval     FALSE         Property is not in JSON context

**/
BOOLEAN
MatchPropertyWithJsonContext (
  IN  EFI_STRING  Property,
  IN  CHAR8       *Json
);

#endif
