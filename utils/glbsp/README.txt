
README for glBSP
================

by Andrew Apted.  4th September 2000.


Introduction
------------

glBSP is a nodes builder specially designed to be used with OpenGL
ports of the DOOM game engine. It adheres to the "GL-Friendly Nodes"
specification, which means it adds some new special nodes to a WAD
file that makes it very easy (and fast !) for an OpenGL DOOM engine to
compute the polygons needed for drawing the levels.

There are many DOOM ports that understand the GL Nodes which glBSP
creates, including: EDGE, the Doomsday engine (JDOOM), and Doom3D.
See the links below.


Status
------

The current version of glBSP is 1.91.  It has been tested and known to
work on numerous large wads, including DOOM I shareware, DOOM II,
TeamTNT's Eternal III, and beta versions of Fanatic's QDOOM.

NOTE: Version 1.91 introduces support for "V2 GL Nodes".  This allows
      the BSP information to be stored with greater precision,
      preventing problems like "slime trails".  It may take a while
      before all DOOM ports that understand GL Nodes are updated to
      read the new format.  Until they do, use the -v1 option to
      generate the old format.


Copyright
---------

glBSP is Copyright (C) 2000 Andrew Apted.  It is based on `BSP 2.3'
(C) Colin Reed and Lee Killough, which was created from the basic
theory stated in DEU5 (OBJECTS.C) by Raphael Quinet.

All trademarks are the propriety of their owners.


License
-------

Thanks to Lee Killough and André Majorel (the previous and current
maintainers of BSP, respectively), glBSP is now under the GNU General
Public License (GPL).  See the file `COPYING' in the source package
(or go to http://www.gnu.org) for the full text, but to summarise:

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.


Contact
-------

Questions, bug reports, suggestsions, etc... about glBSP should be
directed to the glBSP public forum which can be found here:

   http://sourceforge.net/forum/?group_id=10592

Alternatively you could join the glbsp-public mailing list.  Details
about doing this can be found here:

   http://sourceforge.net/mail/?group_id=10592


Links
-----

The homepage for glBSP can be found here:
   http://glbsp.sourceforge.net/

The "GL Friendly Nodes" specifications can be found here:
   http://glbsp.sourceforge.net/glnodes.html

The EDGE homepage can be found here:
   http://edge.sourceforge.net/

The JDOOM homepage is here:
   http://www.newdoom.com/jdoom

The Doom3D site can be found here:
   http://www.redrival.com/fudgefactor/doom3d.html


Acknowledgements
----------------

Andy Baker, for making binaries and stuff.

Marc A. Pullen, for testing and helping with the documentation.

Lee Killough and André Majorel, for giving their permission to put
glBSP under the GNU GPL.

Jaakko Keränen, who gave some useful feedback on the "GL Friendly
Nodes" specification.

Marc Rousseau (author of ZenNode 1.0), Robert Fenske Jr (author of
Warm 1.6), L.M. Witek (author of Reject 1.1), and others, for
releasing the source code to their WAD utilities, and giving me lots
of ideas to "borrow" :), like blockmap packing.

Colin Reed and Lee Killough (and others), who wrote the original BSP
2.3 which glBSP is based on.

Matt Fell, for the Doom Specs v1.666.

Raphael Quinet, for DEU and the original idea.

id Software, for not only creating such an irresistable game, but
releasing the source code for so much of their stuff.

... and everyone else who deserves it ! 

