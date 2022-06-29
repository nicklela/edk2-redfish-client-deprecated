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

class GenCollectionDriver:
    """
    Generate collection driver class
    """
    def __init__(self, files, config):
        """ Constructor """
        self.files = files
        self.config = config
        self.htmlutil = HtmlUtils()
        #self.root = os.path.abspath ("json-schema")
        self.root = config ['import_from']

    @staticmethod
    def Replacekeyworkds (KeyDict, ContentLines):
        for index in range(0, len(ContentLines)):
            for Keyword in KeyDict.keys ():
                if Keyword in ContentLines[index]:
                    ContentLines[index] = ContentLines[index].replace (Keyword, KeyDict[Keyword])
        return ContentLines

    def gen_Edk2CollectionDriver (self, collection_typename, res_path_list):
        """Generate Edk2 Redfish collection driver code"""
        template_filelist = []
        output_path = self.config['output_path']

        if output_path is None: return

        #collection_path = 'Redfish' + collection_typename + 'Dxe'
        collection_path = collection_typename + 'Dxe'

        # Create output folder
        edk2feature_source_collection_dir = os.path.normpath(output_path + "/RedfishFeatureDrivers/" + collection_path)
        if not os.path.exists (edk2feature_source_collection_dir):                
            os.makedirs(edk2feature_source_collection_dir)

        # Find EDK2 driver template
        edk2_collection_template_path = self.config['edk2_template_path'] + '/CollectionTempFiles'

        for root, _, filenames in os.walk(edk2_collection_template_path):
            for fname in filenames:
                file_path = os.path.join(root, fname)
                fi = open(file_path,"r")
                content_lines = fi.readlines()
                fi.close()
                template_obj = {'_edk2_path': fname,
                                '_content_lines' : content_lines }
                template_filelist.append(template_obj)

        Edk2CollectionDriver_KeywordsDict = {}
        Edk2CollectionDriver_KeywordsDict["!**EDK2_COLLECTION_TYPENAME**!"] = collection_typename
        Edk2CollectionDriver_KeywordsDict["!**EDK2_RESOURCE_TYPENAME**!"] = collection_typename.replace('Collection', '')
        Edk2CollectionDriver_KeywordsDict["!**EDK2_RESOURCE_TYPENAME_CAPITAL**!"] = collection_typename.replace('Collection', '').upper()
        Edk2CollectionDriver_KeywordsDict["!**EDK2_COLLECTION_TYPENAME_CAPITAL**!"] = collection_typename.upper()
        Edk2CollectionDriver_KeywordsDict["!**EDK2_COLLECTION_DRIVER_FILEGUID**!"] = str(uuid.uuid4())
        res_path = ''
        for index in range (0, len(res_path_list)):
            #
            # Remove "ServiceRoot" because we don't have edk2 feature driver manages
            # ServiceRoot schema.
            #
            res_path_instance = res_path_list[index]
            if "ServiceRoot/" in res_path_instance:
                res_path_instance = res_path_instance [len("ServiceRoot/"):]
            res_path += res_path_instance + ';'
        res_path = res_path [:len(res_path)-1]
        #
        # Remove the end collection symbol. Because we only
        # keep the property name of the collection this driver manages.
        #
        #new_res_path = ''
        #for res_path_instance in res_path.split(';'):
        #    if res_path_instance.endswith ('/{}'):
        #        res_path_instance = res_path_instance [:len(res_path_instance)-len('/{}')]
        #    new_res_path = new_res_path + res_path_instance +';'
        #new_res_path = new_res_path[:len(new_res_path)-1]
        Edk2CollectionDriver_KeywordsDict["!**EDK2_REDFISH_RESOURCE_URI**!"] = res_path # build resource uri path

        for Obj in template_filelist:
            if Obj['_edk2_path'].find('CollectionDxe') > -1: fname = collection_typename + "Dxe.c"
            if Obj['_edk2_path'].find('CollectionInclude') > -1: fname = collection_typename + "Dxe.h"
            if Obj['_edk2_path'].find('CollectionInf') > -1: fname = collection_typename + "Dxe.inf"

            file_path = os.path.join (edk2feature_source_collection_dir, fname)
            fo = open(file_path,"w+")
            fo_content_lines = GenCollectionDriver.Replacekeyworkds(Edk2CollectionDriver_KeywordsDict, Obj['_content_lines'])
            fo.writelines (fo_content_lines)
            fo.close()
        return