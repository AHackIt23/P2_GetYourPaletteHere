Instruction to run code in Github VS Code workspace:

- open command terminal
- execute these commads:
    sudo apt-get install -y libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev
    sudo apt-get update && sudo apt-get install -y libsdl2-dev
    sudo apt-get update && sudo apt-get install -y xvfb x11vnc novnc fluxbox
    Xvfb :1 -screen 0 1280x720x24 & export DISPLAY=:1 && fluxbox & x11vnc -display :1 -nopw -forever & websockify --web=/usr/share/novnc/ 0.0.0.0:6080 localhost:5900 &
    rm -rf build/
    cmake -B build
    cmake --build build
    export DISPLAY=:1
    export XDG_RUNTIME_DIR=/tmp/runtime-root
    chmod 700 $XDG_RUNTIME_DIR
    ./build/P2_GetYourPaletteHere
-next, select "PORTS" tab next to "TERMINAL" 
-hover over the 6080 port and select the globe icon to open the program through vnc webpage software
-select the connect button
-click to the left of Tester and type src/tester.jpg or src/tester2.jpg to test the program functionality
//running the program from your individual hardware is necessary forthe drag and drop functionality
- type Ctrl + C to stop anything running in terminal when done
-then execute:
    pkill -f Xvfb; pkill -f fluxbox; pkill -f x11vnc; pkill -f websockify; pkill -f novnc
    //to safely close open ports and close open windows manually
    