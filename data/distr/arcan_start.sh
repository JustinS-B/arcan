#!/bin/sh
#
# This is an example of a setup- and configure script for running
# arcan as a native display/desktop system. Distribution managers
# are strongly encouraged to take this and tweak to fit to their
# liking.
#
# The most 'important' system- related tuning at this stage is
# preparing the home directory, scanning/ preparing the database
# (arcan_dbscan.sh) and how to deal with related appls. Part of
# the problem right now is that very little is actually writable
# from an appl (by design) but the current state is a bit too
# restrictive, but that we need a 'scratch copy' of available
# appls that should be synched with any updates to individual
# packages (e.g. 'durden' as a standalone package).
#
# The approach we're going for here is that a copy is created
# on first use, and let the update scripts of individual appls
# synch/- copy a possible upgrade in some post-install script.
#
# Since a pattern like /usr/share/arcan (owned by main package)
# with overlay from (example) a durden package can't really
# go into /usr/share/arcan/appl/durden on many distributions,
# we go with a scan-prefix like /usr/share/arcan-appl-xxx and
# copy these into user_basedir/appl/xxx
#
user_basedir=$HOME/.arcan
user_statedir=$HOME/.arcan/states

check_permissions() {

}

setup_homedir() {

}

check_permissions()
setup_homedir()

# argument string:
#

# basic folders:

# permission sanity check
# we need a /dev/dri/cardn- node, with write permissions.
# we need accesss to dev/input/event* or user-defined overrides.
# look for other arcan processes with the same user
# check its open fds and filter card nodes based on that

# check so that an appl- was specified as an argument and that
# this appl is part of the appl- basedir. same with fallback.

