/** @file

  (C) Copyright 2022 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "RedfishResourceMapDxe.h"

REDFISH_RESOURCE_MAP_PRIVATE_DATA  *mRedfishResourceMapPrivate = NULL;

/**
  Release REDFISH_RESOURCE_MAP_RECORD resource

  @param[in]    Record    Pointer to REDFISH_RESOURCE_MAP_RECORD instance

  @retval EFI_SUCCESS             REDFISH_RESOURCE_MAP_RECORD is released successfully.
  @retval EFI_INVALID_PARAMETER   Record is NULL

**/
EFI_STATUS
ReleaseResourceMapRecord (
  IN REDFISH_RESOURCE_MAP_RECORD *Record
  )
{
  if (Record == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Record->Uri != NULL) {
    FreePool (Record->Uri);
  }

  if (Record->ConfigLang != NULL) {
    FreePool (Record->ConfigLang);
  }

  FreePool (Record);

  return EFI_SUCCESS;
}

/**
  Create new resource map resource.

  @param[in]    Uri         The URI string matching to this ConfigLang.
  @param[in]    ConfigLang  ConfigLang string.

  @retval REDFISH_RESOURCE_MAP_RECORD *   Pointer to newly created RESOURCE MAP.
  @retval NULL                            No memory available.

**/
REDFISH_RESOURCE_MAP_RECORD *
NewResourceMapRecord (
  IN  CHAR8   *Uri,
  IN  CHAR8   *ConfigLang
  )
{
  REDFISH_RESOURCE_MAP_RECORD *NewRecord;
  UINTN               Size;

  if (IS_EMPTY_STRING (Uri) || IS_EMPTY_STRING (ConfigLang)) {
    return NULL;
  }

  NewRecord = AllocateZeroPool (sizeof (REDFISH_RESOURCE_MAP_RECORD));
  if (NewRecord == NULL) {
    return NULL;
  }

  Size = AsciiStrSize (Uri);
  NewRecord->Uri = AllocateCopyPool (Size, Uri);
  if (NewRecord->Uri == NULL) {
    goto ON_ERROR;
  }

  NewRecord->Size = Size;
  Size = AsciiStrSize (ConfigLang);
  NewRecord->ConfigLang = AllocateCopyPool (Size, ConfigLang);
  if (NewRecord->ConfigLang == NULL) {
    goto ON_ERROR;
  }

  NewRecord->Size += Size;
  return NewRecord;

ON_ERROR:

  if (NewRecord != NULL) {
    ReleaseResourceMapRecord (NewRecord);
  }

  return NULL;
}

/**
  Add new RESOURCE MAP by given URI and ConfigLang string to specify List.

  @param[in]    List        Target RESOURCE MAP list to add.
  @param[in]    Uri         The URI string matching to this ConfigLang.
  @param[in]    ConfigLang  ConfigLang string.

  @retval EFI_SUCCESS   RESOURCE MAP recourd is added.
  @retval Others        Fail to add RESOURCE MAP.

**/
EFI_STATUS
AddResourceMapRecord (
  IN  REDFISH_RESOURCE_MAP_LIST *List,
  IN  CHAR8             *Uri,
  IN  CHAR8             *ConfigLang
  )
{
  REDFISH_RESOURCE_MAP_RECORD *NewRecord;

  if (List == NULL || IS_EMPTY_STRING (Uri) || IS_EMPTY_STRING (ConfigLang)) {
    return EFI_INVALID_PARAMETER;
  }

  NewRecord = NewResourceMapRecord (Uri, ConfigLang);
  if (NewResourceMapRecord == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  InsertTailList (&List->Listheader, &NewRecord->List);
  ++List->Count;
  List->TotalSize += NewRecord->Size;

  return EFI_SUCCESS;
}

/**
  Delete an RESOURCE MAP by given RESOURCE MAP instance.

  @param[in]    List    Target RESOURCE MAP list to be removed.
  @param[in]    Record  Pointer to the instance to be deleted.

  @retval EFI_SUCCESS   RESOURCE MAP recourd is removed.
  @retval Others        Fail to add RESOURCE MAP.

**/
EFI_STATUS
DeleteResourceMapRecord (
  IN  REDFISH_RESOURCE_MAP_LIST   *List,
  IN  REDFISH_RESOURCE_MAP_RECORD *Record
  )
{
  if (List == NULL || Record == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  RemoveEntryList (&Record->List);
  --List->Count;
  List->TotalSize -= Record->Size;

  return ReleaseResourceMapRecord (Record);
}

/**
  Search on given ListHeader for given ConfigLang string.

  @param[in]    ListHeader  Target list to search.
  @param[in]    ConfigLang  Target ConfigLang to search.

  @retval REDFISH_RESOURCE_MAP_RECORD   Target RESOURCE MAP is found.
  @retval NULL                          No RESOURCE MAP with given ConfigLang is found.

**/
REDFISH_RESOURCE_MAP_RECORD *
FindResourceMapRecord (
  IN  LIST_ENTRY  *ListHeader,
  IN  CHAR8       *ConfigLang
  )
{
  LIST_ENTRY          *List;
  REDFISH_RESOURCE_MAP_RECORD *Record;

  if (IsListEmpty (ListHeader)) {
    return NULL;
  }

  Record = NULL;
  List = GetFirstNode (ListHeader);
  while (!IsNull (ListHeader, List)) {
    Record = REDFISH_RESOURCE_MAP_RECORD_FROM_LIST (List);

    if (AsciiStrCmp (Record->ConfigLang, ConfigLang) == 0) {
      return  Record;
    }

    List = GetNextNode (ListHeader, List);
  }

  return NULL;
}

#if RESOURCE_MAP_DEBUG_ENABLED
/**
  Debug output the RESOURCE MAP list.

  @param[in]    ResourceMapList  Target list to dump
  @param[in]    Msg       Debug message string.

  @retval EFI_SUCCESS             Debug dump finished.
  @retval EFI_INVALID_PARAMETER   ResourceMapList is NULL.

**/
EFI_STATUS
DumpResourceMapList (
  IN  REDFISH_RESOURCE_MAP_LIST   *ResourceMapList,
  IN  EFI_STRING          Msg
  )
{
  LIST_ENTRY          *List;
  REDFISH_RESOURCE_MAP_RECORD *Record;

  if (ResourceMapList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IS_EMPTY_STRING (Msg)) {
    DEBUG ((DEBUG_ERROR, "%s\n", Msg));
  }

  if (IsListEmpty (&ResourceMapList->Listheader)) {
    DEBUG ((DEBUG_INFO, "ResourceMap list is empty\n"));
    return EFI_NOT_FOUND;
  }

  DEBUG ((DEBUG_INFO, "Count: %d Total Size: %d\n", ResourceMapList->Count, ResourceMapList->TotalSize));
  Record = NULL;
  List = GetFirstNode (&ResourceMapList->Listheader);
  while (!IsNull (&ResourceMapList->Listheader, List)) {
    Record = REDFISH_RESOURCE_MAP_RECORD_FROM_LIST (List);

    DEBUG ((DEBUG_INFO, "ConfigLang: %a Uri: %a Size: %d\n", Record->ConfigLang, Record->Uri, Record->Size));

    List = GetNextNode (&ResourceMapList->Listheader, List);
  }

  return EFI_SUCCESS;
}

/**
  Debug output raw data buffer.

  @param[in]    Buffer      Debug output data buffer.
  @param[in]    BufferSize  The size of Buffer in byte.

  @retval EFI_SUCCESS             Debug dump finished.
  @retval EFI_INVALID_PARAMETER   Buffer is NULL.

**/
EFI_STATUS
DumpRawBuffer (
  IN  CHAR8    *Buffer,
  IN  UINTN     BufferSize
  )
{
  UINTN  Index;

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Index = 0;

  DEBUG ((DEBUG_ERROR, "Buffer size: %d\n", BufferSize));
  while (Buffer[Index] != '\0') {
    DEBUG ((DEBUG_ERROR, "(%d) %c ", (Index + 1), Buffer[Index]));

    ++Index;
  }
  DEBUG ((DEBUG_ERROR, "\n"));

  return EFI_SUCCESS;
}
#endif

/**
  Release all ResourceMap from list.

  @param[in]    ResourceMapList    The list to be released.

  @retval EFI_SUCCESS             All etag is released.
  @retval EFI_INVALID_PARAMETER   ResourceMapList is NULL.

**/
EFI_STATUS
ReleaseResourceMapList (
  IN  REDFISH_RESOURCE_MAP_LIST   *ResourceMapList
  )
{
  LIST_ENTRY          *List;
  LIST_ENTRY          *Next;
  REDFISH_RESOURCE_MAP_RECORD *Record;

  if (ResourceMapList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsListEmpty (&ResourceMapList->Listheader)) {
    return EFI_SUCCESS;
  }

  Record = NULL;
  Next = NULL;
  List = GetFirstNode (&ResourceMapList->Listheader);
  while (!IsNull (&ResourceMapList->Listheader, List)) {
    Record = REDFISH_RESOURCE_MAP_RECORD_FROM_LIST (List);
    Next = GetNextNode (&ResourceMapList->Listheader, List);

    DeleteResourceMapRecord (ResourceMapList, Record);

    List = Next;
  }

  return EFI_SUCCESS;
}

/**
  Save etag in list to UEFI variable.

  @param[in]    ResourceMapList      The list to be saved.
  @param[in]    VariableName  The UEFI variable name.

  @retval EFI_SUCCESS             All etag is saved.
  @retval EFI_INVALID_PARAMETER   VariableName or ResourceMapList is NULL.

**/
EFI_STATUS
SaveResourceMapList (
  IN  REDFISH_RESOURCE_MAP_LIST   *ResourceMapList,
  IN  EFI_STRING          VariableName
  )
{
  LIST_ENTRY          *List;
  REDFISH_RESOURCE_MAP_RECORD *Record;
  CHAR8               *VarData;
  VOID                *Data;
  CHAR8               *Seeker;
  UINTN               VarSize;
  UINTN               StrSize;
  EFI_STATUS          Status;

  if (ResourceMapList == NULL || IS_EMPTY_STRING (VariableName)) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsListEmpty (&ResourceMapList->Listheader)) {
    return EFI_SUCCESS;
  }

  //
  // Caculate the total size we need to keep ResourceMap list.
  //
  VarSize = ResourceMapList->TotalSize + 1; // terminator character
  VarData = AllocateZeroPool (VarSize);
  if (VarData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Seeker = VarData;
  Record = NULL;
  List = GetFirstNode (&ResourceMapList->Listheader);
  while (!IsNull (&ResourceMapList->Listheader, List)) {
    Record = REDFISH_RESOURCE_MAP_RECORD_FROM_LIST (List);

    StrSize = AsciiStrSize (Record->Uri);
    CopyMem (Seeker, Record->Uri, StrSize);

    Seeker += (StrSize - 1);
    *Seeker = '|';
    ++Seeker;

    StrSize = AsciiStrSize (Record->ConfigLang);
    CopyMem (Seeker, Record->ConfigLang, StrSize);

    Seeker += (StrSize - 1);
    *Seeker = '\n';

    ++Seeker;

    List = GetNextNode (&ResourceMapList->Listheader, List);;
  }

  *Seeker = '\0';

#if RESOURCE_MAP_DEBUG_ENABLED
  DumpRawBuffer (VarData, VarSize);
#endif

  ASSERT (((UINTN)Seeker - (UINTN)VarData + 1) == VarSize);

  //
  // Check if variable exists already. If yes, remove it first.
  //
  Status = GetVariable2 (
             VariableName,
             &mRedfishVariableGuid,
             (VOID *)&Data,
             NULL
             );
  if (!EFI_ERROR (Status)) {
    FreePool (Data);
    gRT->SetVariable (VariableName, &mRedfishVariableGuid, VARIABLE_ATTRIBUTE_NV_BS, 0, NULL);
  }


  return gRT->SetVariable (VariableName, &mRedfishVariableGuid, VARIABLE_ATTRIBUTE_NV_BS, VarSize, (VOID *)VarData);
}

/**
  Read etag from UEFI variable if it exists.

  @param[in]    ResourceMapList      The list to be loaded.
  @param[in]    VariableName  The UEFI variable name.

  @retval EFI_SUCCESS             All etag is read successfully.
  @retval EFI_INVALID_PARAMETER   VariableName or ResourceMapList is NULL.
  @retval EFI_NOT_FOUND           No etag is found on UEFI variable.

**/
EFI_STATUS
InitialResourceMapList (
  IN  REDFISH_RESOURCE_MAP_LIST   *ResourceMapList,
  IN  EFI_STRING          VariableName
  )
{
  CHAR8     *VarData;
  CHAR8     *UriPointer;
  CHAR8     *ConfigLangPointer;
  CHAR8     *Seeker;
  UINTN      VariableSize;
  EFI_STATUS Status;

  if (ResourceMapList == NULL || IS_EMPTY_STRING (VariableName)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check if variable exists already.
  //
  Status = GetVariable2 (
             VariableName,
             &mRedfishVariableGuid,
             (VOID *)&VarData,
             &VariableSize
             );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  Seeker = VarData;
  UriPointer = VarData;
  ConfigLangPointer = VarData;
  while (*Seeker != '\0') {

    //
    // Find URI
    //
    Seeker = AsciiStrStr (UriPointer, "|");
    if (Seeker == NULL) {
      DEBUG ((DEBUG_ERROR, "%a, data corrupted\n", __FUNCTION__));
      Status = EFI_DEVICE_ERROR;
      goto ON_ERROR;
    }

    *Seeker = '\0';
    ConfigLangPointer = ++Seeker;

    //
    // Find RESOURCE MAP
    //
    Seeker = AsciiStrStr (ConfigLangPointer, "\n");
    if (Seeker == NULL) {
      DEBUG ((DEBUG_ERROR, "%a, data corrupted\n", __FUNCTION__));
      Status = EFI_DEVICE_ERROR;
      goto ON_ERROR;
    }

    *Seeker = '\0';

    AddResourceMapRecord (ResourceMapList, UriPointer, ConfigLangPointer);

    UriPointer = ++Seeker;
  }

#if RESOURCE_MAP_DEBUG_ENABLED
  DumpResourceMapList (ResourceMapList, L"Initial ResourceMap List from Variable");
#endif

  Status = EFI_SUCCESS;

ON_ERROR:

  FreePool (VarData);

  return Status;
}

/**
  Get URI string by given ConfigLang. It is the responsibility of the caller to free the memory allocated.

  @param[in]   This                    Pointer to EDKII_REDFISH_RESOURCE_MAP_PROTOCOL instance.
  @param[in]   ConfigLang              Config language to search
  @param[out]  Uri                     Returned URI mapping to give ConfigLang

  @retval EFI_SUCCESS                  The Uri is found successfully.
  @retval EFI_INVALID_PARAMETER        Invalid parameter is given.

**/
EFI_STATUS
RedfishResourceMapGet (
  IN  EDKII_REDFISH_RESOURCE_MAP_PROTOCOL *This,
  IN  CHAR8                               *ConfigLang,
  OUT CHAR8                               **Uri
  )
{
  REDFISH_RESOURCE_MAP_RECORD       *Target;
  REDFISH_RESOURCE_MAP_PRIVATE_DATA *Private;

  if (This == NULL || IS_EMPTY_STRING (ConfigLang) || Uri == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = REDFISH_RESOURCE_MAP_PRIVATE_FROM_THIS (This);

  *Uri = NULL;

  Target = FindResourceMapRecord (&Private->ResourceList.Listheader, ConfigLang);
  if (Target == NULL) {
    return EFI_NOT_FOUND;
  }

  *Uri = AllocateCopyPool (AsciiStrSize (Target->Uri), Target->Uri);

  return EFI_SUCCESS;
}


/**
  Save URI string which maps to given ConfigLang.

  @param[in]   This                Pointer to EDKII_REDFISH_RESOURCE_MAP_PROTOCOL instance.
  @param[in]   ConfigLang          Config language to set
  @param[in]   Uri                 Uri which is mapping to give ConfigLang. If Uri is NULL,
                                   the record will be removed.

  @retval EFI_SUCCESS              Uri is saved successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
RedfishResourceMapSet (
  IN  EDKII_REDFISH_RESOURCE_MAP_PROTOCOL   *This,
  IN  CHAR8                                 *ConfigLang,
  IN  CHAR8                                 *Uri        OPTIONAL
  )
{
  REDFISH_RESOURCE_MAP_RECORD       *Target;
  REDFISH_RESOURCE_MAP_PRIVATE_DATA *Private;
  EFI_STATUS                        Status;

  if (This == NULL || IS_EMPTY_STRING (ConfigLang)) {
    return EFI_INVALID_PARAMETER;
  }

  Private = REDFISH_RESOURCE_MAP_PRIVATE_FROM_THIS (This);

  Status = EFI_NOT_FOUND;
  Target = FindResourceMapRecord (&Private->ResourceList.Listheader, ConfigLang);
  if (Target != NULL) {
    //
    // Remove old one and create new one.
    //
    Status = DeleteResourceMapRecord (&Private->ResourceList, Target);
  }

  //
  // When Uri is NULL, it means that we want to remov this record.
  //
  if (Uri == NULL) {
    return Status;
  }

  return AddResourceMapRecord (&Private->ResourceList, Uri, ConfigLang);
}

/**
  Refresh the resource map database and save database to variable.

  @param[in]   This                Pointer to EDKII_REDFISH_RESOURCE_MAP_PROTOCOL instance.

  @retval EFI_SUCCESS              This handler has been stoped successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
RedfishResourceMapFlush (
  IN  EDKII_REDFISH_RESOURCE_MAP_PROTOCOL    *This
  )
{
  REDFISH_RESOURCE_MAP_PRIVATE_DATA *Private;
  EFI_STATUS                Status;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = REDFISH_RESOURCE_MAP_PRIVATE_FROM_THIS (This);

  Status = SaveResourceMapList (&Private->ResourceList, Private->VariableName);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, save ResourceMap list to variable: %s failed: %r\n", __FUNCTION__, Private->VariableName, Status));
  }

  return Status;
}

/**
  Callback function executed when the ExitBootService event group is signaled.

  @param[in]   Event    Event whose notification function is being invoked.
  @param[out]  Context  Pointer to the Context buffer

**/
VOID
EFIAPI
RedfishResourceMapOnExitBootService (
  IN  EFI_EVENT  Event,
  OUT VOID       *Context
  )
{
  //
  // Memory is about to be released. Keep list into variable.
  //
  RedfishResourceMapFlush (&mRedfishResourceMapPrivate->Protocol);
}

/**
  Unloads an image.

  @param[in]  ImageHandle           Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
  @retval EFI_INVALID_PARAMETER ImageHandle is not a valid image handle.

**/
EFI_STATUS
EFIAPI
RedfishResourceMapDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS  Status;

  if (mRedfishResourceMapPrivate != NULL) {

    Status = gBS->UninstallProtocolInterface (
                    mRedfishResourceMapPrivate->ImageHandle,
                    &gEdkIIRedfishResourceMapProtocolGuid,
                    (VOID*)&mRedfishResourceMapPrivate->Protocol
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a, can not uninstall gEdkIIRedfishResourceMapProtocolGuid: %r\n", __FUNCTION__, Status));
      ASSERT (FALSE);
    }

    ReleaseResourceMapList (&mRedfishResourceMapPrivate->ResourceList);

    if (mRedfishResourceMapPrivate->VariableName != NULL) {
      FreePool (mRedfishResourceMapPrivate->VariableName);
    }

    if (mRedfishResourceMapPrivate->Event != NULL) {
      gBS->CloseEvent (mRedfishResourceMapPrivate->Event);
    }

    FreePool (mRedfishResourceMapPrivate);
    mRedfishResourceMapPrivate = NULL;
  }


  return EFI_SUCCESS;
}

//
// EDKII_REDFISH_RESOURCE_MAP_PROTOCOL.
//
EDKII_REDFISH_RESOURCE_MAP_PROTOCOL mRedfishResourceMapProtocol = {
  RedfishResourceMapGet,
  RedfishResourceMapSet,
  RedfishResourceMapFlush
};

/**
  This is the declaration of an EFI image entry point. This entry point is
  the same for UEFI Applications, UEFI OS Loaders, and UEFI Drivers including
  both device drivers and bus drivers.

  @param[in]  ImageHandle       The firmware allocated handle for the UEFI image.
  @param[in]  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval Others                An unexpected error occurred.
**/
EFI_STATUS
EFIAPI
RedfishResourceMapDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS    Status;

  mRedfishResourceMapPrivate = AllocateZeroPool (sizeof (REDFISH_RESOURCE_MAP_PRIVATE_DATA));
  if (mRedfishResourceMapPrivate == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  InitializeListHead (&mRedfishResourceMapPrivate->ResourceList.Listheader);
  mRedfishResourceMapPrivate->VariableName = AllocateCopyPool (StrSize (RESOURCE_MAP_VARIABLE_NAME), RESOURCE_MAP_VARIABLE_NAME);
  if (mRedfishResourceMapPrivate->VariableName == NULL) {
    goto ON_ERROR;
  }

  mRedfishResourceMapPrivate->ImageHandle = ImageHandle;
  CopyMem (&mRedfishResourceMapPrivate->Protocol, &mRedfishResourceMapProtocol, sizeof (EDKII_REDFISH_RESOURCE_MAP_PROTOCOL));

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEdkIIRedfishResourceMapProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  (VOID*)&mRedfishResourceMapPrivate->Protocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, can not install gEdkIIRedfishResourceMapProtocolGuid: %r\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    goto ON_ERROR;
  }

  //
  // Create Exit Boot Service event.
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  RedfishResourceMapOnExitBootService,
                  NULL,
                  &gEfiEventExitBootServicesGuid,
                  &mRedfishResourceMapPrivate->Event
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Fail to register Exit Boot Service event.", __FUNCTION__));
    goto ON_ERROR;
  }

  //
  // Read existing record from variable.
  //
  Status = InitialResourceMapList (&mRedfishResourceMapPrivate->ResourceList, mRedfishResourceMapPrivate->VariableName);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a, Initial ResourceMap List: %r\n", __FUNCTION__, Status));
  }

  return EFI_SUCCESS;

ON_ERROR:

  RedfishResourceMapDriverUnload (ImageHandle);

  return Status;
}
