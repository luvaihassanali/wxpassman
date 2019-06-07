# wxpassman
Password Manager C++ using wxWidgets

Tested using Arch Linux -required packages sqlite, wxwidget, crypto++ 

Build commands I use:
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/luv/wxWidgets-3.0.4/gtk-build/lib

g++ tbtest.cpp \`wx-config --cxxflags --libs\` -lcryptopp -L/home/luv/wxWidgets-3.0.4/gtk-build -lsqlite3 -o password_manager
```

The application starts minimized in system tray. When you click on icon you will be prompted for key; use the same key each time you start application as this is used to encrypt passwords. If you enter wrong key an error message will show when you try and copy a password and application will exit.
After you enter key on first launch, you will not be prompted untill you log off/in.  You can add new entries or delete using buttons, search through entries in the text bar, or double click entry in list to copy password. When double clicked, the application automatically minimizes and you have 3 seconds before clipboard is cleared to copy your password into desired location. Passwords are encryped then stored in sqlite database 'data.db'. 
