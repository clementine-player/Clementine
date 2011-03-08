libportfwd
----------
A basic, lightweight c++ wrapper around miniupnp and nat-pmp libraries
for setting up port fwds, detecting max up/downstream bandwidth, and 
finding out external IP address.

See: http://miniupnp.free.fr/

Designed to wrap up miniupnpc+natpmp libs into a static lib with a small API
so other projects can easily setup port fwds without shipping extra libs/deps.

Should detect any upnp or nat-pmp router and automatically use the appropriate
library under the hood.

Uses cmake to build needed bits of miniupnpc..

NB/TODO
-------
I don't have a nat-pmp capable device (eg: airport express)
so haven't implemented that bit yet. only supports upnp atm.

Usage
-----
See the demo in main.cpp, but here's the jist:

 Portfwd pf;
 pf.init(2000);   // 2000 = ms to wait for response from router
 pf.add(1234);    // port to fwd to you
 pf.remove(1234); // remove port fwding on exit

