#include <iostream>
#include <csignal>
#include <proc/readproc.h>
#include <cstring>
#include <array>
#include <vector>
#include <sstream>

bool killByPID(int pid) {
    int res = kill(pid, SIGKILL);
    if (res != 0) {
        return false;
    }

    return true;
}

bool killByName(const std::string& name) {
    bool killedOne = false;
    PROCTAB* proc = openproc(PROC_FILLCOM | PROC_FILLSTAT);
    proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_info));
    while (readproc(proc, &proc_info) != nullptr) {
        if (*proc_info.cmdline == name) {
            int res = kill(proc_info.ppid, SIGKILL);
            if (res != 0) {
                return false;
            }
            killedOne = true;
        }
    }
    closeproc(proc);

    return killedOne;
}

std::vector<std::string> parseEnvNames(const char* nameRaw) {
    std::vector<std::string> res;
    std::string namesStr(nameRaw);
    std::stringstream names(namesStr);
    std::string singleName;

    while(std::getline(names, singleName, ',')) {
        res.push_back(singleName);
    }

    return res;
}

bool parse_args(int argc, char** argv, std::string& raw_id, std::string& name) {
    if (argc != 3 && argc != 5) {
        return false;
    }

    bool idSet;
    bool nameSet;
    if (std::string(argv[1]) == "--id") {
        raw_id = argv[2];
        name = "";
        idSet = true;
    } else if (std::string(argv[1]) == "--name") {
        name = argv[2];
        raw_id = "";
        nameSet = true;
    } else {
        return false;
    }

    if (argc == 3) {
        return true;
    }

    if (std::string(argv[3]) == "--name" && !nameSet) {
        name = argv[4];
    } else if (std::string(argv[3]) == "--id" && !idSet) {
        raw_id = argv[4];
    } else {
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    std::string raw_id;
    int id = 0;
    std::string name;

    if (!parse_args(argc, argv, raw_id, name)) {
        std::cout << "Usage: killer" << " --id <id> | --name <name>" << std::endl;
        return 1;
    }

    if (!raw_id.empty()) {
        try {
            id = std::stoi(raw_id);
        } catch (std::exception& e) {
            std::cout << "Invalid argument: " << e.what() << std::endl;
            return 1;
        }
    }

    bool killed = false;
    if (id != 0) {
        killed = killByPID(id);
        if (killed) {
            std::cout << "Process with pid " << id << " killed." << std::endl;
        } else {
            std::cout << "Process with pid " << id << " was not killed." << std::endl;
        }
    }

    if (!name.empty()) {
        killed = killByName(name);
        if (killed) {
            std::cout << "Process with name " << name << " killed." << std::endl;
        } else {
            std::cout << "Process with name " << name << " was not killed." << std::endl;
        }
    }

    char* envNames = getenv("PROC_TO_KILL");
    std::vector<std::string> toKill = parseEnvNames(envNames);

    std::cout << "Parsing env variable..." << std::endl;
    for(const std::string& k : toKill) {
        killed = killByName(k);
        if (killed) {
            std::cout << "Process with name " << k << " killed." << std::endl;
        } else {
            std::cout << "Process with name " << k << " was not killed." << std::endl;
        }
    }

    return 0;
}
