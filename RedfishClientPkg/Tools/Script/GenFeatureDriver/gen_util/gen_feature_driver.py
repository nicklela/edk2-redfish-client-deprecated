#! /usr/local/bin/python3
#
# Redfish JSON resource convert to feature driver source code generator.
#
# (C) Copyright 2021 Hewlett Packard Enterprise Development LP
#

import urllib.request
import json
import os
import re
import warnings
import uuid

from .schema_parse_util import SchemaParser
from .html_util import HtmlUtils

# Common Define
from .gen_common_def import COPYRIGHT
from .gen_common_def import SCHEMA_INFO_COMMENT

# Logger
import ToolLogger
from ToolLogger import TOOL_LOG_NONE
from ToolLogger import TOOL_LOG_TO_CONSOLE
from ToolLogger import TOOL_LOG_TO_FILE
from ToolLogger import TOOL_LOG_SIMPLE_PROGRESS_DOTS

DEBUG = True        # Print Debug
FEATURE_DRIVER = 0  # Type
COLLECTION     = 1
NON_COLLECTION = 2
VERBOSE_INFO   = "[FEATURE_DRIVER] "
ToolLogInformation = ToolLogger.ToolLog (TOOL_LOG_TO_FILE, "RedfishCs.log")

def DEBUG_PrintMessage (Level, Message):
    if DEBUG:
        print(Message)
        ToolLogInformation.LogIt (Level.upper() + Message)

class GenFeatureDriver:
    """
    Generate feature driver class
    """
    def __init__(self, files, config):
        """ Constructor """
        self.files = files
        self.config = config
        self.htmlutil = HtmlUtils()
        self.root = config ['import_from'] # os.path.abspath ("json-schema")
        self.edk2collection_database = []

    @staticmethod
    def Replacekeyworkds (KeyDict, ContentLines):
        for index in range(0, len(ContentLines)):
            for Keyword in KeyDict.keys ():
                if Keyword in ContentLines[index]:
                    ContentLines[index] = ContentLines[index].replace (Keyword, KeyDict[Keyword])
        return ContentLines

    def get_local_absfilepath (self, fname):
        fname = os.path.join (self.root, fname)
        return fname

    def is_in_localfile (self, target_path):
        if '://' in target_path:
            fname = target_path.split('/')[-1]
            target_fname = self.get_local_absfilepath (fname)
        else:
            target_fname = target_path

        if target_fname in self.files:
            return True, target_fname
        else:
            return False, target_path

    def load_filepath (self, target_path):
        local_data = None
        isLocal, target_fname = self.is_in_localfile(target_path)
       
        if isLocal is True: 
            local_data = self.htmlutil.load_as_json (target_fname)
            return local_data
        else:
            local_data = self.htmlutil.http_load_as_json (target_fname, self.config)
            return local_data
    
    def get_collection_list (self):
        return self.edk2collection_database

    def KeyType2HandleType (self, KeyType, KeyItemType):
        handle_type = ''
        IsArray = False
        if isinstance(KeyType, list):
            # if 'null' in KeyType: return
            if 'array' in KeyType:
                if KeyItemType == None:
                    return handle_type, False
                IsArray = True
                if "$ref" in KeyItemType:
                    DEBUG_PrintMessage (VERBOSE_INFO, '$ref in array "Item" is not supported yet.')
                    return handle_type, False
                if "type" not in KeyItemType:
                    DEBUG_PrintMessage (VERBOSE_INFO, '"type" is not defined in "Item" for array type.')
                    return handle_type, False                    
                KeyType, _ = self.KeyType2HandleType (KeyItemType['type'], None)
                return KeyType, True

            for Obj in KeyType:
                if Obj == "integer" or Obj == "number":
                   handle_type = 'Numeric'
                if Obj =="boolean":
                   handle_type = 'Boolean'
                if Obj == "string":
                   handle_type = 'String'
        else:
            if KeyType == "array":
                if KeyItemType == None:
                    return handle_type, None
                else:
                    IsArray = True
                    if "$ref" in KeyItemType:
                        DEBUG_PrintMessage (VERBOSE_INFO, '$ref in array "Item" is not supported yet.')
                        return handle_type, False
                    if "type" not in KeyItemType:
                        DEBUG_PrintMessage (VERBOSE_INFO, '"type" is not defined in "Item" for array type.')
                        return handle_type, False                    
                    KeyType, _ = self.KeyType2HandleType (KeyItemType['type'], None)
                    return KeyType, True

            if KeyType == "integer" or KeyType == "number":
                handle_type = 'Numeric'
            if KeyType == "boolean":
                handle_type = 'Boolean'
            if KeyType == "string":
                handle_type = 'String'

        return handle_type, IsArray

    #
    # When KeyType == None && key_items_type == None:
    #   For the empty property (so-called vague data type)
    #
    def gen_ConsumeResourceCCode (self, fname, KeyValue, KeyType, key_items_type):
        """Gen the the data struct of the process"""
        C_SRC_TAB_SPACE = "  "

        """initial data"""
        content_of_code = ""
        HandleTypeMethod = ""
        HandleTypeValue = ""
        HandleTypeValueNumber = None
        redpath = struct_name = KeyValue
        schema_version_dot = fname.split('.')[1][1:].replace('_','.')
        schema_version = fname.split('.')[1].upper()
        schema_typename = fname.split('.')[0]
        handle_type = ''
        Unsupported_Array_type = False
        
        # print (KeyValue)
        if '.' in KeyValue: 
            struct_name = KeyValue.replace('.','->')
            redpath = KeyValue.replace('.','/')

        # Covert KeyType
        if KeyType == None and key_items_type == None:
            handle_type = "Vague"
            isArray = False
        else:
            handle_type, isArray = self.KeyType2HandleType (KeyType, key_items_type)

        if handle_type == '':
            Unsupported_Array_type = True
        else:
            if handle_type == "Numeric":
                if not isArray:
                    HandleTypeMethod = "ApplyFeatureSettingsNumericType"
                    HandleTypeValue = "(UINTN)*" + schema_typename + "Cs->" + struct_name
                else:
                    HandleTypeMethod = "ApplyFeatureSettingsNumericArrayType"
                    HandleTypeValue = schema_typename + "Cs->" + struct_name

            if handle_type == "Boolean":
                if not isArray:
                    HandleTypeMethod = "ApplyFeatureSettingsBooleanType"
                    HandleTypeValue = "(BOOLEAN)*" + schema_typename + "Cs->" + struct_name
                else:
                    HandleTypeMethod = "ApplyFeatureSettingsBooleanArrayType"
                    HandleTypeValue = schema_typename + "Cs->" + struct_name

            if handle_type == "String":
                if not isArray:
                    HandleTypeMethod = "ApplyFeatureSettingsStringType"
                    HandleTypeValue = schema_typename + "Cs->" + struct_name
                else:
                    HandleTypeMethod = "ApplyFeatureSettingsStringArrayType"
                    HandleTypeValue = schema_typename + "Cs->" + struct_name

            if handle_type == "Vague":               
                HandleTypeMethod = "ApplyFeatureSettingsVagueType"
                HandleTypeValue = "EmptyPropCs->KeyValuePtr"
                HandleTypeValueNumber = "EmptyPropCs->NunmOfProperties"
        
        # Validate empty properties
        KeyValueComment = ""
        if handle_type == "Vague":
            if "RedfishCS_Type_EmptyProp_CS_Data   *EmptyPropCs;" not in self.consume_code_variables:
                self.consume_code_variables.append ("RedfishCS_Type_EmptyProp_CS_Data   *EmptyPropCs;")

            KeyValueComment += C_SRC_TAB_SPACE*2 + "//\n" + \
                               C_SRC_TAB_SPACE*2 + "// Validate empty property.\n" + \
                               C_SRC_TAB_SPACE*2 + "//\n"
            KeyValueComment += C_SRC_TAB_SPACE*2 + "if (BiosCs->Attributes->Prop.BackLink == BiosCs->Attributes->Prop.ForwardLink) {\n" + \
                               C_SRC_TAB_SPACE*3 + "goto ON_RELEASE;\n" + \
                               C_SRC_TAB_SPACE*2 + "}\n" +\
                               C_SRC_TAB_SPACE*2 + "EmptyPropCs = (RedfishCS_Type_EmptyProp_CS_Data *)BiosCs->Attributes->Prop.ForwardLink;\n" + \
                               C_SRC_TAB_SPACE*2 + "if (EmptyPropCs->Header.ResourceType == RedfishCS_Type_JSON) {\n" + \
                               C_SRC_TAB_SPACE*3 + "DEBUG ((DEBUG_ERROR, \"%a, Empty property with RedfishCS_Type_JSON type resource is not supported yet. (%s)\\n\", __FUNCTION__, Private->Uri));\n" + \
                               C_SRC_TAB_SPACE*3 + "goto ON_RELEASE;\n" + \
                               C_SRC_TAB_SPACE*2 + "}\n"

        # Generate structure comment
        KeyValueComment += C_SRC_TAB_SPACE*2 + "//\n" + \
                          C_SRC_TAB_SPACE*2 + "// Find corresponding configure language for collection resource.\n" + \
                          C_SRC_TAB_SPACE*2 + "//\n"
        

        if handle_type == "Vague":  
            content_of_code += C_SRC_TAB_SPACE + "//\n" + \
                               C_SRC_TAB_SPACE + "// Handle " + struct_name.upper() + '->EmptyProperty\n' + \
                               C_SRC_TAB_SPACE + "//\n"
        else:
            content_of_code += C_SRC_TAB_SPACE + "//\n" + \
                               C_SRC_TAB_SPACE + "// Handle " + struct_name.upper() + '\n' + \
                               C_SRC_TAB_SPACE + "//\n"            

        if Unsupported_Array_type:
            DEBUG_PrintMessage (VERBOSE_INFO, 'Unsupported Array')
            content_of_code +=  "//\n"
            content_of_code +=  "// ****** Warning ******\n"
            content_of_code +=  "// Unsupported array type:" + handle_type + "\n"
            content_of_code +=  "//\n\n"

        else:         
            content_of_code +=  C_SRC_TAB_SPACE + "if ("+ schema_typename + "Cs->" + struct_name +" != NULL) {\n" + \
                                KeyValueComment + \
                                C_SRC_TAB_SPACE*2 + "ConfigureLang = GetConfigureLang (" + schema_typename + "Cs->odata_id, " + "\"" + redpath + "\");\n" + \
                                C_SRC_TAB_SPACE*2 + "if (ConfigureLang != NULL) {\n"

            if HandleTypeValueNumber == None:
                content_of_code += C_SRC_TAB_SPACE*3 + "Status = " + HandleTypeMethod + " (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, " + HandleTypeValue + ");\n"
            else:
                content_of_code += C_SRC_TAB_SPACE*3 + "Status = " + HandleTypeMethod + " (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, ConfigureLang, " + HandleTypeValue + ", " + HandleTypeValueNumber + ");\n"

            content_of_code +=  C_SRC_TAB_SPACE*3 + "if (EFI_ERROR (Status)) {\n" + \
                                C_SRC_TAB_SPACE*4 + "DEBUG ((DEBUG_ERROR, \"%a, apply setting for %s failed: %r\\n\", __FUNCTION__, ConfigureLang, Status));\n" + \
                                C_SRC_TAB_SPACE*3 + "}\n\n" + \
                                C_SRC_TAB_SPACE*3 + "FreePool (ConfigureLang);\n" + \
                                C_SRC_TAB_SPACE*2 + "} else {\n" + \
                                C_SRC_TAB_SPACE*3 + "DEBUG ((DEBUG_ERROR, \"%a, can not get configure language for URI: %s\\n\", __FUNCTION__, Private->Uri));\n" + \
                                C_SRC_TAB_SPACE*2 + "}\n" + \
                                C_SRC_TAB_SPACE + "}\n\n"

        return content_of_code

    def gen_ProvisionResourceCCode (self, fname, KeyValue, KeyType, KeyItemsType):
        """Gen the the data struct of the process"""
        C_SRC_TAB_SPACE = "  "

        """initial data"""
        content_of_code = ""
        HandleTypeMethod = ""
        HandleTypeValue = ""
        redpath = struct_name = KeyValue
        schema_version_dot = fname.split('.')[1][1:].replace('_','.')
        schema_version = fname.split('.')[1].upper()
        schema_typename = fname.split('.')[0]
        handle_type = ''
        Unsupported_Array_type = False
 
        # print (KeyValue)
        if '.' in KeyValue: 
            struct_name = KeyValue.replace('.','->')
            redpath = KeyValue.replace('.','/')

        # Covert KeyType
        if KeyType == None and KeyItemsType == None:
            handle_type = "Vague"
            isArray = False
        else:
            handle_type, isArray = self.KeyType2HandleType (KeyType, KeyItemsType)

        if handle_type == '':
            Unsupported_Array_type = True
        else:
            if handle_type == "Numeric":
                if not isArray:
                    HandleTypeMethod = "GetPropertyNumericValue"
                    HandleTypeValue = "NumericValue"
                    HandleCondition = "if (ProvisionMode || *"+ schema_typename + "Cs->" + struct_name +" != *" + HandleTypeValue +") {\n"
                    ConvertMethod = schema_typename + "Cs->" + struct_name + " = " + HandleTypeValue + ";\n" + \
                                C_SRC_TAB_SPACE*4 + "PropertyChanged = TRUE;\n"+ \
                                C_SRC_TAB_SPACE*3 + "}\n"
                    if "INT64                         *NumericValue;" not in self.provision_code_variables:
                        self.provision_code_variables.append ("INT64                         *NumericValue;")
                else:
                    HandleTypeMethod = "GetPropertyNumericArrayValue"
                    HandleTypeValue = "NumericArrayValue"
                    HandleCondition = "if (ProvisionMode || !CompareRedfishNumericArrayValues (" + schema_typename + "Cs->" + struct_name + ", NumericArrayValue, ArraySize)) {\n"
                    ConvertMethod    = "AddRedfishNumericArray (&" + schema_typename + "Cs->" + struct_name + ", NumericArrayValue, ArraySize);\n" + \
                                    C_SRC_TAB_SPACE*4 + "PropertyChanged = TRUE;\n" + \
                                    C_SRC_TAB_SPACE*3 + "}\n"
                    if "INT64                         *NumericArrayValue;" not in self.provision_code_variables:
                        self.provision_code_variables.append ("INT64                         *NumericArrayValue;")
                    if "UINTN                         ArraySize;" not in self.provision_code_variables:
                        self.provision_code_variables.append ("UINTN                         ArraySize;")                        

            if handle_type == "Boolean":
                if not isArray:
                    HandleTypeMethod = "GetPropertyBooleanValue"
                    HandleTypeValue = "BooleanValue"
                    HandleCondition = "if (ProvisionMode || *"+ schema_typename + "Cs->" + struct_name +" != *" + HandleTypeValue +") {\n"
                    ConvertMethod = "IntegerValue = AllocatePool (sizeof (*IntegerValue));\n" + \
                                C_SRC_TAB_SPACE*4 + "if (IntegerValue != NULL) {\n" + \
                                C_SRC_TAB_SPACE*4 + "  *IntegerValue = (BooleanValue ? 0x01 : 0x00);\n" + \
                                C_SRC_TAB_SPACE*4 + "  " + schema_typename + "Cs->" + struct_name + " = IntegerValue;\n" + \
                                C_SRC_TAB_SPACE*4 + "  PropertyChanged = TRUE;\n"+ \
                                C_SRC_TAB_SPACE*3 + "  }\n" + \
                                C_SRC_TAB_SPACE*3 + "}\n"
                    if "BOOLEAN                       *BooleanValue;" not in self.provision_code_variables:
                        self.provision_code_variables.append ("BOOLEAN                       *BooleanValue;")
                    if "INT32                         *IntegerValue;" not in self.provision_code_variables:
                        self.provision_code_variables.append ("INT32                         *IntegerValue;")
                else:
                    HandleTypeMethod = "GetPropertyBooleanArrayValue"
                    HandleTypeValue = "BooleanArrayValue"
                    HandleCondition = "if (ProvisionMode || !CompareRedfishBooleanArrayValues (" + schema_typename + "Cs->" + struct_name + ", BooleanArrayValue, ArraySize)) {\n"
                    ConvertMethod    = "AddRedfishBooleanArray (&" + schema_typename + "Cs->" + struct_name + ", BooleanArrayValue, ArraySize);\n" + \
                                    C_SRC_TAB_SPACE*4 + "PropertyChanged = TRUE;\n" + \
                                    C_SRC_TAB_SPACE*3 + "}\n"
                    if "BOOLEAN                       *BooleanArrayValue;" not in self.provision_code_variables:
                        self.provision_code_variables.append ("BOOLEAN                       *BooleanArrayValue;")
                    if "UINTN                         ArraySize;" not in self.provision_code_variables:
                        self.provision_code_variables.append ("UINTN                         ArraySize;")                        

            if handle_type == "String":
                if not isArray:
                    HandleTypeMethod = "GetPropertyStringValue"
                    HandleTypeValue = "AsciiStringValue"
                    HandleCondition = "if (ProvisionMode || AsciiStrCmp ("+ schema_typename + "Cs->" + struct_name +", " + HandleTypeValue +") != 0) {\n"
                    ConvertMethod = schema_typename + "Cs->" + struct_name + " = " + HandleTypeValue + ";\n" + \
                                C_SRC_TAB_SPACE*4 + "PropertyChanged = TRUE;\n" + \
                                C_SRC_TAB_SPACE*3 + "}\n"
                    if "CHAR8                         *AsciiStringValue;" not in self.provision_code_variables:
                        self.provision_code_variables.append ("CHAR8                         *AsciiStringValue;")                               
                else:
                    HandleTypeMethod = "GetPropertyStringArrayValue"
                    HandleTypeValue = "AsciiStringArrayValue"
                    HandleCondition = "if (ProvisionMode || !CompareRedfishStringArrayValues (" + schema_typename + "Cs->" + struct_name + ", AsciiStringArrayValue, ArraySize)) {\n"
                    ConvertMethod    = "AddRedfishCharArray (&" + schema_typename + "Cs->" + struct_name + ", AsciiStringArrayValue, ArraySize);\n" + \
                                    C_SRC_TAB_SPACE*4 + "PropertyChanged = TRUE;\n" + \
                                    C_SRC_TAB_SPACE*3 + "}\n"
                    if "CHAR8                         **AsciiStringArrayValue;" not in self.provision_code_variables:
                        self.provision_code_variables.append ("CHAR8                         **AsciiStringArrayValue;")
                    if "UINTN                         ArraySize;" not in self.provision_code_variables:
                        self.provision_code_variables.append ("UINTN                         ArraySize;")

            if handle_type == "Vague":
                HandleTypeMethod = "GetPropertyVagueValue"
                HandleTypeValue = "PropertyVagueValues"
                HandleCondition = "if (ProvisionMode || !CompareRedfishPropertyVagueValues (\n" + \
                                    C_SRC_TAB_SPACE*14 + " ((RedfishCS_Type_EmptyProp_CS_Data *)" + schema_typename + "Cs->" + struct_name + "->Prop.ForwardLink)->KeyValuePtr,\n" + \
                                    C_SRC_TAB_SPACE*14 + " ((RedfishCS_Type_EmptyProp_CS_Data *)" + schema_typename + "Cs->" + struct_name + "->Prop.ForwardLink)->NunmOfProperties,\n" + \
                                    C_SRC_TAB_SPACE*14 + " PropertyVagueValues,\n" + \
                                    C_SRC_TAB_SPACE*14 + " VagueValueNumber)) {\n"
                                    
                ConvertMethod = "//\n" + \
                                C_SRC_TAB_SPACE*4 + "// Use the properties on system to replace the one on Redfish service.\n" + \
                                C_SRC_TAB_SPACE*4 + "//\n" + \
                                C_SRC_TAB_SPACE*4 + "FreeEmptyPropKeyValueList (((RedfishCS_Type_EmptyProp_CS_Data *)" + schema_typename + "Cs->" + struct_name + "->Prop.ForwardLink)->KeyValuePtr);\n" + \
                                C_SRC_TAB_SPACE*4 + "((RedfishCS_Type_EmptyProp_CS_Data *)" + schema_typename + "Cs->" + struct_name + "->Prop.ForwardLink)->KeyValuePtr = PropertyVagueValues;\n" + \
                                C_SRC_TAB_SPACE*4 + "((RedfishCS_Type_EmptyProp_CS_Data *)" + schema_typename + "Cs->" + struct_name + "->Prop.ForwardLink)->NunmOfProperties = VagueValueNumber;\n" + \
                                C_SRC_TAB_SPACE*4 + "PropertyChanged = TRUE;\n" + \
                                C_SRC_TAB_SPACE*3 + "}\n"                
                #ConvertMethod = "AddRedfishVagueValues (&" + schema_typename + "Cs->" + struct_name + ", PropertyVagueValues);\n" + \
                #                    C_SRC_TAB_SPACE*4 + "PropertyChanged = TRUE;\n" + \
                #                    C_SRC_TAB_SPACE*3 + "}\n"
                if "RedfishCS_EmptyProp_KeyValue  *PropertyVagueValues;" not in self.provision_code_variables:
                    self.provision_code_variables.append ("RedfishCS_EmptyProp_KeyValue  *PropertyVagueValues;")
                if "UINT32                        VagueValueNumber;" not in self.provision_code_variables:
                    self.provision_code_variables.append ("UINT32                        VagueValueNumber;")
                                
        # Generate structure comment
        content_of_code += C_SRC_TAB_SPACE + "//\n" + \
                           C_SRC_TAB_SPACE + "// Handle " + struct_name.upper() + '\n' + \
                           C_SRC_TAB_SPACE + "//\n"

        if Unsupported_Array_type:
            DEBUG_PrintMessage (VERBOSE_INFO, 'Unsupported Array')
            content_of_code +=  "//"
            content_of_code +=  "// ****** Warning ******\n"
            content_of_code +=  "// Unsupported array type:" + handle_type + "\n"
            content_of_code +=  "//\n\n"

        else:                           
            content_of_code +=  C_SRC_TAB_SPACE + "if (PropertyChecker ("+ schema_typename + "Cs->" + struct_name +", ProvisionMode)) {\n"

            if not isArray:
                if handle_type != "Vague":
                    content_of_code += C_SRC_TAB_SPACE*2 + HandleTypeValue + " = " + HandleTypeMethod + " (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L\"" + redpath +"\", ConfigureLang);\n"
                else:
                    content_of_code += C_SRC_TAB_SPACE*2 + HandleTypeValue + " = " + HandleTypeMethod + " (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L\"" + redpath +"\", ConfigureLang, &VagueValueNumber);\n"                        
            else:
                content_of_code += C_SRC_TAB_SPACE*2 + HandleTypeValue + " = " + HandleTypeMethod + " (RESOURCE_SCHEMA, RESOURCE_SCHEMA_VERSION, L\"" + redpath +"\", ConfigureLang, &ArraySize);\n"

            content_of_code += C_SRC_TAB_SPACE*2 + "if (" + HandleTypeValue + " != NULL) {\n" + \
                                C_SRC_TAB_SPACE*3 + HandleCondition + \
                                C_SRC_TAB_SPACE*4 + ConvertMethod + \
                                C_SRC_TAB_SPACE*2 + "}\n" + \
                                C_SRC_TAB_SPACE + "}\n"
        return content_of_code

    def gen_InitialCsMemberCode (self, fname, member_name, prop_reference_name, externalRefHttp):
        C_SRC_TAB_SPACE = "  "
        schema_version_dot = fname.split('.')[1][1:].replace('_','.')
        schema_version = fname.split('.')[1].upper()
        schema_typename = fname.split('.')[0]
        prop_reference_cs_name = member_name

        if (prop_reference_name is not None and prop_reference_name.startswith('#/definitions/')):
            if prop_reference_cs_name != prop_reference_name[len('#/definitions/'):]:
                prop_reference_cs_name = prop_reference_name[len('#/definitions/'):]

        initial_cs_member_result =  C_SRC_TAB_SPACE + "//\n" + \
                                    C_SRC_TAB_SPACE + "// Handle " + member_name.upper() + '\n' + \
                                    C_SRC_TAB_SPACE + "//\n"
        initial_cs_member_result += C_SRC_TAB_SPACE + "if (" + schema_typename + "Cs->" + member_name + " == NULL) {\n" + \
                                    C_SRC_TAB_SPACE*2 + schema_typename + "Cs->" + member_name + " = AllocateZeroPool (sizeof (Redfish" + schema_typename + "_" + schema_version + "_" + prop_reference_cs_name + "_CS));\n" + \
                                    C_SRC_TAB_SPACE*2 + "ASSERT (" + schema_typename + "Cs->" + member_name + " != NULL);\n" + \
                                    C_SRC_TAB_SPACE + "}\n\n"
        self.hasInitialCsMember = True
        return initial_cs_member_result

    def gen_InitialCsMember (self, fname, member_name, prop_reference_name, externalRefHttp):
        C_SRC_TAB_SPACE = "  "

        if externalRefHttp == None:
            initial_cs_member_result = self.gen_InitialCsMemberCode (fname, member_name, prop_reference_name, externalRefHttp)
        else:
            if externalRefHttp ['has_ref'] == False:
                #initial_cs_member_result = self.gen_InitialCsMemberCode (fname, member_name, prop_reference_name, externalRefHttp)
                initial_cs_member_result =  C_SRC_TAB_SPACE + "//\n" + \
                                            C_SRC_TAB_SPACE + "// " + member_name.upper() + ' is not handled.' + ' Defined in ' + externalRefHttp['HttpUri'] + '\n' + \
                                            C_SRC_TAB_SPACE + "//\n\n"  
            else:
                self.hasInitialCsMember = False
                if externalRefHttp['is_collection_of_result'] != None and externalRefHttp['is_collection_of_result']["is_collection_of"] != None:
                  initial_cs_member_result =  C_SRC_TAB_SPACE + "//\n" + \
                                              C_SRC_TAB_SPACE + "// " + member_name.upper() + ' will be handled by collection driver.\n' + \
                                              C_SRC_TAB_SPACE + "//\n\n"                
                else:
                  initial_cs_member_result =  C_SRC_TAB_SPACE + "//\n" + \
                                              C_SRC_TAB_SPACE + "// " + member_name.upper() + ' will be handled by feature driver.\n' + \
                                              C_SRC_TAB_SPACE + "//\n\n"      
        return initial_cs_member_result

    def gen_ResourceOpenCsMember (self, fname, member_name, externalRefHttp):
        C_SRC_TAB_SPACE = "  "
        schema_version_dot = fname.split('.')[1][1:].replace('_','.')
        schema_version = fname.split('.')[1].upper()
        schema_typename = fname.split('.')[0]

        if self.hasInitialCsMember == False:
            return ""

        open_cs_member =  C_SRC_TAB_SPACE + "//\n" + \
                                    C_SRC_TAB_SPACE + "// Handle " + member_name.upper() + '\n' + \
                                    C_SRC_TAB_SPACE + "//\n"
        open_cs_member +=  C_SRC_TAB_SPACE + "if (" + schema_typename + "Cs->" + member_name + " != NULL) {\n"
        return open_cs_member

    def gen_ResourceCloseCsMember (self, fname, member_name, externalRefHttp):

        if self.hasInitialCsMember == False:
            return ""
                    
        C_SRC_TAB_SPACE = "  "

        open_cs_member =  C_SRC_TAB_SPACE + "}\n\n"
        return open_cs_member

    def gen_ResourceCodeAddTab (self, code, number_tabs):
        C_SRC_TAB_SPACE = "  "
        newCode = ""

        for line in code.splitlines():
            if line != "":
                newCode += C_SRC_TAB_SPACE * number_tabs + line + "\n"
            else:
                newCode += "\n"  
        return newCode

    def gen_HandleCase (self, fname, prop_name, prop_type, prop_items_type):
        consume_content_result = provision_content_result =''

        consume_content_result = self.gen_ConsumeResourceCCode (fname, prop_name, prop_type, prop_items_type)
        provision_content_result = self.gen_ProvisionResourceCCode (fname, prop_name, prop_type, prop_items_type)

        return consume_content_result, provision_content_result

    def handle_prop_ref (self, parent_json_data, prop_name, prop_ref):
        collection_items = []
        non_collection_items = []
        property_items = []

        if (prop_ref.startswith('#/definitions/')):
            return
  
        typename, ___ = SchemaParser.find_typename (parent_json_data)
        local_ref = SchemaParser.normalize_ref (prop_ref)

        # chcek ref is collection path or not.
        local_data = self.load_filepath (local_ref)
        result = SchemaParser.parse_unversioned_file(local_data)

        root_uri_path = typename + '/' + prop_name

        if result is None: return

        if result.get('is_collection_of') is not None:
            collection_uri = local_ref
            isCollectionExist = False
            if self.is_in_localfile (collection_uri):
                fname = collection_uri.split('/')[-1]
                collection_fname = os.path.join (self.root, fname)

                DEBUG_PrintMessage (VERBOSE_INFO, prop_name + ' is collection of ' + collection_uri)
                self.edk2collection_database.append ({'collection_filename': collection_uri,
                                                      'collection_propname': root_uri_path})
        return


    def handle_prop_value (self, fname, json_data):
        """handle property data in current schema file"""
        consume_code_result = ''
        provision_code_result = ''
        self.provision_code_variables = []
        self.consume_code_variables = []
        self.has_etag = False

        property_data = []
        property_data, IsEmptyProperty = SchemaParser.parse_versioned_file (json_data, self.config)

        if property_data is None: return

        for Obj in property_data:
            # Find type and data by $ref path
            if Obj['property_type'] is None:
               prop_name = Obj['property_name']
               prop_ref = Obj['ref']
               prop_items_type = Obj['property_items_type']
               isRefHttp = False
               externalHttpRef = None

               if prop_name == "@odata.etag":
                   self.has_etag = True
                   # Don't generate code for @odata.etag
                   continue

               if prop_ref is None : continue

               if (prop_ref is not None and 
                   prop_ref.startswith('#/definitions/')):
                   DEBUG_PrintMessage (VERBOSE_INFO, '[ '+ fname + ' ] : ' + prop_name + "ref " + prop_ref)
               elif prop_ref.startswith('http://'):
                    isRefHttp = True
                    refpath_uri = prop_ref
                    if '#' in prop_ref:
                        refpath_uri, refpath_path = prop_ref.split('#')                  
                    ref_http_json_data = HtmlUtils.http_load_as_json (refpath_uri, self.config)
                    result = SchemaParser.parse_unversioned_file (ref_http_json_data)

                    if "$ref" in ref_http_json_data:
                        refInHttpJsonData = True
                    else:
                        refInHttpJsonData = False
                    externalHttpRef = {'IsHttp' : isRefHttp,
                                       'HttpUri' : refpath_uri,
                                       'is_collection_of_result' : result,
                                       'has_ref' : refInHttpJsonData
                                      }                

               property_items, isEmptyProperty = SchemaParser.parse_prop_ref (json_data, prop_ref, self.config)

               if property_items is None: continue

               self.hasInitialCsMember = False
               # if there're properties in #/definitions/
               if isinstance (property_items, list):
                    if isEmptyProperty:
                        consume_code_result += self.gen_InitialCsMember (fname, prop_name, prop_ref, None)
                        provision_code_result += self.gen_ResourceOpenCsMember (fname, prop_name, None)
                        content_result, resource_content_result = self.gen_HandleCase(fname, prop_name, None, None)
                        consume_code_result += content_result
                        # Add tab to indent each line of resource_content_result
                        resource_content_result = self.gen_ResourceCodeAddTab (resource_content_result, 1)
                        provision_code_result += resource_content_result
                        provision_code_result += self.gen_ResourceCloseCsMember (fname, prop_name, None)
                    else:
                        consume_code_result += self.gen_InitialCsMember (fname, prop_name, prop_ref, externalHttpRef)
                        provision_code_result += self.gen_ResourceOpenCsMember (fname, prop_name, externalHttpRef)                        
                        for prop_ref_obj in property_items:
                            if prop_ref_obj['ref'] is not None: 
                                self.handle_prop_ref (json_data, prop_name, prop_ref_obj['ref'])

                            if prop_ref_obj['property_type'] is None: continue 

                            if isinstance (prop_ref_obj['property_type'], list):
                                message = '[ '+ fname + ' ] : ' + prop_name + '.'+ prop_ref_obj['property_name'] + ", type {"
                                for listitem in prop_ref_obj['property_type']:
                                    message += listitem + ','
                                message += "}"

                            else:
                                message = '[ '+ fname + ' ] : ' + prop_name + '.'+ prop_ref_obj['property_name'] + ", type " + prop_ref_obj['property_type']\
                        
                            DEBUG_PrintMessage (VERBOSE_INFO, message)

                            if 'enum' in prop_ref_obj['property_type']:
                                message = '[ '+ fname + ' ] : ' + 'enum_data_of' + prop_ref_obj['enum_data'][0]
                                DEBUG_PrintMessage (VERBOSE_INFO, message)

                            def_prop_name = prop_name + '.'+ prop_ref_obj['property_name']
                            def_prop_type = prop_ref_obj['property_type']
                            prop_items_type = prop_ref_obj['property_items_type']

                            content_result, resource_content_result = self.gen_HandleCase(fname, def_prop_name, def_prop_type, prop_items_type)

                            if (content_result is None or 
                                resource_content_result is None): continue

                            consume_code_result += content_result

                            # Add tab to indent each line of resource_content_result
                            resource_content_result = self.gen_ResourceCodeAddTab (resource_content_result, 1)

                            provision_code_result += resource_content_result                                           
                        provision_code_result += self.gen_ResourceCloseCsMember (fname, prop_name, externalHttpRef)

               # if there's no property in /definitions/, means a data structure
               else:
                    if 'enum' in property_items:

                        message = '[ '+ fname + " ] : " + prop_name + "type: enum ," + property_items.get('type') + '\n' + \
                                  '[ '+ fname + ' ] : ' + 'enum_data_of' + property_items['enum'][0]

                        DEBUG_PrintMessage (VERBOSE_INFO, message)
                        prop_type = property_items.get('type')

                        content_result, resource_content_result = self.gen_HandleCase(fname, prop_name, prop_type, prop_items_type)

                        if (content_result is None or 
                            resource_content_result is None): continue

                        consume_code_result += content_result
                        provision_code_result += resource_content_result

            # if prop with a known type
            else:
                prop_name = Obj['property_name']
                prop_type = Obj['property_type']
                prop_items_type = Obj['property_items_type']

                DEBUG_PrintMessage (VERBOSE_INFO, '[ '+ fname + " ] : " + prop_name + ", type: " + prop_type[0])
                
                content_result, resource_content_result = self.gen_HandleCase(fname, prop_name, prop_type, prop_items_type)

                if (content_result is None or 
                    resource_content_result is None): continue

                consume_code_result += content_result
                provision_code_result += resource_content_result

        return consume_code_result, provision_code_result

    def gen_Edk2CommonHeader (self, schema_typename, schema_version, parent_prop_name):
        content = ''
        schema_info_prefix = "#define RESOURCE_SCHEMA"

        content += COPYRIGHT
        content += "#ifndef EFI_REDFISH_" + schema_typename.upper() + "_COMMON_H_\n"
        content += "#define EFI_REDFISH_" + schema_typename.upper() + "_COMMON_H_\n\n"

        content += "#include <RedfishJsonStructure/" + schema_typename + '/' + schema_version + '/' + 'Efi' + schema_typename + schema_version.upper() + ".h>\n"
        content += "#include <RedfishResourceCommon.h>\n\n"
        content += SCHEMA_INFO_COMMENT
        content += schema_info_prefix + '         "' + schema_typename + '"\n'
        content += schema_info_prefix + '_MAJOR   "' + schema_version[1] + '"\n'
        content += schema_info_prefix + '_MINOR   "' + schema_version[3] + '"\n'
        content += schema_info_prefix + '_ERRATA  "' + schema_version[5] + '"\n'
        content += schema_info_prefix + '_VERSION "v' + schema_version[1:] + '"\n'
        content += '#define REDPATH_ARRAY_PATTERN   ' + 'L"/' + parent_prop_name + '/\\\\{.*\\\\}/"' + '\n'
        content += '#define REDPATH_ARRAY_PREFIX    ' + 'L"/' + parent_prop_name + '/"\n'
        content += schema_info_prefix + '_FULL    "'  + 'x-uefi-redfish-' + schema_typename + '.' + schema_version + '"\n'
        content += '\n#endif'

        return content

    def gen_Edk2CommonDriver (self, edk_ouput_path, schema_typename, schema_version, schema_process_code, schema_resource_code, parent_prop_name):
        content_lines = ''
        # Find common edk2 feature driver template
        edk2_template_path = self.config['edk2_template_path']

        # Common
        edk2redfish_feature_driver_common = edk_ouput_path + os.path.normpath("/Common")
        if not os.path.exists (edk2redfish_feature_driver_common):
            os.makedirs(edk2redfish_feature_driver_common)

        # Create resource Common header
        fname = schema_typename + "Common.h"
        file_path = os.path.join (edk2redfish_feature_driver_common, fname)
        fo = open(file_path,"w")
        fo_content_lines = self.gen_Edk2CommonHeader(schema_typename, schema_version, parent_prop_name)
        fo.writelines (fo_content_lines)
        fo.close()

        file_path = edk2_template_path + os.path.normpath("/RedfishFeatureCommon.temp")
        fi = open(file_path,"r")
        content_lines = fi.readlines()
        fi.close()

        # Keyword list
        Edk2CommonDriver_KeywordsDict = {}

        common_path = '#include "' + schema_typename + 'Common.h"'
        common_schema_title = '#' + schema_typename + '.' + schema_version + '.' + schema_typename
        common_schema_namespace = '  EFI_REDFISH_' + schema_typename.upper() + '_' + schema_version.upper() + "     *"+ schema_typename + ';\n' + \
                                 '  EFI_REDFISH_' + schema_typename.upper() + '_' + schema_version.upper() + '_CS' + "  *"+ schema_typename + 'Cs;'

        schema_resource_assignment = '  ' + schema_typename + 'Cs = ' + schema_typename + '->' + schema_typename + ';'

        Edk2CommonDriver_KeywordsDict["!**EDK2_COMMON_DRIVER_HEADER_CONTENT**!"] = common_path
        Edk2CommonDriver_KeywordsDict["!**EDK2_RESOURCE_TITLE**!"]               = common_schema_title
        Edk2CommonDriver_KeywordsDict["!**EDK2_RESOURCE_NAMESPACE**!"]           = common_schema_namespace
        Edk2CommonDriver_KeywordsDict["!**EDK2_RESOURCE_ASSIGNMENT**!"]          = schema_resource_assignment
        Edk2CommonDriver_KeywordsDict["!**EDK2_RESOURCE_TYPE**!"]                = schema_typename        
        Edk2CommonDriver_KeywordsDict["!**EDK2_RESOURCE_PROP_HANDLE_CODE**!"]    = schema_process_code
        Edk2CommonDriver_KeywordsDict["!**EDK2_RESOURCE_PROP_PROVISION_CODE**!"] = schema_resource_code

        if self.has_etag:
            Edk2CommonDriver_KeywordsDict["!**EDK2_FEATURE_DRIVER_CHECK_ETAG**!"] = schema_typename + "Cs->odata_etag"
        else:
            Edk2CommonDriver_KeywordsDict["!**EDK2_FEATURE_DRIVER_CHECK_ETAG**!"] = "NULL"

        variable_declaration = ""
        for variable in self.provision_code_variables:
            variable_declaration += ("  " + variable + "\n")
        variable_declaration += "\n"
        Edk2CommonDriver_KeywordsDict["!**EDK2_PROVISION_VARIABLE_DECLARATION**!"] = variable_declaration

        variable_declaration = ""
        for variable in self.consume_code_variables:
            variable_declaration += ("  " + variable + "\n")
        Edk2CommonDriver_KeywordsDict["!**EDK2_CONSUME_VARIABLE_DECLARATION**!"] = variable_declaration

        # Output Common driver
        fname = schema_typename + "Common.c"
        file_path = os.path.join (edk2redfish_feature_driver_common, fname)
        fo = open(file_path,"w")
        fo_content_lines = GenFeatureDriver.Replacekeyworkds(Edk2CommonDriver_KeywordsDict, content_lines)
        fo.writelines (fo_content_lines)
        fo.close()

        return 


    def gen_Edk2RedfishFeatureFile (self, versioned_fname, schema_process_code, schema_resource_code, parent_prop_name):
        """Generate Edk2 Redfish feature driver code"""
        template_filelist = []
        output_path = self.config['output_path']

        if output_path is None: return

        # Create output folder
        edk2feature_source_dir = os.path.normpath(output_path + "/RedfishFeatureDrivers")
        if not os.path.exists (output_path):                
            os.makedirs(edk2feature_source_dir)

        # Create folder of target schema
        schema_version = versioned_fname.split('.')[1]
        schema_typename = versioned_fname.split('.')[0]

        # Feature driver/Common 
        edk2redfish_feature_driver = edk2feature_source_dir + os.path.normpath("/" + schema_typename + "/" + schema_version)
        if not os.path.exists (edk2redfish_feature_driver):
            os.makedirs(edk2redfish_feature_driver)

        # Generate EDK2 Common Driver
        self.gen_Edk2CommonDriver (edk2redfish_feature_driver, schema_typename, schema_version, schema_process_code, schema_resource_code, parent_prop_name)

        # Skip PEI driver for now.
        # PEI
        #edk2redfish_feature_driver_pei = edk2redfish_feature_driver + os.path.normpath("/Pei")
        #if not os.path.exists (edk2redfish_feature_driver_pei):
        #    os.makedirs(edk2redfish_feature_driver_pei)

        # DXE
        edk2redfish_feature_driver_dxe = edk2redfish_feature_driver + os.path.normpath("/Dxe")
        if not os.path.exists (edk2redfish_feature_driver_dxe):
            os.makedirs(edk2redfish_feature_driver_dxe)

        # Find EDK2 driver template
        edk2_template_path = self.config['edk2_template_path']
        edk2feature_component_filelist = self.config['edk2_component_filelist']

        for root, _, filenames in os.walk(edk2_template_path):
            for fname in filenames:
                if (fname.find('Common') > -1 or 
                    fname.find('Collection') > -1): 
                    continue

                file_path = os.path.join(root, fname)
                fi = open(file_path,"r")
                content_lines = fi.readlines()
                fi.close()
                template_obj = {'_edk2_path': fname,
                                '_content_lines' : content_lines }
                template_filelist.append(template_obj)

        # Replace keyword dict
        Edk2FeatureDriver_KeywordsDict = {}
        common_driver_name = schema_typename + 'Common'
        feature_driver_basename = schema_typename + 'Dxe'

        Edk2FeatureDriver_KeywordsDict["!**EDK2_FEATURE_DRIVER_BASENAME**!"]   = feature_driver_basename
        Edk2FeatureDriver_KeywordsDict["!**EDK2_FEATURE_DRIVER_FILEGUID**!"]   = str(uuid.uuid4())
        Edk2FeatureDriver_KeywordsDict["!**EDK2_COMMON_DRIVER_NAME**!"]        = common_driver_name
        Edk2FeatureDriver_KeywordsDict["!**EDK2_RESOURCE_TYPE**!"]             = schema_typename

        # Skip PEI driver for now.
        #for Obj in template_filelist:
        #    if Obj['_edk2_path'].find('Dxe') > -1: fname = schema_typename + "Pei.c"
        #    if Obj['_edk2_path'].find('Inf') > -1: fname = schema_typename + "Pei.inf"
        #
        #    file_path = os.path.join (edk2redfish_feature_driver_pei, fname)
        #    fo = open(file_path,"w")
        #    fo_content_lines = GenFeatureDriver.Replacekeyworkds(Edk2FeatureDriver_KeywordsDict, Obj['_content_lines'])
        #    fo.writelines (fo_content_lines)
        #    fo.close()

        for Obj in template_filelist:
            if Obj['_edk2_path'].find('Dxe') > -1: fname = schema_typename + "Dxe.c"
            if Obj['_edk2_path'].find('Inf') > -1: fname = schema_typename + "Dxe.inf"

            file_path = os.path.join (edk2redfish_feature_driver_dxe, fname)
            fo = open(file_path,"w")
            fo_content_lines = GenFeatureDriver.Replacekeyworkds(Edk2FeatureDriver_KeywordsDict, Obj['_content_lines'])
            fo.writelines (fo_content_lines)
            fo.close()

        for fname in edk2feature_component_filelist:
            file_path = os.path.join(output_path, fname)
            if not os.path.exists (file_path):
               fo = open(file_path,"w")
               fo.close()
        return 