#!/usr/bin/python

import json
import os
import subprocess
import sys
import tempfile
from of_daemon import *
from of_util import *
from optparse import OptionParser

if sys.version < '2.5':
    sys.stderr.write("You need Python 2.5 or newer.)\n")
    sys.exit(1)

parser = OptionParser()
parser.add_option("-c", "--cluster-config", dest="cluster_conf")
parser.add_option("-9", "--kill-9", action="store_true", dest="kill9")
(opts, args) = parser.parse_args()
if opts.cluster_conf == None:
    sys.stderr.write("you must give a Redfish cluster configuration file\n")
    sys.exit(1)

jo = load_conf_file(opts.cluster_conf)

diter = DaemonIter.from_conf_object(jo, None)
i = 0
for d in diter:
    i = i + 1
    print "processing daemon %d" % i
    print d.jo
    try:
        pid = d.run_with_output("cat " + d.get_pid_file())
        print "daemon " + str(i) + " is running as process " + pid
    except CalledProcessError, e:
        print "daemon " + str(i) + " is NOT running."
