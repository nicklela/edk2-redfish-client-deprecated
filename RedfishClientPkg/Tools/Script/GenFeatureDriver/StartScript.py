#
# Redfish JSON resource convert to feature driver source code generator.
#
# (C) Copyright 2021 Hewlett Packard Enterprise Development LP
#
import os
import sys
import logging
import re
import uuid
import shutil
import json
import ToolLogger
import http.client
import textwrap
import argparse
import warnings

from gen_util import GenFeatureDriverUtil

# Format user warnings simply
class InfoWarning(UserWarning):
    """ A warning class for informational messages that don't need a stack trace. """
    pass

def simple_warning_format(message, category, filename, lineno, file=None, line=None):
    """ a basic format for warnings from this program """
    if category == InfoWarning:
        return '  Info: %s' % (message) + "\n"
    else:
        return '  Warning: %s (%s:%s)' % (message, filename, lineno) + "\n"

class GenFeatureCode:
    """ Redfish feature driver Generator class. """

    def __init__(self, import_from, config):
        self.import_from = import_from
        self.config = config

    @staticmethod
    def parse_command_line():

        help_description = 'Feature driver.\n\n'
        help_epilog = ('Example:\n   Gen_Feature.py --config="./config.json"\n')
        parser = argparse.ArgumentParser(description=help_description,
                                            epilog=help_epilog,
                                            formatter_class=argparse.RawDescriptionHelpFormatter)

        parser.add_argument('--config', dest="config_file",
                            help=('Path to a config file, containing configuration '
                                    ' in JSON format. '
                                    ))
        parser.add_argument('--logfile', dest="log_file",
                            help=('Path to a Log file, containing configuration '
                                    ' in JSON format. '
                                    ))

        command_line_args = vars(parser.parse_args())
        return command_line_args.copy()

    @staticmethod
    def parse_config_file(config_fn):
        """ Attempt to open and parse a config file, which should be a JSON file. Returns a dictionary. """

        config_file_read = False
        config_data = {}
        try:
            with open(config_fn, 'r', encoding="utf8") as config_file:
                config_data = json.load(config_file)
                config_file_read = True
        except (OSError) as ex:
            warnings.warn('Unable to open ' + config_fn + ' to read: ' + str(ex))
            sys.exit()
        except (json.decoder.JSONDecodeError) as ex:
            warnings.warn(config_fn + " appears to be invalid JSON. JSON decoder reports: " + str(ex))
            sys.exit()

        return config_data

    @staticmethod
    def get_files(import_dir):
        """From text input (command line or parsed from file), create a list of files. """
        files_to_process = []
        if os.path.isdir(import_dir): 
            
            for root, _, files in os.walk(import_dir):
                files.sort(key=str.lower)
                for filename in files:
                    if filename[-4:] == 'json':
                        files_to_process.append(os.path.join(root, filename))
        elif os.path.isfile(import_dir):
            files_to_process.append(import_dir)
        else:
            warnings.warn('Oops, ' + import_dir + ' not found, or contains no .json files.\n')
        return files_to_process

    def startScript(self):
        files = self.get_files(self.import_from)
        files_to_process = [os.path.abspath(filename) for filename in files]

        util_obj = GenFeatureDriverUtil(files_to_process, self.config)
        util_obj.run()
        return

def main():
    command_line_args = GenFeatureCode.parse_command_line()

    if command_line_args.get('config_file'):
        config = GenFeatureCode.parse_config_file(command_line_args['config_file'])
    else:
        config = {}

    output_path = config['output_path']

    if output_path is None: return
    if not os.path.exists (output_path):
        os.makedirs(output_path)

    gen_obj = GenFeatureCode(config['import_from'], config)
    gen_obj.startScript()
    
    return

if __name__ == '__main__':
    main()
    print ('end')
