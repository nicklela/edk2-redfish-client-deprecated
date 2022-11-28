/** @file
  This file defines the Redfish event library interface.

  (C) Copyright 2022 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef REDFISH_ADDENDUM_LIB_H_
#define REDFISH_ADDENDUM_LIB_H_

#include <Uefi.h>
#include <Library/JsonLib.h>
#include <Protocol/EdkIIRedfishResourceAddendumProtocol.h>

/**
  Provising redfish resource with with addendum data in given schema.

  @param[in]   Uri              Uri of input resource.
  @param[in]   Schema           Redfish schema string.
  @param[in]   Version          Schema version string.
  @param[in]   JsonText         Input resource in JSON format string.
  @param[out]  JsonWithAddendum The input resource with addendum value attached.

  @retval EFI_SUCCESS              Addendum data is attached.
  @retval EFI_UNSUPPORTED          No addendum data is required in given schema.
  @retval Others                   Some error happened.

**/
EFI_STATUS
RedfishGetAddendumData (
  IN     EFI_STRING Uri,
  IN     CHAR8 *Schema,
  IN     CHAR8 *Version,
  IN     CHAR8 *JsonText,
  OUT    CHAR8 **JsonWithAddendum
  );

/**
  Provising redfish OEM resource with given schema information.

  @param[in]   Uri             Uri of input resource.
  @param[in]   Schema          Redfish schema string.
  @param[in]   Version         Schema version string.
  @param[in]   JsonText        Input resource in JSON format string.
  @param[out]  JsonWithOem     The input resource with OEM value attached.

  @retval EFI_SUCCESS              OEM data is attached.
  @retval EFI_UNSUPPORTED          No OEM data is required in given schema.
  @retval Others                   Some error happened.

**/
EFI_STATUS
RedfishGetOemData (
  IN  EFI_STRING Uri,
  IN  CHAR8 *Schema,
  IN  CHAR8 *Version,
  IN  CHAR8 *JsonText,
  OUT CHAR8 **JsonWithOem
  );

#endif
