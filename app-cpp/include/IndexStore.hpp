#ifndef INDEX_STORE_H
#define INDEX_STORE_H

#include <string>
#include <mutex>
#include <vector>
#include <unordered_map>

// Data structure that stores a document number and the number of time a word/term appears in the document
struct DocFreqPair {
    long documentNumber;
    long wordFrequency;
};

class IndexStore {
    // map to keep track of the DocumentMap
    std::unordered_map<long, std::string> map_docs;

    // map to keep track of the TermInvertedIndex
    std::unordered_map<std::string, std::vector<DocFreqPair>> map_Tii;

    // Locks for the DocumentMap and the TermInvertedIndex
    std::mutex documentMapLock;
    std::mutex termInvertedIndexLock;
    std::mutex docCounterMutex; // Another mutex for document counter

    // A counter to keep track of the number of documents
    int n_docCounter = 0;

    public:
        // constructor
        IndexStore();

        // default virtual destructor
        virtual ~IndexStore() = default;
        
        long putDocument(std::string documentPath);
        std::string getDocument(long documentNumber);
        void updateIndex(long documentNumber, const std::unordered_map<std::string, long> &wordFrequencies);
        std::vector<DocFreqPair> lookupIndex(std::string term);
};

#endif