#include <iostream>
#include <csignal>
#include <cstring>
#include <array>
#include <vector>
#include <sstream>

int main(int argc, char** argv) {
    setenv("PROC_TO_KILL", "nano,chrome", true);

    system("./killer --id 2566 --name nano > output.txt");

    unsetenv("PROC_TO_KILL");
}
