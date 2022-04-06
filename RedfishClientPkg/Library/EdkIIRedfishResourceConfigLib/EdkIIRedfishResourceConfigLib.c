/** @file
  Redfish resource config library implementation

  (C) Copyright 2022 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <RedfishBase.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/EdkIIRedfishResourceConfigLib.h>
#include <Library/RedfishFeatureUtilityLib.h>
#include <Library/RedfishPlatformConfigLib.h>

EDKII_REDFISH_RESOURCE_CONFIG_PROTOCOL *mRedfishResourceConfigProtocol = NULL;
REDFISH_SCHEMA_INFO                    mSchemaInfoCache;

#define SCHEMA_NAME_PREFIX_OFFSET 15 // x-uefi-redfish-

/**

  Get schema information by given protocol and service instance.

  @param[in]  RedfishService      Pointer to Redfish service instance.
  @param[in]  JsonStructProtocol  Json Structure protocol instance.
  @param[in]  Uri                 Target URI.
  @param[out] SchemaInfo          Returned schema information.

  @retval     EFI_SUCCESS         Schema information is returned successfully.
  @retval     Others              Errors occur.

**/
EFI_STATUS
GetRedfishSchemaInfo (
  IN  REDFISH_SERVICE                   *RedfishService,
  IN  EFI_REST_JSON_STRUCTURE_PROTOCOL  *JsonStructProtocol,
  IN  EFI_STRING                        Uri,
  OUT REDFISH_SCHEMA_INFO               *SchemaInfo
  )
{
  EFI_STATUS                      Status;
  REDFISH_RESPONSE                Response;
  REDFISH_PAYLOAD                 Payload;
  CHAR8                           *JsonText;
  EFI_REST_JSON_STRUCTURE_HEADER  *Header;

  if (RedfishService == NULL || JsonStructProtocol == NULL || IS_EMPTY_STRING (Uri) || SchemaInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetResourceByUri (RedfishService, Uri, &Response);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, failed to get resource from %s: %r", __FUNCTION__, Uri, Status));
    return Status;
  }

  Payload = Response.Payload;
  ASSERT (Payload != NULL);

  JsonText = JsonDumpString (RedfishJsonInPayload (Payload), EDKII_JSON_COMPACT);
  ASSERT (JsonText != NULL);

  //
  // Convert JSON text to C structure.
  //
  Status = JsonStructProtocol->ToStructure (
                                 JsonStructProtocol,
                                 NULL,
                                 JsonText,
                                 &Header
                                 );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, ToStructure() failed: %r\n", __FUNCTION__, Status));
    return Status;
  }

  AsciiStrCpyS (SchemaInfo->Schema, REDFISH_SCHEMA_STRING_SIZE,  Header->JsonRsrcIdentifier.NameSpace.ResourceTypeName);
  AsciiStrCpyS (SchemaInfo->Major, REDFISH_SCHEMA_VERSION_SIZE,  Header->JsonRsrcIdentifier.NameSpace.MajorVersion);
  AsciiStrCpyS (SchemaInfo->Minor, REDFISH_SCHEMA_VERSION_SIZE,  Header->JsonRsrcIdentifier.NameSpace.MinorVersion);
  AsciiStrCpyS (SchemaInfo->Errata, REDFISH_SCHEMA_VERSION_SIZE,  Header->JsonRsrcIdentifier.NameSpace.ErrataVersion);

  //
  // Release resource.
  //
  JsonStructProtocol->DestoryStructure (JsonStructProtocol, Header);
  FreePool (JsonText);
  RedfishFreeResponse (Response.StatusCode, Response.HeaderCount, Response.Headers, Response.Payload);

  return EFI_SUCCESS;
}

/**

  Get supported schema list by given specify schema name.

  @param[in]  Schema      Schema type name.
  @param[out] SchemaInfo  Returned schema information.

  @retval     EFI_SUCCESS         Schema information is returned successfully.
  @retval     Others              Errors occur.

**/
EFI_STATUS
GetSupportedSchemaVersion (
  IN   CHAR8                *Schema,
  OUT  REDFISH_SCHEMA_INFO  *SchemaInfo
  )
{
  EFI_STATUS  Status;
  CHAR8       *SupportSchema;
  CHAR8       *SchemaName;
  UINTN       Index;
  UINTN       Index2;
  BOOLEAN     Found;

  if (IS_EMPTY_STRING (Schema) || SchemaInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = RedfishPlatformConfigGetSupportedSchema (NULL, &SupportSchema);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Supported schema: %a\n", SupportSchema));

  Index = 0;
  Found = FALSE;
  SchemaName = SupportSchema;
  while (TRUE) {

    if (SupportSchema[Index] == ';' || SupportSchema[Index] == '\0') {
      if (AsciiStrnCmp (&SchemaName[SCHEMA_NAME_PREFIX_OFFSET], Schema, AsciiStrLen (Schema)) == 0) {
        Found = TRUE;
        SupportSchema[Index] = '\0';
        break;
      }

      SchemaName = &SupportSchema[Index + 1];
    }

    if (SupportSchema[Index] == '\0') {
      break;
    }

    ++Index;
  }

  if (Found) {

    AsciiStrCpyS (SchemaInfo->Schema, REDFISH_SCHEMA_STRING_SIZE, Schema);

    //
    // forward to '.'
    //
    Index = 0;
    while (SchemaName[Index] != '\0' && SchemaName[Index] != '.') {
      ++Index;
    }
    ASSERT (SchemaName[Index] != '\0');

    //
    // Skip '.' and 'v'
    //
    Index += 2;

    //
    // forward to '_'
    //
    Index2 = Index;
     while (SchemaName[Index2] != '\0' && SchemaName[Index2] != '_') {
      ++Index2;
    }
    ASSERT (SchemaName[Index2] != '\0');

    AsciiStrnCpyS (SchemaInfo->Major, REDFISH_SCHEMA_VERSION_SIZE, &SchemaName[Index], (Index2 - Index));
    Index = Index2;

    //
    // Skip '_'
    //
    ++Index;

    //
    // forward to '_'
    //
    Index2 = Index;
     while (SchemaName[Index2] != '\0' && SchemaName[Index2] != '_') {
      ++Index2;
    }
    ASSERT (SchemaName[Index2] != '\0');

    AsciiStrnCpyS (SchemaInfo->Minor, REDFISH_SCHEMA_VERSION_SIZE, &SchemaName[Index], (Index2 - Index));
    Index = Index2;

     //
    // Skip '_'
    //
    ++Index;

    AsciiStrCpyS (SchemaInfo->Errata, REDFISH_SCHEMA_VERSION_SIZE, &SchemaName[Index]);
  }

  FreePool (SupportSchema);

  return (Found ? EFI_SUCCESS : EFI_NOT_FOUND);
}


/**

  Find Redfish Resource Config Protocol that supports given schema and version.

  @param[in]  Schema      Schema name.
  @param[in]  Major       Schema version major number.
  @param[in]  Minor       Schema version minor number.
  @param[in]  Errata      Schema version errata number.

  @retval     EDKII_REDFISH_RESOURCE_CONFIG_PROTOCOL *    Pointer to protocol
  @retval     NULL                                        No protocol found.

**/
EDKII_REDFISH_RESOURCE_CONFIG_PROTOCOL  *
GetRedfishResourceConfigProtocol (
  IN  REDFISH_SCHEMA_INFO *Schema
  )
{
  EFI_STATUS                              Status;
  EFI_HANDLE                              *HandleBuffer;
  UINTN                                   NumberOfHandles;
  UINTN                                   Index;
  EDKII_REDFISH_RESOURCE_CONFIG_PROTOCOL  *Protocol;
  REDFISH_SCHEMA_INFO                     SchemaInfo;
  BOOLEAN                                 Found;

  if (IS_EMPTY_STRING (Schema->Schema) || IS_EMPTY_STRING (Schema->Major) || IS_EMPTY_STRING (Schema->Minor) || IS_EMPTY_STRING (Schema->Errata)) {
    return NULL;
  }

  if (mRedfishResourceConfigProtocol != NULL) {
    if (AsciiStrCmp (Schema->Schema, mSchemaInfoCache.Schema) == 0 &&
        AsciiStrCmp (Schema->Major, mSchemaInfoCache.Major) == 0 &&
        AsciiStrCmp (Schema->Minor, mSchemaInfoCache.Minor) == 0 &&
        AsciiStrCmp (Schema->Errata, mSchemaInfoCache.Errata) == 0) {
          return mRedfishResourceConfigProtocol;
        }
  }

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEdkIIRedfishResourceConfigProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  Found = FALSE;

  for (Index = 0; Index < NumberOfHandles; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEdkIIRedfishResourceConfigProtocolGuid,
                    (VOID **) &Protocol
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = Protocol->GetInfo (Protocol, &SchemaInfo);
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (AsciiStrCmp (Schema->Schema, SchemaInfo.Schema) == 0 &&
        AsciiStrCmp (Schema->Major, SchemaInfo.Major) == 0 &&
        AsciiStrCmp (Schema->Minor, SchemaInfo.Minor) == 0 &&
        AsciiStrCmp (Schema->Errata, SchemaInfo.Errata) == 0) {
          Found = TRUE;
          break;
        }
  }

  FreePool (HandleBuffer);

  if (Found) {
    mRedfishResourceConfigProtocol = Protocol;
    CopyMem (&mSchemaInfoCache, Schema, sizeof (REDFISH_SCHEMA_INFO));
  }

  return (Found ? Protocol : NULL);
}


/**
  Provising redfish resource by given URI.

  @param[in]   Schema              Redfish schema information.
  @param[in]   Uri                 Target URI to create resource.
  @param[in]   HttpPostMode        TRUE if resource does not exist, HTTP POST method is used.
                                   FALSE if the resource exist but some of properties are missing,
                                   HTTP PUT method is used.

  @retval EFI_SUCCESS              Value is returned successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
EdkIIRedfishResourceConfigProvisionging (
  IN     REDFISH_SCHEMA_INFO  *Schema,
  IN     EFI_STRING           Uri,
  IN     BOOLEAN              HttpPostMode
  )
{
  EDKII_REDFISH_RESOURCE_CONFIG_PROTOCOL *protocol;

  protocol = GetRedfishResourceConfigProtocol (Schema);
  if (protocol == NULL) {
    return EFI_DEVICE_ERROR;
  }

  return protocol->Provisioning (protocol, Uri, HttpPostMode);
}

/**
  Consume resource from given URI.

  @param[in]   Uri                 The target URI to consume.

  @retval EFI_SUCCESS              Value is returned successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
EdkIIRedfishResourceConfigConsume (
  IN     REDFISH_SCHEMA_INFO  *Schema,
  IN     EFI_STRING           Uri
  )
{
  EDKII_REDFISH_RESOURCE_CONFIG_PROTOCOL *protocol;

  protocol = GetRedfishResourceConfigProtocol (Schema);
  if (protocol == NULL) {
    return EFI_DEVICE_ERROR;
  }

  return protocol->Consume (protocol, Uri);
}


/**
  Update resource to given URI.

  @param[in]   Uri                 The target URI to consume.

  @retval EFI_SUCCESS              Value is returned successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
EdkIIRedfishResourceConfigUpdate (
  IN     REDFISH_SCHEMA_INFO  *Schema,
  IN     EFI_STRING           Uri
  )
{
  EDKII_REDFISH_RESOURCE_CONFIG_PROTOCOL *protocol;

  protocol = GetRedfishResourceConfigProtocol (Schema);
  if (protocol == NULL) {
    return EFI_DEVICE_ERROR;
  }

  return protocol->Update (protocol, Uri);
}

/**
  Check resource on given URI.

  @param[in]   Uri                 The target URI to consume.

  @retval EFI_SUCCESS              Value is returned successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
EdkIIRedfishResourceConfigCheck (
  IN     REDFISH_SCHEMA_INFO  *Schema,
  IN     EFI_STRING           Uri
  )
{
  EDKII_REDFISH_RESOURCE_CONFIG_PROTOCOL *protocol;

  protocol = GetRedfishResourceConfigProtocol (Schema);
  if (protocol == NULL) {
    return EFI_DEVICE_ERROR;
  }

  return protocol->Check (protocol, Uri);
}

/**
  Identify resource on given URI.

  @param[in]   Uri                 The target URI to consume.

  @retval EFI_SUCCESS              This is target resource which we want to handle.
  @retval EFI_UNSUPPORTED          This is not the target resource.
  @retval Others                   Some error happened.

**/
EFI_STATUS
EdkIIRedfishResourceConfigIdentify (
  IN     REDFISH_SCHEMA_INFO  *Schema,
  IN     EFI_STRING           Uri
  )
{
  EDKII_REDFISH_RESOURCE_CONFIG_PROTOCOL *protocol;

  protocol = GetRedfishResourceConfigProtocol (Schema);
  if (protocol == NULL) {
    return EFI_DEVICE_ERROR;
  }

  return protocol->Identify (protocol, Uri);
}

/**

  Initial resource config library instace.

  @param[in] ImageHandle     The image handle.
  @param[in] SystemTable     The system table.

  @retval  EFI_SUCEESS  Install Boot manager menu success.
  @retval  Other        Return error status.

**/
EFI_STATUS
EFIAPI
RedfishResourceConfigConstructor (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  mRedfishResourceConfigProtocol = NULL;
  ZeroMem (&mSchemaInfoCache, sizeof (REDFISH_SCHEMA_INFO));

  return EFI_SUCCESS;
}

/**
  Release allocated resource.

  @param[in] ImageHandle       Handle that identifies the image to be unloaded.
  @param[in] SystemTable      The system table.

  @retval EFI_SUCCESS      The image has been unloaded.

**/
EFI_STATUS
EFIAPI
RedfishResourceConfigDestructor (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  mRedfishResourceConfigProtocol = NULL;

  return EFI_SUCCESS;
}
