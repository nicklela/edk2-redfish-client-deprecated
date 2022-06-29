#
# Redfish JSON resource convert to feature driver source code generator.
#
# (C) Copyright 2021 Hewlett Packard Enterprise Development LP
#

COPYRIGHT  = "/** @file\n\n" + \
                "  Redfish feature driver implementation - internal header file\n" + \
                "  (C) Copyright 2020-2021 Hewlett Packard Enterprise Development LP<BR>\n" + \
                "  SPDX-License-Identifier: BSD-2-Clause-Patent\n\n" \
                "**/\n\n"

SCHEMA_INFO_COMMENT = "//\n" + \
                      "// Schema information.\n" + \
                      "//\n"

TABLE_END = "=======================================================================\n"