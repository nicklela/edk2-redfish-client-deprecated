/** @file
  Redfish feature driver implementation - computer system collection

  (C) Copyright 2022 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "ComputerSystemCollectionDxe.h"

REDFISH_COLLECTION_PRIVATE *mRedfishCollectionPrivate = NULL;

EFI_STATUS
HandleResource (
  IN  REDFISH_COLLECTION_PRIVATE *Private,
  IN  EFI_STRING                 Uri
  )
{
  EFI_STATUS                              Status;
  REDFISH_SCHEMA_INFO                     SchemaInfo;
  EFI_STRING                              ConfigLang;

  if (Private == NULL || IS_EMPTY_STRING (Uri)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Resource match
  //

  DEBUG ((REDFISH_DEBUG_TRACE, "%a, process resource for: %s\n", __FUNCTION__, Uri));

  Status = GetRedfishSchemaInfo (Private->RedfishService, Private->JsonStructProtocol, Uri, &SchemaInfo);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, failed to get schema information from: %a %r\n", __FUNCTION__, Uri, Status));
    return Status;
  }

  //
  // Check and see if this is target resource that we want to handle.
  // Some resource is handled by other provider so we have to make sure this first.
  //
  DEBUG ((REDFISH_DEBUG_TRACE, "%s Identify for %s\n", __FUNCTION__, Uri));
  ConfigLang = RedfishGetConfigLanguage (Uri);
  if (ConfigLang == NULL) {
    Status = EdkIIRedfishResourceConfigIdentify (&SchemaInfo, Uri);
    if (EFI_ERROR (Status)) {
      if (Status == EFI_UNSUPPORTED) {
        DEBUG ((DEBUG_INFO, "%a, \"%s\" is not handled by us\n", __FUNCTION__, Uri));
        return EFI_SUCCESS;
      }

      DEBUG ((DEBUG_ERROR, "%a, fail to identify resource: \"%s\": %r\n", __FUNCTION__, Uri, Status));
      return Status;
    }
  } else {
    DEBUG ((REDFISH_DEBUG_TRACE, "%a, history record found: %s\n", __FUNCTION__, ConfigLang));
    FreePool (ConfigLang);
  }


  //
  // Check and see if target property exist or not even when collection memeber exists.
  // If not, we sill do provision.
  //
  DEBUG ((REDFISH_DEBUG_TRACE, "%a Check for %s\n", __FUNCTION__, Uri));
  Status = EdkIIRedfishResourceConfigCheck (&SchemaInfo, Uri);
  if (EFI_ERROR (Status)) {
    //
    // The target property does not exist, do the provision to create property.
    //
    DEBUG ((REDFISH_DEBUG_TRACE, "%a provision for %s\n", __FUNCTION__, Uri));
    Status = EdkIIRedfishResourceConfigProvisionging (&SchemaInfo, Uri, FALSE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a, failed to provision with GET mode: %r\n", __FUNCTION__, Status));
    }

    return Status;
  }

  //
  // Consume first.
  //
  DEBUG ((REDFISH_DEBUG_TRACE, "%a consume for %s\n", __FUNCTION__, Uri));
  Status = EdkIIRedfishResourceConfigConsume (&SchemaInfo, Uri);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, failed to consume resoruce for: %s: %r\n", __FUNCTION__, Uri, Status));
  }

  //
  // Patch.
  //
  DEBUG ((REDFISH_DEBUG_TRACE, "%a update for %s\n", __FUNCTION__, Uri));
  Status = EdkIIRedfishResourceConfigUpdate (&SchemaInfo, Uri);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, failed to update resoruce for: %s: %r\n", __FUNCTION__, Uri, Status));
  }

  return Status;
}

EFI_STATUS
HandleCollectionResource (
  IN  REDFISH_COLLECTION_PRIVATE  *Private
  )
{
  EFI_STATUS                              Status;
  EFI_REDFISH_COMPUTERSYSTEMCOLLECTION    *ComputerSystem;
  EFI_REDFISH_COMPUTERSYSTEMCOLLECTION_CS *CollectionCs;
  RedfishCS_Link                          *List;
  RedfishCS_Header                        *Header;
  RedfishCS_Type_Uri_Data                 *UriData;
  EFI_STRING                              MemberUri;
  UINTN                                   Size;
  UINTN                                   Count;

  if (Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Private->JsonStructProtocol == NULL || Private->CollectionJson == NULL) {
    return EFI_NOT_READY;
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a, process collection for: %s\n", __FUNCTION__, Private->CollectionPath));

  //
  // Convert JSON text to C structure.
  //
  Status = Private->JsonStructProtocol->ToStructure (
                                          Private->JsonStructProtocol,
                                          NULL,
                                          Private->CollectionJson,
                                          (EFI_REST_JSON_STRUCTURE_HEADER **)&ComputerSystem
                                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, ToStructure() failed: %r\n", __FUNCTION__, Status));
    return Status;
  }

  CollectionCs = ComputerSystem->ComputerSystemCollection;

  if (*CollectionCs->Membersodata_count == 0) {
    return EFI_NOT_FOUND;
  }

  if (IsLinkEmpty (&CollectionCs->Members)) {
    return EFI_NOT_FOUND;
  }

  Count = 0;
  List = GetFirstLink (&CollectionCs->Members);
  while (TRUE) {

    Header = (RedfishCS_Header *)List;
    if (Header->ResourceType == RedfishCS_Type_Uri) {
      UriData = (RedfishCS_Type_Uri_Data *)Header;

      ++Count;
      Size = (StrLen (Private->CollectionPath) + REDFISH_MAX_COLLECTION_INDEX_LEN) * sizeof (CHAR16);
      MemberUri = AllocatePool (Size);
      ASSERT (MemberUri != NULL);
      UnicodeSPrint (MemberUri, Size, L"%s[%d]", Private->CollectionPath, Count);

      Status = HandleResource (Private, MemberUri);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a, process ComputerSystem resource: %a failed: %r\n", __FUNCTION__, UriData->Uri, Status));
      }
    }

    if (IsLinkAtEnd (&CollectionCs->Members, List)) {
      break;
    }

    List = GetNextLink (&CollectionCs->Members, List);
  }

  //
  // Release resource.
  //
  Private->JsonStructProtocol->DestoryStructure (Private->JsonStructProtocol, (EFI_REST_JSON_STRUCTURE_HEADER *)ComputerSystem);

  return EFI_SUCCESS;
}

EFI_STATUS
CreateCollectionResource (
  IN  REDFISH_COLLECTION_PRIVATE           *Private
  )
{
  EFI_STATUS           Status;
  REDFISH_SCHEMA_INFO  SchemaInfo;

  if (Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a, create resource for collection for: %s\n", __FUNCTION__, Private->CollectionPath));

  Status = GetSupportedSchemaVersion (REDFISH_SCHEMA_NAME, &SchemaInfo);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, failed to find supported schema from HII database: %r\n", __FUNCTION__, Status));
    return Status;
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a, supported schema: %a %a.%a.%a\n", __FUNCTION__, SchemaInfo.Schema, SchemaInfo.Major, SchemaInfo.Minor, SchemaInfo.Errata));

  Status = EdkIIRedfishResourceConfigProvisionging (&SchemaInfo, Private->CollectionPath, TRUE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, failed to create resoruce for: %s: %r\n", __FUNCTION__, Private->CollectionPath, Status));
  }

  return Status;
}

EFI_STATUS
ReleaseCollectionResource (
  IN REDFISH_COLLECTION_PRIVATE *Private
  )
{

  if (Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Release resource
  //
  if (Private->RedResponse.Payload != NULL) {
    RedfishFreeResponse (
      Private->RedResponse.StatusCode,
      Private->RedResponse.HeaderCount,
      Private->RedResponse.Headers,
      Private->RedResponse.Payload
      );
    Private->RedResponse.StatusCode = NULL;
    Private->RedResponse.HeaderCount = 0;
    Private->RedResponse.Headers = NULL;
    Private->RedResponse.Payload = NULL;
  }

  if (Private->CollectionJson != NULL) {
    FreePool (Private->CollectionJson);
    Private->CollectionJson = NULL;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
CollectionHandler (
  IN REDFISH_COLLECTION_PRIVATE *Private
  )
{
  EFI_STATUS  Status;

  if (Private == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((REDFISH_DEBUG_TRACE, "%a, collection handler for %s\n", __FUNCTION__, Private->CollectionPath));

  //
  // Query collection from Redfish service.
  //
  Status = GetResourceByPath (Private->RedfishService, Private->CollectionPath, &Private->RedResponse);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, unable to get resource from: %s :%r\n", __FUNCTION__, Private->CollectionPath, Status));
    goto ON_RELEASE;
  }

  Private->CollectionPayload = Private->RedResponse.Payload;
  ASSERT (Private->CollectionPayload != NULL);

  Private->CollectionJson = JsonDumpString (RedfishJsonInPayload (Private->CollectionPayload), EDKII_JSON_COMPACT);
  ASSERT (Private->CollectionJson != NULL);

  Status = HandleCollectionResource (Private);
  if (EFI_ERROR (Status) && Status == EFI_NOT_FOUND) {
    Status = CreateCollectionResource (Private);
  }

ON_RELEASE:

  ReleaseCollectionResource (Private);

  return Status;
}

/**
  The callback function provided by Redfish Feature driver.

  @param[in]     This                Pointer to EDKII_REDFISH_FEATURE_PROTOCOL instance.
  @param[in]     FeatureAction       The action Redfish feature driver should take.
  @param[in]     Context             The context of Redfish feature driver.
  @param[in,out] InformationReturned The pointer to retrive the pointer to
                                     FEATURE_RETURNED_INFOMATION. The memory block of this
                                     information should be freed by caller.

  @retval EFI_SUCCESS              Redfish feature driver callback is executed successfully.
  @retval Others                   Some errors happened.

  @retval EFI_SUCCESS              Redfish feature driver callback is executed successfully.
  @retval Others                   Some errors happened.

**/
EFI_STATUS
EFIAPI
RedfishCollectionFeatureCallback (
  IN     EDKII_REDFISH_FEATURE_PROTOCOL *This,
  IN     FEATURE_CALLBACK_ACTION        FeatureAction,
  IN     VOID                           *Context,
  IN OUT FEATURE_RETURNED_INFORMATION   **InformationReturned
  )
{
  EFI_STATUS                   Status;
  REDFISH_SERVICE              RedfishService;
  REDFISH_COLLECTION_PRIVATE   *Private;

  if (FeatureAction != CallbackActionStartOperation) {
    return EFI_UNSUPPORTED;
  }

  Private = (REDFISH_COLLECTION_PRIVATE *)Context;

  RedfishService = Private->RedfishService;
  if (RedfishService == NULL) {
    return EFI_NOT_READY;
  }

  //
  // Initialize collection path
  //
  Private->CollectionPath = RedfishGetUri (REDFISH_MANAGED_URI);
  if (Private->CollectionPath == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }

  Status = CollectionHandler (Private);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, CollectionHandler failure: %r\n", __FUNCTION__, Status));
  }

  return EFI_SUCCESS;
}

/**
  Initialize a Redfish configure handler.

  This function will be called by the Redfish config driver to initialize each Redfish configure
  handler.

  @param[in]   This                     Pointer to EDKII_REDFISH_CONFIG_HANDLER_PROTOCOL instance.
  @param[in]   RedfishConfigServiceInfo Redfish service informaion.

  @retval EFI_SUCCESS                  The handler has been initialized successfully.
  @retval EFI_DEVICE_ERROR             Failed to create or configure the REST EX protocol instance.
  @retval EFI_ALREADY_STARTED          This handler has already been initialized.
  @retval Other                        Error happens during the initialization.

**/
EFI_STATUS
EFIAPI
RedfishCollectionInit (
  IN  EDKII_REDFISH_CONFIG_HANDLER_PROTOCOL  *This,
  IN  REDFISH_CONFIG_SERVICE_INFORMATION   *RedfishConfigServiceInfo
  )
{
  REDFISH_COLLECTION_PRIVATE *Private;

  Private = REDFISH_COLLECTION_PRIVATE_DATA_FROM_PROTOCOL (This);

  Private->RedfishService = RedfishCreateService (RedfishConfigServiceInfo);
  if (Private->RedfishService == NULL) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  Stop a Redfish configure handler.

  @param[in]   This                Pointer to EDKII_REDFISH_CONFIG_HANDLER_PROTOCOL instance.

  @retval EFI_SUCCESS              This handler has been stoped successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
EFIAPI
RedfishCollectionStop (
  IN  EDKII_REDFISH_CONFIG_HANDLER_PROTOCOL       *This
  )
{
  REDFISH_COLLECTION_PRIVATE *Private;

  Private = REDFISH_COLLECTION_PRIVATE_DATA_FROM_PROTOCOL (This);

  if (Private->RedfishService != NULL) {
    RedfishCleanupService (Private->RedfishService);
    Private->RedfishService = NULL;
  }

  ReleaseCollectionResource (Private);

  if (Private->FeatureProtocol != NULL) {
    Private->FeatureProtocol->Unregister (
                                Private->FeatureProtocol,
                                REDFISH_MANAGED_URI,
                                NULL
                                );
  }

  return EFI_SUCCESS;
}

/**
  Callback function when gEfiRestJsonStructureProtocolGuid is installed.

  @param[in] Event    Event whose notification function is being invoked.
  @param[in] Context  Pointer to the notification function's context.
**/
VOID
EFIAPI
EfiRestJasonStructureProtocolIsReady
 (
  IN  EFI_EVENT                             Event,
  IN  VOID                                  *Context
  )
{
  EFI_STATUS  Status;

  if (mRedfishCollectionPrivate == NULL) {
    return;
  }

  if (mRedfishCollectionPrivate->JsonStructProtocol != NULL) {
    return;
  }

  Status = gBS->LocateProtocol (
                 &gEfiRestJsonStructureProtocolGuid,
                 NULL,
                 (VOID **)&mRedfishCollectionPrivate->JsonStructProtocol
                 );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, failed to locate gEfiRestJsonStructureProtocolGuid: %r\n", __FUNCTION__, Status));
  }

  gBS->CloseEvent (Event);
}

/**
  Callback function when gEdkIIRedfishFeatureProtocolGuid is installed.

  @param[in] Event    Event whose notification function is being invoked.
  @param[in] Context  Pointer to the notification function's context.
**/
VOID
EFIAPI
EdkIIRedfishFeatureProtocolIsReady
 (
  IN  EFI_EVENT                             Event,
  IN  VOID                                  *Context
  )
{
  EFI_STATUS                      Status;
  EDKII_REDFISH_FEATURE_PROTOCOL  *FeatureProtocol;

  if (mRedfishCollectionPrivate == NULL) {
    return;
  }

  if (mRedfishCollectionPrivate->FeatureProtocol != NULL) {
    return;
  }

  Status = gBS->LocateProtocol (
                 &gEdkIIRedfishFeatureProtocolGuid,
                 NULL,
                 (VOID **)&FeatureProtocol
                 );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, failed to locate gEdkIIRedfishFeatureProtocolGuid: %r\n", __FUNCTION__, Status));
    gBS->CloseEvent (Event);
    return;
  }

  Status = FeatureProtocol->Register (
                              FeatureProtocol,
                              REDFISH_MANAGED_URI,
                              RedfishCollectionFeatureCallback,
                              (VOID *)mRedfishCollectionPrivate
                              );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, failed to register %s: %r\n", __FUNCTION__, REDFISH_MANAGED_URI, Status));
  }

  mRedfishCollectionPrivate->FeatureProtocol = FeatureProtocol;

  gBS->CloseEvent (Event);
}

/**
  Unloads an image.

  @param  ImageHandle           Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
  @retval EFI_INVALID_PARAMETER ImageHandle is not a valid image handle.

**/
EFI_STATUS
EFIAPI
RedfishCollectionUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS                             Status;
  EDKII_REDFISH_CONFIG_HANDLER_PROTOCOL    *ConfigHandler;

  if (mRedfishCollectionPrivate == NULL) {
    return EFI_NOT_READY;
  }

  ConfigHandler = NULL;

  //
  // Firstly, find ConfigHandler Protocol interface in this ImageHandle.
  //
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEdkIIRedfishConfigHandlerProtocolGuid,
                  (VOID **) &ConfigHandler,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
                  );
  if (EFI_ERROR (Status) || ConfigHandler == NULL) {
    return Status;
  }

  ConfigHandler->Stop (ConfigHandler);

  //
  // Last, uninstall ConfigHandler Protocol.
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  ImageHandle,
                  &gEdkIIRedfishConfigHandlerProtocolGuid,
                  ConfigHandler,
                  NULL
                  );

  FreePool (mRedfishCollectionPrivate);
  mRedfishCollectionPrivate = NULL;

  return Status;
}

EDKII_REDFISH_CONFIG_HANDLER_PROTOCOL mRedfishConfigHandler = {
  RedfishCollectionInit,
  RedfishCollectionStop
};

/**
  This is the declaration of an EFI image entry point. This entry point is
  the same for UEFI Applications, UEFI OS Loaders, and UEFI Drivers including
  both device drivers and bus drivers. It initialize the global variables and
  publish the driver binding protocol.

  @param[in]   ImageHandle      The firmware allocated handle for the UEFI image.
  @param[in]   SystemTable      A pointer to the EFI System Table.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ACCESS_DENIED     EFI_ISCSI_INITIATOR_NAME_PROTOCOL was installed unexpectedly.
  @retval Others                Other errors as indicated.
**/
EFI_STATUS
EFIAPI
RedfishCollectionEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS               Status;
  VOID                     *Registration;

  if (mRedfishCollectionPrivate != NULL) {
    return EFI_ALREADY_STARTED;
  }

  mRedfishCollectionPrivate = AllocateZeroPool (sizeof (REDFISH_COLLECTION_PRIVATE));
  CopyMem (&mRedfishCollectionPrivate->ConfigHandler, &mRedfishConfigHandler, sizeof (EDKII_REDFISH_CONFIG_HANDLER_PROTOCOL));

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEdkIIRedfishConfigHandlerProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mRedfishCollectionPrivate->ConfigHandler
                  );

  EfiCreateProtocolNotifyEvent (
    &gEfiRestJsonStructureProtocolGuid,
    TPL_CALLBACK,
    EfiRestJasonStructureProtocolIsReady,
    NULL,
    &Registration
    );

  EfiCreateProtocolNotifyEvent (
    &gEdkIIRedfishFeatureProtocolGuid,
    TPL_CALLBACK,
    EdkIIRedfishFeatureProtocolIsReady,
    NULL,
    &Registration
    );

  return Status;
}
