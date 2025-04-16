#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <stdlib.h>




struct Arguments_for_prog {
    std::string path_to_file = "";
    std::string file_final;
    int n_stats = 10;
    bool print = false; 
    int time = 0;
    time_t to_time = 0;
    time_t from_time = 0;
    int len_file = 0;
    bool to_time_flag = false;
};

int Converter_Num_Month(const std::string month) {
    char months[][12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int i = 0; i < 12; ++i) {
        if (months[i] == month) {
            return i;
        }
    }

    return 0;
}

time_t Converter_Time(const std::string date) {
    struct tm Full_date_form;

    Full_date_form.tm_mday = std::stoi(date.substr(0, date.find('/')));
    Full_date_form.tm_mon = Converter_Num_Month(date.substr(date.find('/') + 1, 3));
    Full_date_form.tm_year = std::stoi(date.substr(date.find('/') + 5, 4)) - 1900;
    Full_date_form.tm_hour = std::stoi(date.substr(date.find(':') + 1, 2)) - 1;
    Full_date_form.tm_min = std::stoi(date.substr(date.find(':') + 4, 2));
    Full_date_form.tm_sec = std::stoi(date.substr(date.find(':') + 7, 2));

    return mktime(&Full_date_form);
}

void Parsing_arg(Arguments_for_prog &arguments, int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.find(".log") != -1) {
            arguments.path_to_file = arg.c_str();
        }
        else if (arg == "-o") {
            arguments.file_final = argv[i + 1];
            i++;
        }
        else if (arg == "-p") {
            arguments.print = true;
        }
        else if (arg == "-s") {
            arguments.n_stats = atoi(argv[i + 1]);
            i++;
        }
        else if (arg == "-w") {
            arguments.time = atoi(argv[i + 1]);
            i++;
        }
        else if (arg == "-f") {
            arguments.from_time = atoi(argv[i + 1]);
            i++;
            arguments.to_time_flag = true;
        }
        else if (arg == "-e") {
            arguments.to_time = atoi(argv[i + 1]);
            i++;
        }
        else if (arg.find("--output=") != std::string::npos) {
            arguments.file_final = arg.substr(9).c_str(); 
        }
        else if (arg.find("--stats=") != std::string::npos) {
            arguments.n_stats = atoi(arg.substr(8).c_str());
        }
        else if (arg.find("--window=") != std::string::npos) {
            arguments.time = atoi(arg.substr(9).c_str());
        }
        else if (arg.find("--from=") != std::string::npos) {
            arguments.from_time = atoi(arg.substr(7).c_str());
            arguments.to_time_flag = true;
        }
        else if (arg.find("--to=") != std::string::npos) {
            arguments.to_time = atoi(arg.substr(5).c_str());
        }
        else if (arg.find("--print") != std::string::npos) {
            arguments.print = true;
        }
        else {
            std::cerr << "Invalid data input format" << std::endl;
            std::cerr << "Enter this in the format: <-command> <value> or <--command=value>" << std::endl;
            break;
        }
    }
    if (arguments.path_to_file == ""){
        std:: cerr << "The file did not open, please retry the request with a .log file" << std::endl;
    }
    std::ifstream work_with_file;
    std::string line;
    time_t value_data;
    std::string data;
    work_with_file.open(arguments.path_to_file);
    while (std::getline(work_with_file, line)){
        if (line.length() < 15){
            continue;
        }
        data = line.substr(line.find("[") + 1, line.rfind("]") - line.find("[") - 1);
        value_data = Converter_Time(data);
        if (arguments.from_time == 0){
            arguments.from_time = value_data;
        }
        if (!arguments.to_time_flag){
            arguments.to_time = value_data;
        }
        arguments.len_file++;
       }
}


void Parser(Arguments_for_prog & arguments){
    std::ofstream file_with_5XX; 
    std::ofstream n_stats_file;
    std::ifstream work_with_file;
    std::string n_stats = "stats.txt";

    std::map<std::string, int> unsorted_5XX;
    std::vector<std::string> requested;
    
    
    std::string request;
    std::string data;
    int i_req = 0;
    int counter = 0;

    file_with_5XX.open(arguments.file_final);
    work_with_file.open(arguments.path_to_file);
    if (work_with_file.is_open()){
        std::string line;
        time_t value_data;
        while (std::getline(work_with_file, line)){
            if (line[line.rfind('"') + 2] == '5'){
                if (line.length() < 15){
                    continue;
                }
                data = line.substr(line.find("[") + 1, line.rfind("]") - line.find("[") - 1);
                value_data = Converter_Time(data);
                if (arguments.from_time <= value_data <= arguments.to_time){
                    if (arguments.print) {
                        std::cout << line << std::endl;
                    }
                    file_with_5XX << line << std::endl;
                    i_req = line.find('"') + 1;
                    request = "";
                    while (i_req != line.rfind('"')){
                        request += line[i_req];
                        i_req += 1;
                    }
                    if ((unsorted_5XX).find(request) != (unsorted_5XX).end()){
                        unsorted_5XX[request] = unsorted_5XX[request] + 1;
                    }
                    else{
                        unsorted_5XX[request] = 1;
                        counter += 1;
                    } 
                }
            }
        }
    }
    else {
        std::cerr << "Not open" << std::endl;
    }
    n_stats_file.open(n_stats);
    int second = 0;
    std::string first;
    std::cout << "Most popular n  request" << std::endl;
    if (n_stats_file.is_open()){
        while(unsorted_5XX.size() > 0 && arguments.n_stats != 0){
            for (auto pair : unsorted_5XX){
                if (pair.second > second){
                    second = pair.second;
                    first = pair.first;
                }
            }
        
            unsorted_5XX.erase(first);
            arguments.n_stats--;
            n_stats_file << first << " " << second << std::endl;
            std::cout << first << " " << second << std::endl;
            second = 0; 
    }       
    }
    n_stats_file.close();
    file_with_5XX.close();
    work_with_file.close();
}

void P_for_window(Arguments_for_prog & arguments) {
    std::ifstream file_for_time(arguments.path_to_file);
    std::vector<time_t> mas(arguments.len_file);

    int maximum_request = 0;
    time_t left = arguments.from_time, right = arguments.to_time;
    time_t left_req_in_time = 0, right_req_in_time = 0;
    int last_id = 0, first_id = 0;
    int counter = 0;
    int time_limit = arguments.time;

    if (!file_for_time.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return;
    }

    std::cout << "File opened successfully." << std::endl;
    std::string line;

    while (std::getline(file_for_time, line)) {
        if (line.length() < 15){
            continue;
        }
        std::string data = line.substr(line.find("[") + 1, line.rfind("]") - line.find("[") - 1);
        time_t conv_date = Converter_Time(data);

        if (conv_date >= left && conv_date <= right) {
            
            if (first_id < arguments.len_file) {
                mas[first_id] = conv_date;

                if (left_req_in_time == 0 && right_req_in_time == 0) {
                    left_req_in_time = right_req_in_time = conv_date;
                }

                right_req_in_time = conv_date;
                counter++;
                first_id++;

                while (right_req_in_time - left_req_in_time > time_limit && last_id < first_id) {
                    left_req_in_time = mas[++last_id];
                    counter--;
                }

                maximum_request = std::max(maximum_request, counter);
            } else {
                std::cerr << "Warning: first_id exceeded array bounds." << std::endl;
            }
        }
    }

    std::cout << "Maximum request count: " << maximum_request << std::endl;
}

int main(int argc, char* argv[]) {
    Arguments_for_prog args;
    Parsing_arg(args, argc, argv);
    if (args.path_to_file != ""){
        Parser(args);
    }
    if (args.time != 0){
        P_for_window(args);
    }
    return 0;
}
