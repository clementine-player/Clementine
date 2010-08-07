NSIS-OS 1.1

Copyright (C) 2001 Robert Rainwater <rrainwater@yahoo.com>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.


OS Version Usage Information
----------------------------
Step 1:
	Call osversion

Step 2:
	The major version is returned to $0 and the minor version is returned to $1.  See below for details on version numbers.


OS Platform Usage Information
-----------------------------
Step 1:
	Call osplatform

Step 2:
	The platform is returned into $0.  Possible values are: "win31", "win9x", "winnt", and "unknown".


Interpreting The Version Numbers
--------------------------------
Major Version:
  Windows 95 		4 
  Windows 98 		4 
  Windows Me 		4 
  Windows NT 3.51 	3 
  Windows NT 4.0 	4 
  Windows 2000 		5 
  Windows XP 		5 
  Windows .NET Server 	5 

Minor Version
  Windows 95 		0 
  Windows 98 		10 
  Windows Me 		90 
  Windows NT 3.51 	51 
  Windows NT 4.0 	0 
  Windows 2000 		0 
  Windows XP 		1 
  Windows .NET Server 	1 



Example
-------
See example.nsi
