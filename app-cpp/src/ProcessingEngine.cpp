#include "ProcessingEngine.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <mutex>

std::mutex resultsMutex;

std::vector<std::string> extractWords(const std::string& line) {
    std::vector<std::string> words;
    std::string word;
    
    for (char c : line) {
        if (std::isalnum(c) || c == '_' || c == '-') {
            word += c;
        } else {
            if (word.length() >= 4) {
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                words.push_back(word);
            }
            word.clear();
        }
    }
    if (word.length() >= 4) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        words.push_back(word);
    }

    return words;
}

ProcessingEngine::ProcessingEngine(std::shared_ptr<IndexStore> store, int numWorkerThreads) :
        store(store), numWorkerThreads(numWorkerThreads) { }


void ProcessingEngine::processFiles(std::vector<std::string> filePaths, IndexResult &result) {
    for (const auto &filePath : filePaths) {
        std::ifstream file(filePath);
        std::unordered_map<std::string, long> wordFrequencies;
        std::string line;
        while (std::getline(file, line)) {
            for (const auto &word : extractWords(line)) {
                wordFrequencies[word]++;
            }
        }
        long documentNumber = store->putDocument(filePath);
        store->updateIndex(documentNumber, wordFrequencies);

        std::lock_guard<std::mutex> lock(resultsMutex);
        result.totalBytesRead += std::filesystem::file_size(filePath);
    }
}

IndexResult ProcessingEngine::indexFolder(std::string folderPath) {
    IndexResult result = {0.0, 0};
    auto begin = std::chrono::high_resolution_clock::now();
    
    std::vector<std::string> filePaths;
    for (const auto &path : std::filesystem::recursive_directory_iterator(folderPath)) {
        if (path.is_regular_file()) {
            filePaths.push_back(path.path().string());
        }
    }
    
    int numThreads = std::min(numWorkerThreads, static_cast<int>(filePaths.size()));
    std::vector<std::thread> workers;
    std::vector<std::vector<std::string>> threadFileBatches(numThreads);
    
    for (size_t i = 0; i < filePaths.size(); ++i) {
        threadFileBatches[i % numThreads].push_back(filePaths[i]);
    }
    
    for (int i = 0; i < numThreads; ++i) {
        workers.emplace_back(&ProcessingEngine::processFiles, this, threadFileBatches[i], std::ref(result));
    }
    
    for (auto &worker : workers) {
        worker.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.executionTime = std::chrono::duration<double>(end - begin).count();
    return result;
}

// IndexResult ProcessingEngine::indexFolder2(std::string folderPath) {
//     IndexResult result = {0.0, 0};
//     // get the start time
//     // crawl the folder path and extrac all file paths
//     // create the worker threads and give to each worker thread a subset of the documents that need to be indexed
//     // for each file put the document path in the index store and retrieve the document number
//     // for each file extract all valid words/terms and count their frequencies
//     // increment the total number of read bytes
//     // update the main index with the word frequencies for each document
//     // join all of the worker threads
//     // get the stop time and calculate the execution time
//     // return the execution time and the total number of bytes read
//     // fetch the start time
//     auto begin = std::chrono::high_resolution_clock::now();

//     // iterate through all the files in the folder using recursive_directory_iterator
//     for (const auto& path : std::filesystem::recursive_directory_iterator(folderPath)) {

//         // if path represents a regular file, index the file
//         if (path.is_regular_file()) {

//             std::ifstream file(path.path());
//             std::unordered_map<std::string, long> wordFrequencies;

//             //long docID = store->putDocument(path.path());
//             //wordFrequencies.reserve(50000);

//             std::string line;
//             while (std::getline(file, line)) {
//                 for (const auto& word : extractWords(line)) {
//                     wordFrequencies[word]++;
//                 }
//             }

//             // put the document path in the index store and retrieve the document number
//             long documentNumber = store->putDocument(path.path().string());
//             store->updateIndex(documentNumber, wordFrequencies);

//             // increment the total number of read bytes
//             result.totalBytesRead += std::filesystem::file_size(path.path());
//         }
//     }

//     // get the stop time and calculate the execution time
//     auto end = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> duration = end - begin;
//     result.executionTime = duration.count();

//     // return the execution time and the total number of bytes read
//     return result;
// }

SearchResult ProcessingEngine::search(std::vector<std::string> terms) {
    SearchResult result = {0.0, { }};
    // get the start time
    // for each term get the pairs of documents and frequencies from the index store
    // combine the returned documents and frequencies from all of the specified terms
    // sort the document and frequency pairs and keep only the top 10
    // for each document number get from the index store the document path
    // get the stop time and calculate the execution time
    // return the execution time and the top 10 documents and frequencies
    // get the start time
    auto begin = std::chrono::high_resolution_clock::now();

    std::unordered_map<long, long> documentFrequencies;
    std::unordered_map<long, int> documentTermCount;
    int termCount = terms.size();

    for (const auto& term : terms) {
        // for each term get the pairs of documents and frequencies from the index store
        auto tfIDs = store->lookupIndex(term);
        for (const auto& [docId, freq] : tfIDs) {
            documentFrequencies[docId] += freq;
            documentTermCount[docId]++;
        }
    }

    std::vector<std::pair<long, long>> sortedDocuments;
    for (const auto& [docId, freq] : documentFrequencies) {
        if (documentTermCount[docId] == termCount) {
            sortedDocuments.push_back({docId, freq});
        }
    }

    std::sort(sortedDocuments.begin(), sortedDocuments.end(), [](const auto& a, const auto& b) {
        return b.second < a.second;
    });

    // sort the document and frequency pairs in descending order of frequency
    for (const auto& [docId, freq] : sortedDocuments) {
        result.documentFrequencies.push_back({store->getDocument(docId), freq});
    }

    // get the stop time and calculate the execution time
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - begin;

    // return the execution time and the top 10 documents and frequencies
    result.executionTime = duration.count();

    return std::move(result);
}
