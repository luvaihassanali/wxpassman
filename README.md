# wxpassman
Password Manager C++ using wxWidgets

Tested using Arch Linux -required packages sqlite, wxwidget, crypto++ 

Build commands I use:
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/luv/wxWidgets-3.0.4/gtk-build/lib

g++ tbtest.cpp \`wx-config --cxxflags --libs\` -lcryptopp -L/home/luv/wxWidgets-3.0.4/gtk-build -lsqlite3 -o password_manager
```
Using Sqllitestudio for linux create your sqlite database "data.db" with table ENTRIES and columns RED (primary), YELLOW, GREEN as TEXT elements and keep it in same location as .o file 
