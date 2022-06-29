#! /usr/local/bin/python3
#
# Redfish JSON resource convert to feature driver source code generator.
#
# (C) Copyright 2021 Hewlett Packard Enterprise Development LP
#

import urllib.request
from urllib import request as urlrequest
import json
import os
import re
import warnings

class HtmlUtils:

    timeout = 4 # Seconds for HTTP timeout

    @staticmethod
    def is_http_path(uri):
        if '://' in uri:
            return True
        else:
            return False

    @staticmethod
    def load_as_json(filename):
        """Load json data from a file, printing an error message on failure."""

        # We will generate an "unversioned" odata URI, which is not a thing that exists,
        # in order to group objects. This is a hack and should be eliminated.
        if '/odata.json' in filename:
            return None

        data = {}
        try:
            # Parse file as json
            jsondata = open(filename, 'r', encoding="utf8")
            data = json.load(jsondata)
            jsondata.close()
        except (OSError, json.JSONDecodeError) as ex:
            warnings.warn('Unable to read ' + filename + ': ' + str(ex))

        return data

    @staticmethod
    def http_load_as_json(uri, config):
        """Load a URI and convert from JSON"""

        #
        # Check if this link has cached.
        #
        CacheDir = config['output_path'] + '\\HttpCache'
        if not os.path.exists(CacheDir):
            os.mkdir(CacheDir)
        Cachefn = uri.split ('/')
        Cachefn = Cachefn [len(Cachefn)-1]
        Cachefn = CacheDir + '\\' + Cachefn

        if os.path.exists(Cachefn):
            with open(Cachefn, 'r',  encoding='UTF-8') as f:
                json_string = f.read()
                json_data = json.loads(json_string)
                return json_data
        try:
            if '://' not in uri:
                uri = 'http://' + uri

            # We will generate an "unversioned" odata URI, which is not a thing that exists,
            # in order to group objects. This is a hack and should be eliminated.
            if 'odata.json' in uri:
                return None

            # Set Proxy
            req = urlrequest.Request(uri)
            if config ['ProxyHost'] != '':
                if config ['ProxyPort'] != '':
                    proxy = config ['ProxyHost'] + ":" + config ['ProxyPort']
                    req.set_proxy (proxy, 'http')
                else:
                    req.set_proxy (config ['ProxyHost'], 'http')
            try:
                f = urlrequest.urlopen(req)
            except Exception as ex:
                warnings.warn("Unable to retrieve data from '" + uri + "': " + str(ex))
                return None
            json_string = f.read().decode('utf-8')
            json_data = json.loads(json_string)

            #
            # Save to cache
            #
            with open(Cachefn, 'w+',  encoding='UTF-8') as f:
                f.write (json_string)
                f.close

            return json_data

        except Exception as ex:
            warnings.warn("Fail to build connection for retrieving data from '" + uri + "': " + str(ex))
            return None