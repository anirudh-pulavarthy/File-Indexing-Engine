#include "AppInterface.hpp"

#include <iostream>
#include <string>

AppInterface::AppInterface(std::shared_ptr<ProcessingEngine> engine) : engine(engine) { }

void AppInterface::readCommands() {
    // implement the read commands method
    std::string command;
    
    while (true) {
        std::cout << "> ";
        
        // read from command line
        std::getline(std::cin, command);

        // if the command is quit, terminate the program       
        if (command == "quit") {
            std::cout << "Quitting file retrieval engine. Goodbye!" << std::endl;
            break;
        }
        
        // if the command begins with index, index the files from the specified directory
        if (command.size() >= 5 && command.substr(0, 5) == "index") {
            // parse command and call indexFolder on the processing engine
            // print the execution time and the total number of bytes read
            std::string directory = command.substr(6);
            
            auto result = engine->indexFolder(directory);

            std::cout << "Completed indexing " 
                << result.totalBytesRead 
                << " bytes of data with "
                << engine->getNumWorkerThreads()
                << " worker threads." << std::endl;

            std::cout << "CPU time for indexing = " << result.executionTime << " seconds." << std::endl;
            
            //std::cout << "Size of data read (in bytes): " << result.totalBytesRead << std::endl;
            continue;
        }

        // if the command begins with search, search for files that matches the query
        if (command.size() >= 6 && command.substr(0, 6) == "search") {
            // parse command and call search on the processing engine
            // print the execution time and the top 10 search results
            std::string query = command.substr(7);
            std::vector<std::string> terms;
            size_t pos = 0;
            while ((pos = query.find(" AND ")) != std::string::npos) {
                terms.push_back(query.substr(0, pos));
                query.erase(0, pos + 5);
            }
            terms.push_back(query);

            auto result = engine->search(terms);

            std::cout << "Search completed in " << result.executionTime << " seconds." << std::endl;
            std::cout << "Search results (top 10 of " << result.documentFrequencies.size() << "):" << std::endl;

            // print only the top 10 search results
            for (size_t i = 0; i < std::min(result.documentFrequencies.size(), size_t(10)); ++i) {
                const auto& doc = result.documentFrequencies[i];
                std::cout << doc.documentPath << " (" << doc.wordFrequency << " occurrences)" << std::endl;
            }
            continue;
        }

        std::cout << "Invalid command. Try again..." << std::endl;
    }
}