#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <set>

using namespace std;

// Simple Logger class for printing messages with severity tags
class Logger {
public:
    enum class Level { DEBUG, INFO, WARNING, ERROR };

    // Log a message with given severity level
    static void log(Level level, const string& message) {
        string tag;
        switch (level) {
            case Level::DEBUG: tag = "[DEBUG]"; break;
            case Level::INFO: tag = "[INFO]"; break;
            case Level::WARNING: tag = "[WARNING]"; break;
            case Level::ERROR: tag = "[ERROR]"; break;
        }
        cout << tag << " " << message << endl;
    }
};

// Simple CPU Emulator class
class SimpleCPU {
private:
    unordered_map<string, int> registers;  // CPU registers (AX, BX, CX, DX)
    vector<int> memory;                    // Simulated memory of 64KB size
    vector<string> instructions;           // Loaded program instructions
    int pc = 0;                            // Program counter to track current instruction
    set<int> modifiedMemoryAddresses;      // Set of memory addresses modified during execution

public:
    // Constructor: Initialize registers and allocate memory
    SimpleCPU() {
        registers["AX"] = 0;
        registers["BX"] = 0;
        registers["CX"] = 0;
        registers["DX"] = 0;
        memory.resize(65536, 0);  // 64KB memory initialized to 0
    }

    // Load program instructions from a file
    void loadProgram(const string& filename) {
        instructions.clear();  // Clear previous instructions
        pc = 0;               // Reset program counter
        ifstream file(filename);
        string line;

        // Read each line from file and store as an instruction
        while (getline(file, line)) {
            if (!line.empty()) {
                instructions.push_back(line);
            }
        }

        Logger::log(Logger::Level::INFO, "Program loaded with " + to_string(instructions.size()) + " instructions.");
    }

    // Get value of an operand: register, memory address or constant integer
    int getOperandValue(const string& operand) {
        // Check if operand is memory address e.g. [100]
        if (!operand.empty() && operand.front() == '[' && operand.back() == ']') {
            int addr = stoi(operand.substr(1, operand.size() - 2));  // Extract address number
            return memory[addr];  // Return value stored at memory address
        }
        // Check if operand is a register
        else if (registers.count(operand)) {
            return registers[operand];  // Return register value
        }
        else {
            // Otherwise, treat operand as a constant integer
            return stoi(operand);
        }
    }

    // Set value to a destination operand (register or memory)
    void setOperandValue(const string& dest, int value) {
        // If destination is memory address
        if (!dest.empty() && dest.front() == '[' && dest.back() == ']') {
            int addr = stoi(dest.substr(1, dest.size() - 2));  // Extract address
            memory[addr] = value;                              // Store value in memory
            modifiedMemoryAddresses.insert(addr);              // Track modified address
        }
        // If destination is register
        else if (registers.count(dest)) {
            registers[dest] = value;  // Set register value
        }
    }

    // Run the loaded program instructions
    void run() {
        Logger::log(Logger::Level::INFO, "Starting execution...");

        // Loop until end of instructions or HLT encountered
        while (pc < (int)instructions.size()) {
            string line = instructions[pc++];  // Fetch next instruction
            istringstream iss(line);
            string cmd, op1, op2;

            iss >> cmd >> op1;

            // Remove trailing comma if present in op1
            if (!op1.empty() && op1.back() == ',') {
                op1.pop_back();
            }

            iss >> op2;  // Read second operand (if any)

            Logger::log(Logger::Level::DEBUG, "Executing: " + cmd + " " + op1 + (op2.empty() ? "" : ", " + op2));

            // Execute instructions based on command
            if (cmd == "MOV") {
                setOperandValue(op1, getOperandValue(op2));  // MOV dest, src
            }
            else if (cmd == "ADD") {
                // Calculate sum of operands and store in AX register
                int result = getOperandValue(op1) + getOperandValue(op2);
                registers["AX"] = result;
            }
            else if (cmd == "SUB") {
                // Subtract second operand from first and store in AX
                int result = getOperandValue(op1) - getOperandValue(op2);
                registers["AX"] = result;
            }
            else if (cmd == "MUL") {
                // Multiply operands and store in AX
                int result = getOperandValue(op1) * getOperandValue(op2);
                registers["AX"] = result;
            }
            else if (cmd == "DIV") {
                int divisor = getOperandValue(op2);
                if (divisor == 0) {
                    Logger::log(Logger::Level::ERROR, "Division by zero error.");
                    continue;  // Skip division by zero
                }
                int result = getOperandValue(op1) / divisor;
                registers["AX"] = result;
            }
            else if (cmd == "HLT") {
                Logger::log(Logger::Level::INFO, "Program halted.");
                break;  // Stop execution
            }
            else {
                Logger::log(Logger::Level::ERROR, "Unknown instruction: " + cmd);
            }
        }

        // Print final CPU state after execution
        printState();
    }

    // Print registers and modified memory contents
    void printState() {
        cout << "\nRegister Values:\n";
        for (const auto& reg : registers) {
            cout << reg.first << " = " << reg.second << endl;
        }

        cout << "\nModified Memory Locations:\n";
        for (int addr : modifiedMemoryAddresses) {
            cout << "[" << addr << "] = " << memory[addr] << endl;
        }
    }
};

int main() {
    SimpleCPU cpu;                // Create CPU instance
    cpu.loadProgram("program.txt");  // Load program from file "program.txt"
    cpu.run();                   // Run the program
    return 0;
}
