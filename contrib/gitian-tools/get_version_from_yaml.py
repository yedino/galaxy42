#!/usr/bin/env python

# usage example: $ python3 get_version_from_yaml.py galaxy-linux.yml

import yaml
import sys

with open(sys.argv[1], 'r') as full:
    try:
        galaxy_yaml = yaml.load(full)
        url = galaxy_yaml["remotes"][0]["url"]
        commit = galaxy_yaml["remotes"][0]["commit"]
        print(commit,"on",url)

    except yaml.YAMLError as exc:
        print(exc)
