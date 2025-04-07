#include "IndexStore.hpp"

IndexStore::IndexStore() {
    // initialize the DocumentMap and TermInvertedIndex members
    // initialize the document counter, document map and term inverted index
    n_docCounter = 0;
    map_docs = std::unordered_map<long, std::string>();
    map_Tii = std::unordered_map<std::string, std::vector<DocFreqPair>>();
}

long IndexStore::putDocument(std::string documentPath) {
    // assign a unique number to the document path and return the number
    // IMPORTANT! you need to make sure that only one thread at a time can access this method

    std::lock_guard<std::mutex> counterLock(docCounterMutex); // Ensure thread safety for document counter
    long documentNumber = n_docCounter++;
    
    std::lock_guard<std::mutex> lock(documentMapLock); // Ensure thread safety for DocumentMap
    map_docs[documentNumber] = documentPath;
    return documentNumber;

    // std::lock_guard<std::mutex> lock(documentMapLock);

    // long documentNumber = n_docCounter++;
    // map_docs[documentNumber] = documentPath;

    // return documentNumber;
}

std::string IndexStore::getDocument(long documentNumber) {
    std::string documentPath = "";
    // retrieve the document path that has the given document number
    if (map_docs.find(documentNumber) != map_docs.end())
        documentPath = map_docs[documentNumber];
    else
        ;//std::cout<< "Illegal document number requested: Error in IndexStore::getDocument()" << std::endl;

    return documentPath;
}

void IndexStore::updateIndex(long documentNumber, const std::unordered_map<std::string, long> &wordFrequencies) {
    // update the TermInvertedIndex with the word frequencies of the specified document
    // IMPORTANT! you need to make sure that only one thread at a time can access this method

    std::lock_guard<std::mutex> lock(termInvertedIndexLock); // Ensure thread safety for TermInvertedIndex
    for (const auto &wordFreq : wordFrequencies) {
        const std::string &w = wordFreq.first;
        auto freq = wordFreq.second;
        map_Tii[w].push_back({documentNumber, freq});
    }
}

std::vector<DocFreqPair> IndexStore::lookupIndex(std::string term) {
    std::vector<DocFreqPair> results = {};
    
    // return the document and frequency pairs for the specified term
    if (map_Tii.find(term) != map_Tii.end()) {
        results = map_Tii[term];
    }
    // else {
    //     std::cout << "Term not found in the index: Error in IndexStore::lookupIndex()" << std::endl;
    // }
    return std::move(results);
}
