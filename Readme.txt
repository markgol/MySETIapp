

MySETIapp, a set tools for decoding bitstreams into various formats and manipulating those files
MySETIapp.cpp
(C) 2023, Mark Stegall
Author: Mark Stegall

This file is part of MySETIapp.

MySETIapp is free software : you can redistribute it and /or modify it under
the terms of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

MySETIapp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with MySETIapp.
If not, see < https://www.gnu.org/licenses/>. 

Background:
From the 'A Sign in Space' website, https://asignin.space/:
A Sign in Space is an interdisciplinary project by media artist Daniela de Paulis,
in collaboration with the SETI Institute, the European Space Agency,
the Green Bank Observatory and INAF, the Italian National Institute for Astrophysics.
The project consists in transmitting a simulated extraterrestrial message as part
of a live performance, using an ESA spacecraft as celestial source.The objective
of the project is to involve the world - wide Search for Extraterrestrial
Intelligence community, professionals from different fieldsand the broader public
in the reception, decodingand interpretation of the message.This process will
require global cooperation, bridging a conversation around the topics of SETI,
space researchand society, across multiple culturesand fields of expertise.
https://www.seti.org/event/sign-space

The message was transmitted from the ESA's ExoMars Trace Gas Orbiter (TGO)
  on May 24 at 19:16 UTC/12:15 pm PDT.

It was received by three radio telescopes on earth May 24,2023.
A group of individuals in the Discord 'A Sign in Space' channel
unscrambled the message from the radio telemetry.
The message published as Data17.bin is the correctly transcribed
bitstream of the message payload given to ESA.

The next step in the problem is the decoding of the payload bitstream into
the next level of the message.

The types of programs that the Discord group is using is quite varied like:
Excel, Photoshop, GIMP, Java, Python and c/c++.  A number of
people are also using online tools that use file uploads, typically text based.
Excel uses text files for import like CSV.
Photoshop and GIMP can use raw binary files typically 8 bit, 16 bit or 32 bit per element.
None of these use bit packed binary input. So the first step is to translate the
bitstream message into a format that can be used by the various tools. The next
steps involve examination of the data to solve how to decode it.


The programs is a set of tools to help in this process.

These covers many of the requests people have had in the Discord group

Version History
V1.0.0.1 2023-08-20, Initial Release.
					 Limitations, external viewer must be used to open image/BMP file
					 (original viewer was leftover from the 'bitblt' days and was removed)

Beta V1.1.0.1	plans
				implement a WIC based iamge viewer
Language:
c/c++

OS:
Microsoft Windows, versions compatible with using Visual Studio 2019

IDE environment:
Visual Studio 2019

To build:
1. Open Visual Studio
2. Open MySETIapp solution, MySETIapp.sln
3. Select Debug or Release, x64 for build configuration
4. In menu Build -> Build solution or Build -> Build MySETIapp

Files:
readme.txt		This file
COPYING.txt		GNU GPL V3.0 or later license file

Aboutlg.cpp			About dialog box source under menu Help
BitDialogs.cpp		Dialog box sources for the menu selections under the
					menu item Bit tools
BitStream.cpp		Bit stream function used by bit tools dialogs
BitStream.h			function prototypes for functions in BitStream.cpp
FileFunctions.cpp	File menu dialogs and common file functions
FileFunctions.h		function prototypes for functions in FileFunctions.cpp
framework.h			include file for standard system include files
Globals.h			reference protypes for globals variable
Imaging.cpp			Image tools menu
Imaging.h			function prototypes for functions in Imaging.cpp
ImagingDialogs.cpp	Dialog box sources for the menu selections under the
					menu item Image tools
MySETIapp.cpp		Main windows program, entry point
MySETIapp.h			include file for main program referencing resource.h
MySETIapp.ico		MySETIapp icon file, full complement of sizes
MySETIapp.rc		resource definitions for MySETIapp, dialogs, menus, version, icons, etc
Resource.h			ID definitions used in MySETIapp.rc
Settings.cpp		Properties menu
targetver.h			Defines the target version of Windows (use latest)
