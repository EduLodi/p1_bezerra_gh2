#include <string>
#include <iostream>
#include <utility>
#include <limits>
#include <ctime>  // Include for time functions

using namespace std;

// Enum for states and actions
enum States { S000, S025, S050, S075, S100, S125, S150 };
enum Inputs { NADA_INPUT = 0, M025, M050, M100, DEV, MEET, ETIRPS, LOG_INPUT };
enum Action { NO_ACTION, D025, D050, D075, D100, D125, D150, LMEET, LETIRPS };

// State transition table: each row is a current state, each column is an input, the output is the next state
States fsm_table[7][8] = {
    {S000, S025, S050, S100, S000, S000, S000, S000}, // S000 state
    {S025, S050, S075, S125, S000, S025, S025, S025}, // S025 state
    {S050, S075, S100, S150, S000, S050, S050, S050}, // S050 state
    {S075, S100, S125, S150, S000, S075, S075, S075}, // S075 state
    {S100, S125, S150, S150, S000, S100, S100, S100}, // S100 state
    {S125, S150, S150, S150, S000, S125, S125, S125}, // S125 state
    {S150, S150, S150, S150, S000, S000, S000, S150}  // S150 state
};

// Action table: each row is a current state, each column is an input, the output is the action taken
Action action_table[7][8] = {
    {NO_ACTION, NO_ACTION, NO_ACTION, NO_ACTION, NO_ACTION, NO_ACTION, NO_ACTION, NO_ACTION},   // S000
    {NO_ACTION, NO_ACTION, NO_ACTION, NO_ACTION, D025, NO_ACTION, NO_ACTION, NO_ACTION},        // S025
    {NO_ACTION, NO_ACTION, NO_ACTION, NO_ACTION, D050, NO_ACTION, NO_ACTION, NO_ACTION},        // S050
    {NO_ACTION, NO_ACTION, NO_ACTION, D050, D075, NO_ACTION, NO_ACTION, NO_ACTION},             // S075
    {NO_ACTION, NO_ACTION, NO_ACTION, D050, D100, NO_ACTION, NO_ACTION, NO_ACTION},             // S100
    {NO_ACTION, NO_ACTION, D025, D100, D125, NO_ACTION, NO_ACTION, NO_ACTION},                  // S125
    {NO_ACTION, D025, D050, D100, D150, LMEET, LETIRPS, NO_ACTION}                              // S150
};

// ClockAndCalendar class to get current date and time
class ClockAndCalendar {
public:
    string get_current_datetime() {
        time_t now = time(0);
        tm* ltm = localtime(&now);

        // Format the date and time
        char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm);
        return string(buf);
    }
};

// Base class for InputOutputHandler
class InputOutputHandler {
public:
    virtual Inputs get_input() = 0;
    virtual void execute_action(Action action) = 0;
    virtual void display_message(const string &message) = 0;
    virtual ~InputOutputHandler() = default;
};

// SodaLogger class to log purchased sodas using a linked list
class SodaLogger {
private:
    struct Node {
        string soda_name;
        Node* next;
        Node(const string& name) : soda_name(name), next(nullptr) {}
    };
    Node* head;

public:
    SodaLogger() : head(nullptr) {}

    ~SodaLogger() {
        // Destructor to free the list
        Node* current = head;
        while (current != nullptr) {
            Node* to_delete = current;
            current = current->next;
            delete to_delete;
        }
    }

    void add_entry(const string& soda_name) {
        Node* new_node = new Node(soda_name);
        new_node->next = head;
        head = new_node;
    }

    void display_log() const {
        Node* current = head;
        cout << "\nSoda Purchase Log:" << endl;
        if (current == nullptr) {
            cout << "No sodas were purchased." << endl;
            return;
        }
        while (current != nullptr) {
            cout << "- " << current->soda_name << endl;
            current = current->next;
        }
    }
};

// Terminal implementation of InputOutputHandler
class TerminalIOHandler : public InputOutputHandler {
private:
    SodaLogger soda_logger;
    ClockAndCalendar clock;  // Instance of ClockAndCalendar

public:
    Inputs get_input() override {
        int input;
        display_message("Select an option:");
        cout << "0 - Show LOG" << endl;
        cout << "1 - Insert $0.25" << endl;
        cout << "2 - Insert $0.50" << endl;
        cout << "3 - Insert $1.00" << endl;
        cout << "4 - Refund" << endl;
        cout << "5 - Buy Meet" << endl;
        cout << "6 - Buy ETIRPS" << endl;
        while (true) {
            display_message("Your choice: ");
            if (cin >> input && input >= 0 && input <= 6) {
                break;
            } else {
                display_message("Invalid input. Please enter a number between 0 and 6.");
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }

        if (input == 0) {
            return LOG_INPUT;
        }

        // Convert user input to enum Inputs
        return static_cast<Inputs>(input);
    }

    void execute_action(Action action) override {
        switch (action) {
            case D025:
                display_message("Dispensing $0.25");
                break;
            case D050:
                display_message("Dispensing $0.50");
                break;
            case D075:
                display_message("Dispensing $0.75");
                break;
            case D100:
                display_message("Dispensing $1.00");
                break;
            case D125:
                display_message("Dispensing $1.25");
                break;
            case D150:
                display_message("Dispensing $1.50");
                break;
            case LMEET:
                display_message("Meet soda dispensed");
                soda_logger.add_entry("Meet");
                break;
            case LETIRPS:
                display_message("Etirps soda dispensed");
                soda_logger.add_entry("Etirps");
                break;
            default:
                display_message("No action");
                break;
        }
    }

    void display_message(const string &message) override {
        cout << clock.get_current_datetime() << " - " << message << endl;
    }

    void display_log() {
        soda_logger.display_log();
    }
};

// StateInstructions class
class StateInstructions {
private:
    States current_state;

public:
    // Constructor with correct initialization
    StateInstructions(States state) : current_state(state) {}

    // Method to handle input and return the next state and action
    pair<States, Action> insert_command(Inputs input) {
        if (input == LOG_INPUT) {
            // Stay in the current state if LOG_INPUT is selected
            return make_pair(current_state, NO_ACTION);
        }
        States next_state = fsm_table[current_state][input];
        Action action_taken = action_table[current_state][input];
        current_state = next_state; // Update the current state
        return make_pair(next_state, action_taken);
    }

    States get_current_state() {
        return current_state;
    }

    // Get current money
    int get_current_money() const {
        switch (current_state) {
            case S000:
                return 0;
            case S025:
                return 25;
            case S050:
                return 50;
            case S075:
                return 75;
            case S100:
                return 100;
            case S125:
                return 125;
            case S150:
                return 150;
            default:
                return 0;
        }
    }
};

// Function to convert state enums to strings
string state_to_string(States state) {
    switch (state) {
        case S000:
            return "S000";
        case S025:
            return "S025";
        case S050:
            return "S050";
        case S075:
            return "S075";
        case S100:
            return "S100";
        case S125:
            return "S125";
        case S150:
            return "S150";
        default:
            return "Unknown State";
    }
}

int main() {
    States initial_state = S000;
    StateInstructions state_instructions(initial_state);

    // Switch between terminal and microcontroller modes here
    TerminalIOHandler io_handler;

    while (true) {
        io_handler.display_message("-------------------------------------------------------------------------------------");
        // Display current money and get input
        int current_money = state_instructions.get_current_money();
        io_handler.display_message("You have " + to_string(current_money) + " cents.");

        Inputs input = io_handler.get_input();

        if (input == LOG_INPUT) {
            io_handler.display_log();
            continue;  // Continue to the next iteration without changing state
        }

        // Process the input and execute the corresponding action
        pair<States, Action> result = state_instructions.insert_command(input);
        States next_state = result.first;
        Action action_taken = result.second;

        io_handler.display_message("Next state: " + state_to_string(next_state));
        io_handler.execute_action(action_taken);
    }

    return 0;
}
