# -*- coding: utf-8 -*-
#-------------------------------------------------------------------------#
#   Copyright (C) 2014 by Christoph Thelen                                #
#   doc_bacardi@users.sourceforge.net                                     #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
#-------------------------------------------------------------------------#


#----------------------------------------------------------------------------
#
# Set up the Muhkuh Build System.
#
SConscript('mbs/SConscript')
Import('env_default')

import os.path


#----------------------------------------------------------------------------
#
# Create the compiler environments.
#
env_netx500_default = env_default.CreateCompilerEnv('500', ['arch=armv5te'])
env_netx500_default.Replace(BOOTBLOCK_CHIPTYPE = 500)

env_netx56_default = env_default.CreateCompilerEnv('56', ['arch=armv5te'])
env_netx56_default.Replace(BOOTBLOCK_CHIPTYPE = 56)

env_netx50_default = env_default.CreateCompilerEnv('50', ['arch=armv5te'])
env_netx50_default.Replace(BOOTBLOCK_CHIPTYPE = 50)

env_netx10_default = env_default.CreateCompilerEnv('10', ['arch=armv5te'])
env_netx10_default.Replace(BOOTBLOCK_CHIPTYPE = 10)

Export('env_netx500_default', 'env_netx56_default', 'env_netx50_default', 'env_netx10_default')


#----------------------------------------------------------------------------
#
# Build the platform library.
#
PLATFORM_LIB_CFG_BUILDS = [500, 56, 50, 10]
SConscript('platform/SConscript', exports='PLATFORM_LIB_CFG_BUILDS')


#----------------------------------------------------------------------------
#
# Get the source code version from the VCS.
#
env_default.Version('targets/version/version.h', 'templates/version.h')


#----------------------------------------------------------------------------
#
# Build all sub-projects.
#
SConscript('uarttest/SConscript')
Import('uarttest_netx500', 'uarttest_netx56', 'uarttest_netx50', 'uarttest_netx10')
Import('LUA_UARTTEST')

#----------------------------------------------------------------------------
#
# Build the documentation.
#

# Get the default attributes.
aAttribs = env_default['ASCIIDOC_ATTRIBUTES']
# Add some custom attributes.
aAttribs.update(dict({
	# Use ASCIIMath formulas.
	'asciimath': True,
	
	# Embed images into the HTML file as data URIs.
	'data-uri': True,
	
	# Use icons instead of text for markers and callouts.
	'icons': True,
	
	# Use numbers in the table of contents.
	'numbered': True,
	
	# Generate a scrollable table of contents on the left of the text.
	'toc2': True,
	
	# Use 4 levels in the table of contents.
	'toclevels': 4
}))

doc = env_default.Asciidoc('targets/doc/org.muhkuh.tests.uart.html', 'README.asciidoc', ASCIIDOC_BACKEND='html5', ASCIIDOC_ATTRIBUTES=aAttribs)


#----------------------------------------------------------------------------
#
# Build the artifact.
#

tArcList = env_default.ArchiveList('zip')

tArcList.AddFiles('doc/',
	doc)

tArcList.AddFiles('lua/',
	LUA_UARTTEST)

tArcList.AddFiles('netx/',
	uarttest_netx500,
	uarttest_netx56,
	uarttest_netx50,
	uarttest_netx10)

tArcList.AddFiles('templates/',
        'templates/test.lua')

tArcList.AddFiles('',
	'ivy/org.muhkuh.tests.uart/install.xml')

strArtifactPath = 'targets/ivy/repository/org/muhkuh/tests/uart/%s' % env_default.ArtifactVersion_Get()
tArc = env_default.Archive(os.path.join(strArtifactPath, 'uart-%s.zip' % env_default.ArtifactVersion_Get()), None, ARCHIVE_CONTENTS=tArcList)

env_default.ArtifactVersion(os.path.join(strArtifactPath, 'ivy-%s.xml' % env_default.ArtifactVersion_Get()), 'ivy/org.muhkuh.tests.uart/ivy.xml')


#----------------------------------------------------------------------------
#
# Make a local demo installation.
#
# Copy all binary binaries.
Command('targets/testbench/netx/uarttest_netx10.bin',  uarttest_netx10,  Copy("$TARGET", "$SOURCE"))
Command('targets/testbench/netx/uarttest_netx50.bin',  uarttest_netx50,  Copy("$TARGET", "$SOURCE"))
Command('targets/testbench/netx/uarttest_netx56.bin',  uarttest_netx56,  Copy("$TARGET", "$SOURCE"))
Command('targets/testbench/netx/uarttest_netx500.bin', uarttest_netx500, Copy("$TARGET", "$SOURCE"))

# Copy all LUA scripts.
Command('targets/testbench/lua/uart_test.lua',  LUA_UARTTEST, Copy("$TARGET", "$SOURCE"))

