Here is my app building template, to quickly draw some stuff on the screen.

It's my honor if this project can help more people :)

Building
--------

As a cmake user, you can write following code to build your app:

`cmake -B build -DCINDER_MAC=1 -DCMAKE_BUILD_TYPE=Release -G Ninja`

then:

`cmake --build build -j8`

remember check the CmakeLists.txt in root directory to see the more platform variables.