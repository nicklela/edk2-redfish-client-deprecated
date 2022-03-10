/** @file

  (C) Copyright 2022 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "RedfishConfigLangMapDxe.h"

REDFISH_CONFIG_LANG_MAP_PRIVATE_DATA  *mRedfishConfigLangMapPrivate = NULL;

/**
  Release REDFISH_CONFIG_LANG_MAP_RECORD resource

  @param[in]    Record    Pointer to REDFISH_CONFIG_LANG_MAP_RECORD instance

  @retval EFI_SUCCESS             REDFISH_CONFIG_LANG_MAP_RECORD is released successfully.
  @retval EFI_INVALID_PARAMETER   Record is NULL

**/
EFI_STATUS
ReleaseConfigLangMapRecord (
  IN REDFISH_CONFIG_LANG_MAP_RECORD *Record
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

  @retval REDFISH_CONFIG_LANG_MAP_RECORD *  Pointer to newly created config language map.
  @retval NULL                              No memory available.

**/
REDFISH_CONFIG_LANG_MAP_RECORD *
NewConfigLangMapRecord (
  IN  CHAR8   *Uri,
  IN  CHAR8   *ConfigLang
  )
{
  REDFISH_CONFIG_LANG_MAP_RECORD *NewRecord;
  UINTN                          Size;

  if (IS_EMPTY_STRING (Uri) || IS_EMPTY_STRING (ConfigLang)) {
    return NULL;
  }

  NewRecord = AllocateZeroPool (sizeof (REDFISH_CONFIG_LANG_MAP_RECORD));
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
    ReleaseConfigLangMapRecord (NewRecord);
  }

  return NULL;
}

/**
  Add new config language map by given URI and ConfigLang string to specify List.

  @param[in]    List        Target config language map list to add.
  @param[in]    Uri         The URI string matching to this ConfigLang.
  @param[in]    ConfigLang  ConfigLang string.

  @retval EFI_SUCCESS   config language map recourd is added.
  @retval Others        Fail to add config language map.

**/
EFI_STATUS
AddConfigLangMapRecord (
  IN  REDFISH_CONFIG_LANG_MAP_LIST *List,
  IN  CHAR8                        *Uri,
  IN  CHAR8                        *ConfigLang
  )
{
  REDFISH_CONFIG_LANG_MAP_RECORD *NewRecord;

  if (List == NULL || IS_EMPTY_STRING (Uri) || IS_EMPTY_STRING (ConfigLang)) {
    return EFI_INVALID_PARAMETER;
  }

  NewRecord = NewConfigLangMapRecord (Uri, ConfigLang);
  if (NewConfigLangMapRecord == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  InsertTailList (&List->Listheader, &NewRecord->List);
  ++List->Count;
  List->TotalSize += NewRecord->Size;

  return EFI_SUCCESS;
}

/**
  Delete an config language map by given config language map instance.

  @param[in]    List    Target config language map list to be removed.
  @param[in]    Record  Pointer to the instance to be deleted.

  @retval EFI_SUCCESS   config language map recourd is removed.
  @retval Others        Fail to add config language map.

**/
EFI_STATUS
DeleteConfigLangMapRecord (
  IN  REDFISH_CONFIG_LANG_MAP_LIST   *List,
  IN  REDFISH_CONFIG_LANG_MAP_RECORD *Record
  )
{
  if (List == NULL || Record == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  RemoveEntryList (&Record->List);
  --List->Count;
  List->TotalSize -= Record->Size;

  return ReleaseConfigLangMapRecord (Record);
}

/**
  Search on given ListHeader for given ConfigLang string.

  @param[in]    ListHeader  Target list to search.
  @param[in]    ConfigLang  Target ConfigLang to search.

  @retval REDFISH_CONFIG_LANG_MAP_RECORD  Target config language map is found.
  @retval NULL                            No config language map with given ConfigLang is found.

**/
REDFISH_CONFIG_LANG_MAP_RECORD *
FindConfigLangMapRecord (
  IN  LIST_ENTRY  *ListHeader,
  IN  CHAR8       *ConfigLang
  )
{
  LIST_ENTRY          *List;
  REDFISH_CONFIG_LANG_MAP_RECORD *Record;

  if (IsListEmpty (ListHeader)) {
    return NULL;
  }

  Record = NULL;
  List = GetFirstNode (ListHeader);
  while (!IsNull (ListHeader, List)) {
    Record = REDFISH_CONFIG_LANG_MAP_RECORD_FROM_LIST (List);

    if (AsciiStrCmp (Record->ConfigLang, ConfigLang) == 0) {
      return  Record;
    }

    List = GetNextNode (ListHeader, List);
  }

  return NULL;
}

#if CONFIG_LANG_MAP_DEBUG_ENABLED
/**
  Debug output the config language map list.

  @param[in]    ConfigLangMapList  Target list to dump
  @param[in]    Msg       Debug message string.

  @retval EFI_SUCCESS             Debug dump finished.
  @retval EFI_INVALID_PARAMETER   ConfigLangMapList is NULL.

**/
EFI_STATUS
DumpConfigLangMapList (
  IN  REDFISH_CONFIG_LANG_MAP_LIST  *ConfigLangMapList,
  IN  EFI_STRING                    Msg
  )
{
  LIST_ENTRY          *List;
  REDFISH_CONFIG_LANG_MAP_RECORD *Record;

  if (ConfigLangMapList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IS_EMPTY_STRING (Msg)) {
    DEBUG ((DEBUG_ERROR, "%s\n", Msg));
  }

  if (IsListEmpty (&ConfigLangMapList->Listheader)) {
    DEBUG ((DEBUG_INFO, "ConfigLangMap list is empty\n"));
    return EFI_NOT_FOUND;
  }

  DEBUG ((DEBUG_INFO, "Count: %d Total Size: %d\n", ConfigLangMapList->Count, ConfigLangMapList->TotalSize));
  Record = NULL;
  List = GetFirstNode (&ConfigLangMapList->Listheader);
  while (!IsNull (&ConfigLangMapList->Listheader, List)) {
    Record = REDFISH_CONFIG_LANG_MAP_RECORD_FROM_LIST (List);

    DEBUG ((DEBUG_INFO, "ConfigLang: %a Uri: %a Size: %d\n", Record->ConfigLang, Record->Uri, Record->Size));

    List = GetNextNode (&ConfigLangMapList->Listheader, List);
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
  Release all ConfigLangMap from list.

  @param[in]    ConfigLangMapList    The list to be released.

  @retval EFI_SUCCESS             All etag is released.
  @retval EFI_INVALID_PARAMETER   ConfigLangMapList is NULL.

**/
EFI_STATUS
ReleaseConfigLangMapList (
  IN  REDFISH_CONFIG_LANG_MAP_LIST   *ConfigLangMapList
  )
{
  LIST_ENTRY          *List;
  LIST_ENTRY          *Next;
  REDFISH_CONFIG_LANG_MAP_RECORD *Record;

  if (ConfigLangMapList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsListEmpty (&ConfigLangMapList->Listheader)) {
    return EFI_SUCCESS;
  }

  Record = NULL;
  Next = NULL;
  List = GetFirstNode (&ConfigLangMapList->Listheader);
  while (!IsNull (&ConfigLangMapList->Listheader, List)) {
    Record = REDFISH_CONFIG_LANG_MAP_RECORD_FROM_LIST (List);
    Next = GetNextNode (&ConfigLangMapList->Listheader, List);

    DeleteConfigLangMapRecord (ConfigLangMapList, Record);

    List = Next;
  }

  return EFI_SUCCESS;
}

/**
  Save etag in list to UEFI variable.

  @param[in]    ConfigLangMapList The list to be saved.
  @param[in]    VariableName      The UEFI variable name.

  @retval EFI_SUCCESS             All etag is saved.
  @retval EFI_INVALID_PARAMETER   VariableName or ConfigLangMapList is NULL.

**/
EFI_STATUS
SaveConfigLangMapList (
  IN  REDFISH_CONFIG_LANG_MAP_LIST  *ConfigLangMapList,
  IN  EFI_STRING                    VariableName
  )
{
  LIST_ENTRY          *List;
  REDFISH_CONFIG_LANG_MAP_RECORD *Record;
  CHAR8               *VarData;
  VOID                *Data;
  CHAR8               *Seeker;
  UINTN               VarSize;
  UINTN               StrSize;
  EFI_STATUS          Status;

  if (ConfigLangMapList == NULL || IS_EMPTY_STRING (VariableName)) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsListEmpty (&ConfigLangMapList->Listheader)) {
    return EFI_SUCCESS;
  }

  //
  // Caculate the total size we need to keep ConfigLangMap list.
  //
  VarSize = ConfigLangMapList->TotalSize + 1; // terminator character
  VarData = AllocateZeroPool (VarSize);
  if (VarData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Seeker = VarData;
  Record = NULL;
  List = GetFirstNode (&ConfigLangMapList->Listheader);
  while (!IsNull (&ConfigLangMapList->Listheader, List)) {
    Record = REDFISH_CONFIG_LANG_MAP_RECORD_FROM_LIST (List);

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

    List = GetNextNode (&ConfigLangMapList->Listheader, List);;
  }

  *Seeker = '\0';

#if CONFIG_LANG_MAP_DEBUG_ENABLED
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

  @param[in]    ConfigLangMapList The list to be loaded.
  @param[in]    VariableName      The UEFI variable name.

  @retval EFI_SUCCESS             All etag is read successfully.
  @retval EFI_INVALID_PARAMETER   VariableName or ConfigLangMapList is NULL.
  @retval EFI_NOT_FOUND           No etag is found on UEFI variable.

**/
EFI_STATUS
InitialConfigLangMapList (
  IN  REDFISH_CONFIG_LANG_MAP_LIST   *ConfigLangMapList,
  IN  EFI_STRING          VariableName
  )
{
  CHAR8     *VarData;
  CHAR8     *UriPointer;
  CHAR8     *ConfigLangPointer;
  CHAR8     *Seeker;
  UINTN      VariableSize;
  EFI_STATUS Status;

  if (ConfigLangMapList == NULL || IS_EMPTY_STRING (VariableName)) {
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
    // Find config language map
    //
    Seeker = AsciiStrStr (ConfigLangPointer, "\n");
    if (Seeker == NULL) {
      DEBUG ((DEBUG_ERROR, "%a, data corrupted\n", __FUNCTION__));
      Status = EFI_DEVICE_ERROR;
      goto ON_ERROR;
    }

    *Seeker = '\0';

    AddConfigLangMapRecord (ConfigLangMapList, UriPointer, ConfigLangPointer);

    UriPointer = ++Seeker;
  }

#if CONFIG_LANG_MAP_DEBUG_ENABLED
  DumpConfigLangMapList (ConfigLangMapList, L"Initial ConfigLangMap List from Variable");
#endif

  Status = EFI_SUCCESS;

ON_ERROR:

  FreePool (VarData);

  return Status;
}

/**
  Get URI string by given ConfigLang. It is the responsibility of the caller to free the memory allocated.

  @param[in]   This                    Pointer to EDKII_REDFISH_CONFIG_LANG_MAP_PROTOCOL instance.
  @param[in]   ConfigLang              Config language to search
  @param[out]  Uri                     Returned URI mapping to give ConfigLang

  @retval EFI_SUCCESS                  The Uri is found successfully.
  @retval EFI_INVALID_PARAMETER        Invalid parameter is given.

**/
EFI_STATUS
RedfishConfigLangMapGet (
  IN  EDKII_REDFISH_CONFIG_LANG_MAP_PROTOCOL  *This,
  IN  CHAR8                                   *ConfigLang,
  OUT CHAR8                                   **Uri
  )
{
  REDFISH_CONFIG_LANG_MAP_RECORD       *Target;
  REDFISH_CONFIG_LANG_MAP_PRIVATE_DATA *Private;

  if (This == NULL || IS_EMPTY_STRING (ConfigLang) || Uri == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = REDFISH_CONFIG_LANG_MAP_PRIVATE_FROM_THIS (This);

  *Uri = NULL;

  Target = FindConfigLangMapRecord (&Private->ConfigLangList.Listheader, ConfigLang);
  if (Target == NULL) {
    return EFI_NOT_FOUND;
  }

  *Uri = AllocateCopyPool (AsciiStrSize (Target->Uri), Target->Uri);

  return EFI_SUCCESS;
}


/**
  Save URI string which maps to given ConfigLang.

  @param[in]   This                Pointer to EDKII_REDFISH_CONFIG_LANG_MAP_PROTOCOL instance.
  @param[in]   ConfigLang          Config language to set
  @param[in]   Uri                 Uri which is mapping to give ConfigLang. If Uri is NULL,
                                   the record will be removed.

  @retval EFI_SUCCESS              Uri is saved successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
RedfishConfigLangMapSet (
  IN  EDKII_REDFISH_CONFIG_LANG_MAP_PROTOCOL  *This,
  IN  CHAR8                                   *ConfigLang,
  IN  CHAR8                                   *Uri        OPTIONAL
  )
{
  REDFISH_CONFIG_LANG_MAP_RECORD       *Target;
  REDFISH_CONFIG_LANG_MAP_PRIVATE_DATA *Private;
  EFI_STATUS                        Status;

  if (This == NULL || IS_EMPTY_STRING (ConfigLang)) {
    return EFI_INVALID_PARAMETER;
  }

  Private = REDFISH_CONFIG_LANG_MAP_PRIVATE_FROM_THIS (This);

  Status = EFI_NOT_FOUND;
  Target = FindConfigLangMapRecord (&Private->ConfigLangList.Listheader, ConfigLang);
  if (Target != NULL) {
    //
    // Remove old one and create new one.
    //
    Status = DeleteConfigLangMapRecord (&Private->ConfigLangList, Target);
  }

  //
  // When Uri is NULL, it means that we want to remov this record.
  //
  if (Uri == NULL) {
    return Status;
  }

  return AddConfigLangMapRecord (&Private->ConfigLangList, Uri, ConfigLang);
}

/**
  Refresh the resource map database and save database to variable.

  @param[in]   This                Pointer to EDKII_REDFISH_CONFIG_LANG_MAP_PROTOCOL instance.

  @retval EFI_SUCCESS              This handler has been stoped successfully.
  @retval Others                   Some error happened.

**/
EFI_STATUS
RedfishConfigLangMapFlush (
  IN  EDKII_REDFISH_CONFIG_LANG_MAP_PROTOCOL    *This
  )
{
  REDFISH_CONFIG_LANG_MAP_PRIVATE_DATA *Private;
  EFI_STATUS                Status;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = REDFISH_CONFIG_LANG_MAP_PRIVATE_FROM_THIS (This);

  Status = SaveConfigLangMapList (&Private->ConfigLangList, Private->VariableName);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, save ConfigLangMap list to variable: %s failed: %r\n", __FUNCTION__, Private->VariableName, Status));
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
RedfishConfigLangMapOnExitBootService (
  IN  EFI_EVENT  Event,
  OUT VOID       *Context
  )
{
  //
  // Memory is about to be released. Keep list into variable.
  //
  RedfishConfigLangMapFlush (&mRedfishConfigLangMapPrivate->Protocol);
}

/**
  Unloads an image.

  @param[in]  ImageHandle           Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
  @retval EFI_INVALID_PARAMETER ImageHandle is not a valid image handle.

**/
EFI_STATUS
EFIAPI
RedfishConfigLangMapDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS  Status;

  if (mRedfishConfigLangMapPrivate != NULL) {

    Status = gBS->UninstallProtocolInterface (
                    mRedfishConfigLangMapPrivate->ImageHandle,
                    &gEdkIIRedfishConfigLangMapProtocolGuid,
                    (VOID*)&mRedfishConfigLangMapPrivate->Protocol
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a, can not uninstall gEdkIIRedfishConfigLangMapProtocolGuid: %r\n", __FUNCTION__, Status));
      ASSERT (FALSE);
    }

    ReleaseConfigLangMapList (&mRedfishConfigLangMapPrivate->ConfigLangList);

    if (mRedfishConfigLangMapPrivate->VariableName != NULL) {
      FreePool (mRedfishConfigLangMapPrivate->VariableName);
    }

    if (mRedfishConfigLangMapPrivate->Event != NULL) {
      gBS->CloseEvent (mRedfishConfigLangMapPrivate->Event);
    }

    FreePool (mRedfishConfigLangMapPrivate);
    mRedfishConfigLangMapPrivate = NULL;
  }


  return EFI_SUCCESS;
}

//
// EDKII_REDFISH_CONFIG_LANG_MAP_PROTOCOL.
//
EDKII_REDFISH_CONFIG_LANG_MAP_PROTOCOL mRedfishConfigLangMapProtocol = {
  RedfishConfigLangMapGet,
  RedfishConfigLangMapSet,
  RedfishConfigLangMapFlush
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
RedfishConfigLangMapDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS    Status;

  mRedfishConfigLangMapPrivate = AllocateZeroPool (sizeof (REDFISH_CONFIG_LANG_MAP_PRIVATE_DATA));
  if (mRedfishConfigLangMapPrivate == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  InitializeListHead (&mRedfishConfigLangMapPrivate->ConfigLangList.Listheader);
  mRedfishConfigLangMapPrivate->VariableName = AllocateCopyPool (StrSize (CONFIG_LANG_MAP_VARIABLE_NAME), CONFIG_LANG_MAP_VARIABLE_NAME);
  if (mRedfishConfigLangMapPrivate->VariableName == NULL) {
    goto ON_ERROR;
  }

  mRedfishConfigLangMapPrivate->ImageHandle = ImageHandle;
  CopyMem (&mRedfishConfigLangMapPrivate->Protocol, &mRedfishConfigLangMapProtocol, sizeof (EDKII_REDFISH_CONFIG_LANG_MAP_PROTOCOL));

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEdkIIRedfishConfigLangMapProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  (VOID*)&mRedfishConfigLangMapPrivate->Protocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a, can not install gEdkIIRedfishConfigLangMapProtocolGuid: %r\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    goto ON_ERROR;
  }

  //
  // Create Exit Boot Service event.
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  RedfishConfigLangMapOnExitBootService,
                  NULL,
                  &gEfiEventExitBootServicesGuid,
                  &mRedfishConfigLangMapPrivate->Event
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Fail to register Exit Boot Service event.", __FUNCTION__));
    goto ON_ERROR;
  }

  //
  // Read existing record from variable.
  //
  Status = InitialConfigLangMapList (&mRedfishConfigLangMapPrivate->ConfigLangList, mRedfishConfigLangMapPrivate->VariableName);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a, Initial ConfigLangMap List: %r\n", __FUNCTION__, Status));
  }

  return EFI_SUCCESS;

ON_ERROR:

  RedfishConfigLangMapDriverUnload (ImageHandle);

  return Status;
}
