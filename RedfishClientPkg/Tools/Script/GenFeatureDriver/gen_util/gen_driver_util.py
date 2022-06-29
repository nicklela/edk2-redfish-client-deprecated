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

# Gen EDK2 Driver Libs
from .schema_parse_util import SchemaParser
from .html_util import HtmlUtils
from .gen_collection_driver import GenCollectionDriver
from .gen_feature_driver import GenFeatureDriver

# Common Define
from .gen_common_def import COPYRIGHT
from .gen_common_def import SCHEMA_INFO_COMMENT
from .gen_common_def import TABLE_END

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
VERBOSE_INFO   = "[INFO] "
ToolLogInformation = ToolLogger.ToolLog (TOOL_LOG_TO_FILE, "RedfishCs.log")

def DEBUG_PrintMessage (Level, Message):
    if DEBUG:
        print(Message)
        ToolLogInformation.LogIt (Level.upper() + Message)

class GenFeatureDriverUtil:

    def __init__(self, files, config):
        """ Constructor """

        self.files = files
        self.config = config
        self.htmlutil = HtmlUtils()
        # self.root = os.path.abspath ("json-schema")
        self.root = config ['import_from']
        self.edk2driver_database = []
        self.edk2collection_database = []
        self._collection_driver = GenCollectionDriver(files, config)
        self._feature_driver = GenFeatureDriver(files, config)

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
            DEBUG_PrintMessage (VERBOSE_INFO, 'Load ' + target_fname + ' from local drive.')
            local_data = self.htmlutil.load_as_json (target_fname)
            return local_data
        else:
            DEBUG_PrintMessage (VERBOSE_INFO, 'Load ' + target_fname + ' through HTTP.')            
            local_data = self.htmlutil.http_load_as_json (target_fname, self.config)
            return local_data

    def check_driver_list(self, data, type=0, collection_root_uri_path = ''):
        isExist = False

        if type == FEATURE_DRIVER:
            for Obj in self.edk2driver_database: 
                if (data == Obj['driver_depex'] or data == Obj['driver_fname']):
                    isExist = True
                    # DEBUG_PrintMessage (VERBOSE_INFO, data + ' is exist...')
                    break

        if type == COLLECTION:
            #for Obj in self.edk2collection_database: 
                #if (data == Obj['driver_depex'] or data == Obj['driver_fname']):
                #    isExist = True
                #    # DEBUG_PrintMessage (VERBOSE_INFO, data + ' is exist...')
                #    break
            for index in range (0, len (self.edk2collection_database)):
                if data == self.edk2collection_database[index]['driver_fname']:
                    isExist = True
                    obj = self.edk2collection_database[index]
                    if collection_root_uri_path not in obj['driver_depex']:
                        self.edk2collection_database[index]['driver_depex'].append (collection_root_uri_path)
                        break
        return isExist

    def get_versioned_list (self, local_data):
        """get versioned file list from collection or non colleciton"""
        target_fname = None
        versioned_f_list =[]

        if local_data is None: return

        result = SchemaParser.parse_unversioned_file (local_data)
        typename, _ = SchemaParser.find_typename (local_data)

        ref_uri = result.get('is_collection_of')

        if ref_uri is not None:
            local_data = self.load_filepath (ref_uri)
            result = SchemaParser.parse_unversioned_file (local_data)

        if result.get('anyOf') is not None:
            anyOf_items = result.get('anyOf')

            for Obj in anyOf_items:
                # ref_uri = Obj['ref']
                # ref_filename = os.path.join (self.root, Obj['filename'])

                isLocal, __ = self.is_in_localfile(Obj['ref'])
                if isLocal is True:
                    ref_filename = os.path.join (self.root, Obj['filename'])
                    versioned_f_list.append (ref_filename)
                else:
                    versioned_f_list.append (Obj['ref'])

        return versioned_f_list

    def extract_prop_info (self, json_data):
        """find collection in current schema file"""
        collection_list = []
        non_collection_list = []
        property_data_list = []

        # must a versioned ServiceRoot schema file 
        property_data, IsEmptyProperty = SchemaParser.parse_versioned_file(json_data, self.config)

        if property_data is None: return

        for Obj in property_data:
            prop_name = Obj['property_name']
            prop_ref = Obj['ref']
            prop_filename = Obj['filename']
            prop_type = Obj['property_type']

            # print(Obj)

            if prop_type is not None: 
                property_data_list.append (prop_name)
                continue

            # only handle collection
            if (prop_ref is None or 
                prop_ref.startswith('#/definitions/')): 
                continue

            local_data = self.load_filepath (prop_ref)

            # check it is collection or not
            result = SchemaParser.parse_unversioned_file(local_data)

            if result is None: continue

            # print(result)
            collection_ref = result.get('is_collection_of')

            if collection_ref is not None:
                print(Obj['filename'])
                collection_list.append({'collection_filename': Obj['ref'],
                                        'collection_propname': prop_name
                                        })
            else:
                non_collection_list.append({'non_collection_filename': Obj['ref'],
                                             'non_collection_propname': prop_name
                                            })

        return collection_list, non_collection_list, property_data_list

    def extract_collection_list (self, collection_list):
        for CollectionObj in collection_list: 
            isExist = False
            member_list = []
            collection_uri = CollectionObj['collection_filename']
            collection_prop_name = CollectionObj['collection_propname']

            local_data = self.load_filepath(collection_uri)
            member_list = self.get_versioned_list(local_data)

            typename, _ = SchemaParser.find_typename (local_data)
            depex_typename = typename
            # build uri path
            root_uri_path = collection_prop_name + '{}'

            # workaround
            if self.check_driver_list (collection_uri, COLLECTION, root_uri_path): 
                continue
            else:
                DEBUG_PrintMessage (VERBOSE_INFO, root_uri_path)
                # Gen EDK2 Collection DXE Driver
                local_name, _ = SchemaParser.find_typename (local_data)
                obj = {'driver_fname': collection_uri,
                       'driver_depex': [],
                       'local_name' : local_name
                       }
                obj['driver_depex'].append (root_uri_path)                       
                self.edk2collection_database.append (obj)
                #self._collection_driver.gen_Edk2CollectionDriver (local_name, root_uri_path)

            for member_fname in member_list:
                if member_fname is None : continue

                if self.check_driver_list (member_fname, FEATURE_DRIVER): 
                    continue
                else:
                    self.edk2driver_database.append ({'driver_fname': member_fname,
                                                        'driver_depex': root_uri_path,
                                                        'exist' : False})
                    DEBUG_PrintMessage (VERBOSE_INFO, '   ' + member_fname)

        DEBUG_PrintMessage (VERBOSE_INFO, ' End =====================================\n')
        return

    def extract_collection_ref (self, collection_files_name):
        """This function is for finding the latest verison"""
        versioned_files_res = self.extract_unversioned_file (collection_files_name)

        if len (versioned_files_res) == 0: return

        for versioned_file_name in versioned_files_res:
            # print(versioned_files_name)
            _, _, fname = versioned_file_name.rpartition(os.sep)
            fname_parts = fname.split('.')

            # if there's no target resource version in config 
            if fname_parts[0] not in self.config: 
                target_fname = versioned_file_name
                continue

            # if target resource version in config 
            schema_version = self.config[fname_parts[0]]
            if fname.find(schema_version) > -1:
                target_fname = versioned_file_name

        # print(target_fname)
        return target_fname

    def extract_versioned_file (self, versioned_fname, depex_uri):
        """it is an versioned file"""
        local_process_code = ""

        DEBUG_PrintMessage (VERBOSE_INFO, 'opening....' + versioned_fname)
        if '://' in versioned_fname:
            local_data = self.load_filepath (versioned_fname)
        else:
            local_data = self.load_filepath (versioned_fname)

        typename, _ = SchemaParser.find_typename (local_data)

        collection_res, non_collection_res, prop_data_res = self.extract_prop_info (local_data)

        depex_typename = typename
        if depex_uri is not None: depex_typename = depex_uri

        # dump data
        DEBUG_PrintMessage (VERBOSE_INFO, 'depex: ' + depex_typename)

        ################################################################################
        # This's a resource collection list, try to extract all of versioned resource. #
        # Generate a Collection driver                                                 #
        ################################################################################
        DEBUG_PrintMessage (VERBOSE_INFO, '[' + versioned_fname + '] Collection driver list:\n')
        DEBUG_PrintMessage (VERBOSE_INFO, '[' + versioned_fname + '] ==========================================\n')
        for CollectionObj in collection_res:
            isExist = False
            member_list = []
            collection_uri = CollectionObj['collection_filename']
            collection_prop_name = CollectionObj['collection_propname']

            local_data = self.load_filepath(collection_uri)
            member_list = self.get_versioned_list(local_data)

            # build uri path
            root_uri_path = depex_typename + '/' + collection_prop_name + '/{}'

            # BUGBUG: 
            #    StorageSystems also refers to ComputerSystem schema. This
            #    causes the error when we parse Systems.
            if collection_prop_name == 'StorageSystems':
                continue

            if self.check_driver_list (collection_uri, COLLECTION, root_uri_path): 
                continue
            else:
                DEBUG_PrintMessage (VERBOSE_INFO, root_uri_path)
                # Find Collection and generate EDK2 Collection DXE Driver
                local_name, _ = SchemaParser.find_typename (local_data)
                obj = {'driver_fname': collection_uri,
                       'driver_depex': [],
                       'local_name' : local_name
                      }
                obj['driver_depex'].append (root_uri_path)
                self.edk2collection_database.append (obj)
                #self._collection_driver.gen_Edk2CollectionDriver (local_name, root_uri_path)

            for member_fname in member_list:
                if member_fname is None : continue

                if self.check_driver_list (member_fname, FEATURE_DRIVER): 
                    continue
                else:
                    self.edk2driver_database.append ({'driver_fname': member_fname,
                                                        'driver_depex': root_uri_path,
                                                        'exist' : False})
                    DEBUG_PrintMessage (VERBOSE_INFO, '   ' + member_fname)

        DEBUG_PrintMessage (VERBOSE_INFO, '[' + versioned_fname + '] End =====================================\n')
        ################################################################################
        # This's a non collection list, try to extract all of versioned resource.      #
        # Generate feature driver                                                      #
        ################################################################################
        DEBUG_PrintMessage (VERBOSE_INFO, '[' + versioned_fname + '] Non Collection driver list:\n')
        DEBUG_PrintMessage (VERBOSE_INFO, '[' + versioned_fname + '] ==========================================\n')
        for NonCollectionObj in non_collection_res:
           
            member_list = []
            non_collection_uri = NonCollectionObj['non_collection_filename']
            non_collection_prop = NonCollectionObj['non_collection_propname']
            # build uri path
            root_uri_path = depex_typename + '/' + non_collection_prop

            if self.check_driver_list (root_uri_path, FEATURE_DRIVER): 
                continue
            else:
                DEBUG_PrintMessage (VERBOSE_INFO, root_uri_path)

            local_data = self.load_filepath(non_collection_uri)
            ref_list = self.get_versioned_list(local_data)

            for ref_fname in ref_list:
                isExist = False
                if ref_fname is None : continue

                if self.check_driver_list (ref_fname, FEATURE_DRIVER): 
                    continue
                else:
                    self.edk2driver_database.append ({'driver_fname': ref_fname,
                                                      'driver_depex': root_uri_path,
                                                      'exist' : False})
                    DEBUG_PrintMessage (VERBOSE_INFO, '   ' + ref_fname)

        DEBUG_PrintMessage (VERBOSE_INFO, '[' + versioned_fname + '] End =====================================\n')
        return 

    def run(self):
        #
        # Step 1: extract versioned file from config['RootSchema']
        #          
        # If cache enabled
        if self.config['CacheDatabase'] == True:
            CacheFile = os.path.join (self.config['output_path'], self.config['RootSchema'] + '_FeatureDriverDatabase.json')
            if os.path.exists(CacheFile):
                self.edk2driver_database = json.load(open(CacheFile, "r", encoding='UTF-8'))
    
        if len(self.edk2driver_database) == 0:
            fname = self.config ['RootSchema']
            versioned_files_name = os.path.join (self.root, fname)
            self.extract_versioned_file(versioned_files_name, None)

            DEBUG_PrintMessage (VERBOSE_INFO, 'Extract versioned file from ' + self.config['RootSchema'])
            for Obj in self.edk2driver_database:
                # print(Obj['driver_fname'])
                self.extract_versioned_file(Obj['driver_fname'], Obj['driver_depex'])

        #
        # Add the additional versioned schemas which are not referred in the root schema. (self.config ['RootSchema']).
        #
        additionalList = self.config ["AdditionalFeatureDriver"]
        for additionalInstance in additionalList:
            self.edk2driver_database.append ({'driver_fname': additionalInstance ['driver_fname'],
                                              'driver_depex': additionalInstance ['driver_depex'],
                                              'exist' : additionalInstance ['exist']})
            self.edk2driver_database[len(self.edk2driver_database) - 1]['type_name'] = additionalInstance ['type_name']
        #
        # Step 2: Start Gen EDK2 Feature driver
        #         Generate collection driver database
        #
        if not self.config["SkipFeatureDriver"]:
            DEBUG_PrintMessage (VERBOSE_INFO, 'Start Gen EDK2 Feature driver list')
            #for Obj in self.edk2driver_database:            
            for index in range(0, len(self.edk2driver_database)):
                if 'type_name' not in self.edk2driver_database[index]:
                    self.edk2driver_database[index]['type_name'] = ""
                Obj = self.edk2driver_database[index]
                # Initial type name
                DEBUG_PrintMessage (VERBOSE_INFO, Obj['driver_fname'])
                versioned_files_name = Obj['driver_fname']

                if '://' in versioned_files_name:
                    fname = versioned_files_name.split('/')[-1]
                else:
                    _, _, fname = versioned_files_name.rpartition(os.sep)

                genDriver = False
                if len (self.config ["GenVersionedFeatureDriver"]) != 0 and self.config['GenSelectedFeatureDriver'] == True:
                    for featureDriver in self.config ["GenVersionedFeatureDriver"]:
                        if 'type_name' in Obj and Obj['type_name'] != "":
                            if featureDriver == Obj['type_name']:
                                genDriver = True
                                break
                    if not genDriver:
                        continue                    

                local_json = self.load_filepath(versioned_files_name)
                typename, _ = SchemaParser.find_typename (local_json)
                self.edk2driver_database[index]['type_name'] = typename

                consume_code, provision_code = self._feature_driver.handle_prop_value (fname, local_json)

                # GEt property name from driver_depex
                parent_prop_name = Obj['driver_depex'].split ("/")
                parent_prop_name = parent_prop_name[len(parent_prop_name)-1]
                parent_prop_name = parent_prop_name.replace('{}', '')
                self._feature_driver.gen_Edk2RedfishFeatureFile (fname, consume_code, provision_code, parent_prop_name)

        # If cache enabled
        if self.config['CacheDatabase'] == True:
            CacheFile = os.path.join (self.config['output_path'], self.config['RootSchema'] + '_FeatureDriverDatabase.json')
            if not os.path.exists(CacheFile):
                with open(CacheFile, 'w+',  encoding='UTF-8') as f:
                    f.write (json.dumps (self.edk2driver_database, indent=4, ensure_ascii=False))
                    f.close                   

        #
        # Step 3: Start generate remaining collection database
        #
        UseCacheCollectionDb = False
        if self.config['CacheDatabase'] == True:
            CacheFile = os.path.join (self.config['output_path'], self.config['RootSchema'] + '_CollectionDriverDatabase.json')
            if os.path.exists(CacheFile):
                self.edk2collection_database = json.load(open(CacheFile, "r", encoding='UTF-8'))
                UseCacheCollectionDb = True

        if UseCacheCollectionDb == False:
            collection_list = self._feature_driver.get_collection_list()
            self.extract_collection_list(collection_list)

        #fname = "AttributeRegistry.json"
        #unversioned_files_name = os.path.join (self.root, fname)

        #local_data = self.load_filepath(unversioned_files_name)
        #ref_list = self.get_versioned_list(local_data)

        #for ref_fname in ref_list:
        #    if ref_fname is None : continue
        #    self.edk2driver_database.append({'driver_fname': ref_fname,
        #                                     'driver_depex': "ComputerSystem",
        #                                      'exist' : False })
        #    self.extract_versioned_file(Obj['driver_fname'], Obj['driver_depex'])

        # If cache enabled
        if self.config['CacheDatabase'] == True:
            CacheFile = os.path.join (self.config['output_path'], self.config['RootSchema'] + '_CollectionDriverDatabase.json')
            if not os.path.exists(CacheFile):
                with open(CacheFile, 'w+',  encoding='UTF-8') as f:
                    f.write (json.dumps (self.edk2collection_database, indent=4, ensure_ascii=False))
                    f.close                             
        
        #
        # Step 4: Generate collection driver source code
        #
        if not self.config["SkipCollectionDriver"]:
            if len(self.config["GenVersionedFeatureDriver"]) == 0 or self.config['GenSelectedCollectionDriver'] == False:
                for obj in self.edk2collection_database:
                    DEBUG_PrintMessage (VERBOSE_INFO, "Generate collection driver: " + obj['local_name'])
                    self._collection_driver.gen_Edk2CollectionDriver (obj['local_name'], obj['driver_depex'])
            else:
                for obj in self.edk2collection_database:
                    for collectiondriver in self.config["GenCollectionDriver"]:                    
                        if collectiondriver == obj ['local_name']:
                            DEBUG_PrintMessage (VERBOSE_INFO, "Generate collection driver: " + obj['local_name'])
                            self._collection_driver.gen_Edk2CollectionDriver (obj['local_name'], obj['driver_depex'])                    
        

        # # list remainders
        # file_list = [os.path.abspath(filename) for filename in self.files]
        # edk2driver_list = []
        # output_path = os.path.abspath ("output/RedfishFeatureDrivers")
        # for root, dirname, filenames in os.walk(output_path):
        #     edk2driver_list = dirname
        #     break;

        # # print (edk2driver_list)

        # for filename in file_list:
        #     # Get the (probably versioned) filename, and save the data:
        #     root, _, fname = filename.rpartition(os.sep)
        #     schema_typename = fname.split('.')[0]

        #     if schema_typename.find('Collection') >= 0:
        #        schema_typename = schema_typename + 'Dxe'

        #     if schema_typename not in edk2driver_list:
        #        edk2driver_list.append (schema_typename)
        #        print (fname)

        return