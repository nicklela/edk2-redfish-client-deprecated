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
from .html_util import HtmlUtils

props_to_exclude = [
        "@odata.context",
        "@odata.type",
        "@odata.id",
        "Name",
        "Id",
        "Description",
        "Oem",
        "Links",
        # skip list
        "Actions",
        "PCIeDevices@odata.count",
        "PCIeFunctions@odata.count",
        "Redundancy@odata.count",
        "Status",
        "UUID",
        "RedfishVersion"
]

class SchemaParser:
    @staticmethod
    def find_typename (data):
        """Get the the reference to the structural definition of the resource or resource collection from $ref """
        if data is None:
           return None, None

        if '$ref' in data:
            ref = data['$ref'][1:] # drop initial '#'

        if '$ref' not in data:
            return None, None

        # If there is no definitions block, there's nothing to do:
        if 'definitions' not in data:
            return None, None

        original_ref = ref

        for pathpart in ref.split('/'):
            if not pathpart: continue
            data = data[pathpart]

        schema_typename = pathpart
        schema_data = data

        return schema_typename, schema_data

    @staticmethod
    def parse_unversioned_file(json_data):
        anyOf_data = []

        # is_collection will be True if there is an "Member" in the schema.
        is_collection_of = None

        result = {'root' : is_collection_of,
                  'anyOf' : anyOf_data,
                  'is_collection_of' : is_collection_of
                  }

        # find resource data from $ref
        typename, json_data = SchemaParser.find_typename(json_data)
        if json_data is None: return

        root = typename

        # it means there's the non-versioned definition of a resource:
        if 'anyOf' in json_data:
            for obj in json_data['anyOf']:
                if '$ref' in obj:
                    refpath_uri, refpath_path = obj['$ref'].split('#')
                    if refpath_path == '/definitions/idRef':
                        continue

                    # get versioned resource filename 
                    ref_fn = refpath_uri.split('/')[-1]
                    anyOf_data.append({'filename': ref_fn,
                                       'ref' : refpath_uri })
                    is_collection_of = None

                else:
                    if 'properties' in obj:
                        if 'Members' in obj['properties']:
                            # It's a resource collection. What is it a collection of?
                            member_ref = obj['properties']['Members'].get('items', {}).get('$ref')
                            if member_ref:
                                is_collection_of = SchemaParser.normalize_ref(member_ref)
                                root = root + '_' + is_collection_of.split('/')[-1]

                result.update({ 'root' : root,
                                'is_collection_of' : is_collection_of,
                                'anyOf' : anyOf_data,
                                })
            return result

    @staticmethod
    def parse_versioned_file(json_data, config):
        """Get the the data struct of the resource"""
        property_data = []
        EmptyProperty = False
        # find resource data from $ref
        typename, local_json_data = SchemaParser.find_typename(json_data)

        if local_json_data is None: return

        # it means there's the non-versioned definition of a resource:
        if 'properties' in local_json_data:
            ref_list = []

            if len (local_json_data['properties']) == 0:
                EmptyProperty = True

            for prop_name, prop_info in local_json_data['properties'].items():
                ref_fn = None
                ref_uri = None
                prop_type = None
                prop_item_type = None

                if prop_name in props_to_exclude:
                    continue
                else:
                    # means there's another $ref.
                    if 'anyOf' in prop_info:
                        for obj in prop_info['anyOf']:
                            if '$ref' in obj:
                                if obj['$ref'].startswith("#"):
                                    prop_data, _ = SchemaParser.parse_prop_ref (json_data, obj['$ref'], config)
                                    if prop_data is not None:
                                        if 'enum' in prop_data:
                                            prop_type = ['enum', prop_data.get('type')]
                                        else:
                                            if 'type' in prop_data:
                                                prop_type = prop_data.get('type')

                    for obj_name, obj_data in prop_info.items():
                        if obj_name == '$ref':
                            if '://' in obj_data:
                                refpath_uri, refpath_path = obj_data.split('#')
                                if refpath_path == '/definitions/idRef':
                                    continue
                                ref_fn = refpath_uri.split('/')[-1]
                                ref_uri = refpath_uri

                            elif obj_data.startswith("#/definitions/"):
                                ref_uri = obj_data

                        if obj_name == 'type':
                            prop_type = obj_data

                        if obj_name == "items":
                            prop_item_type = obj_data

                    property_data.append({ 'property_name': prop_name,
                                           'filename': ref_fn,
                                           'ref' : ref_uri,
                                           'property_type': prop_type,
                                           'property_items_type': prop_item_type })
        return property_data, EmptyProperty

    @staticmethod
    def parse_prop_ref (json_data, prop_ref, config):
        property_data = []
        EmptyProperty = False

        if prop_ref is None:
            return property_data, EmptyProperty

        if prop_ref.startswith('#/definitions/'):
            target_prop = prop_ref[len("#/definitions/"):]
            if target_prop in props_to_exclude:
                return property_data, EmptyProperty
        else:
            return property_data, EmptyProperty

        target_data = json_data['definitions'][target_prop]

        if 'properties' in target_data:
            prop_name = None

            if len (target_data['properties']) == 0:
                EmptyProperty = True

            for prop_name, prop_info in target_data['properties'].items():
                ref_fn = None
                ref_uri = None
                prop_type = None
                prop_enum_data = None
                property_items_type = None

                if prop_name in props_to_exclude:
                    continue          
                
                if 'type' in prop_info:
                    # means it's a known type
                    for (obj_name, obj_data) in prop_info.items():
                        if obj_name == 'type':
                            prop_type = obj_data

                        # Items, if present, will have a definition with either an object, a list of types,
                        # or a $ref:
                        if obj_name == 'items':
                            property_items_type = obj_data

                if 'anyOf' in prop_info:
                    # means there's another $ref.
                    for obj in prop_info['anyOf']:
                        if '$ref' in obj:
                            refpath_uri, refpath_path = obj['$ref'].split('#')
                            if refpath_path == '/definitions/idRef':
                                continue
                            # get ref from current schema file
                            elif obj['$ref'].startswith("#"):
                                # ref_uri = refpath_path
                                ref_uri = obj['$ref']
                                prop_data, _ = SchemaParser.parse_prop_ref (json_data, obj['$ref'], config)
                                if 'enum' in prop_data:
                                    prop_type = ['enum', prop_data.get('type')]
                                    prop_enum_data = prop_data.get('enum')
                            else:
                                ref_uri = obj['$ref']

                if '$ref' in prop_info:
                    ref_uri = prop_info.get('$ref')

                # if ref is in other resource path
                if ref_uri is not None:
                    if ref_uri.startswith('http://'):
                        refpath_uri, refpath_path = ref_uri.split('#')
                        ref_prop = '#' + refpath_path

                        ref_json_data = HtmlUtils.http_load_as_json (refpath_uri, config)
                        result = SchemaParser.parse_unversioned_file (ref_json_data)

                        if result == None:
                            continue

                        if result.get('isCollection'):
                            print (result.get('collection_ref'))
                        else:
                            prop_data, _ = SchemaParser.parse_prop_ref (ref_json_data, ref_prop, config)
                            if prop_data is None: continue
                            if 'enum' in prop_data:
                                prop_type = ['enum', prop_data.get('type')]
                                prop_enum_data = prop_data.get('enum')

                property_data.append({ 'property_name': prop_name,
                                        'filename': ref_fn,
                                        'ref' : ref_uri,
                                        'enum_data' : prop_enum_data,
                                        'property_type': prop_type,
                                        'property_items_type': property_items_type
                                        })
                # print('------------------------------------')
                # print('Name:', target_prop + '.' + prop_name)
                # if ref_uri is not None: print('ref:', ref_uri)
                # if prop_type is not None: print('type:', prop_type)
                # print('------------------------------------')

        # Enumerations go into Property Details
        # if 'enum' in target_data:
        #     prop_enum = target_data.get('enum')
        #     prop_type = target_data.get('type')
        #     prop_data = { 'property_name': target_prop,
        #                   'enum_data': prop_enum,
        #                   'property_type': prop_type }
        #     # print('enum_data:', prop_enum)
        #     return target_data
        else:
            return target_data, EmptyProperty

        return property_data, EmptyProperty

    @staticmethod
    def normalize_ref(ref):
        """Get the normalized version of ref we use to index a schema.

        Get the URL part of ref and strip off the protocol."""
        if '#' in ref:
            ref, path = ref.split('#')

        # if '://' in ref:
        #     protocol, ref = ref.split('://')

        return ref