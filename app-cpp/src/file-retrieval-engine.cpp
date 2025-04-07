#include <iostream>

#include "IndexStore.hpp"
#include "ProcessingEngine.hpp"
#include "AppInterface.hpp"

int main(int argc, char** argv)
{
    int numWorkerThreads = 1;

    // initialize the number of worker threads from argv[1]
    if (argc > 1) {
        numWorkerThreads = std::stoi(argv[1]);
    }
    else
    {
        std::cout << "Usage: " << argv[0] << " <numWorkerThreads>" << std::endl;
        return 1;
    }
    
    std::shared_ptr<IndexStore> store = std::make_shared<IndexStore>();
    std::shared_ptr<ProcessingEngine> engine = std::make_shared<ProcessingEngine>(store, numWorkerThreads);
    std::shared_ptr<AppInterface> interface = std::make_shared<AppInterface>(engine);

    interface->readCommands();

    return 0;
}
