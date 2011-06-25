------------------------------
Building echoprint-codegen-ios
------------------------------

0. If you don't have it, get [boost](http://www.boost.org/)
   **Note:** echoprint-codegen only uses boost headers for some numeric operations, 
   so you don't need to compile boost.
    
   
1. Open `echoprint-codegen-ios.xconfig` and set up your boost include directory, e.g., 
    
    HEADER_SEARCH_PATHS = /Users/artgillespie/dev/src/boost_1_46_1

2. Build!

3. If you get a bunch of build errors, goto 2.

-------------------------------------------
Using echoprint-codegen-ios in your iOS app
-------------------------------------------

Check out the sample app!
