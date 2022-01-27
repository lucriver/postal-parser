#include "blockbuffer.cpp"
#include "blockheaderbuffer.cpp"

int main(){
    BlockBuffer blockbuffer1;
    BlockHeaderBuffer blockheaderbuffer1;

    std::cout << "Please enter a CSV file for block generation:\n> ";
    std::string csvFilename;
    std::getline(std::cin,csvFilename);

    blockheaderbuffer1.getSourceFilename(csvFilename);
    blockheaderbuffer1.generateHeader();
    blockheaderbuffer1.generateBlockFile();
    blockheaderbuffer1.createIndexFile();
    blockheaderbuffer1.updateRbnLinks();

    int response;
    std::cout << "Greetings, would you like to... " << std::endl;
    std::cout << "See extreme zip codes for any state  : (1)" << std::endl;
    std::cout << "See details for any specific zip code: (2)" << std::endl;
    std::cout << "Insert a record to the database      : (3)" << std::endl;
    std::cout << "Delete a record from the database    : (4) " << std::endl;
    std::cout << "See details of record file. (view block file header) : (5)" << std::endl;
    std::cout << "Quit : (9)" << std::endl;

    while(response != 9){
        std::cout << "Enter task # > ";
        std::cin >> response;
        switch(response){
            case(1):{
                char resp = 'a';
                blockbuffer1.createTable(blockheaderbuffer1.blockFilenameString());
                while( !(resp == 'q') && !(resp == 'Q')){
                    std::cout << "------------------------------------------------------------------" << std::endl;
                    std::cout << "Please choose from the following options:" << std::endl;
                    std::cout << "Show All States:   (A)" << std::endl;
                    std::cout << "Show Single State: (B)" << std::endl;
                    std::cout << "Quit this task:    (Q)" << std:: endl;
                    std::cin >> resp;
                    std::cout << "------------------------------------------------------------------" << std::endl;
                    switch(resp){
                        case 'A':
                        case 'a': 
                            blockbuffer1.outputTable();
                            break;
                        case 'b':
                        case 'B': {
                            std::cout << "Input state: ";
                            std::string state;
                            std::cin >> state;
                            blockbuffer1.outputState(state);
                            break;
                        }
                        case 'q':
                        case 'Q':
                            break;
                        default:{
                            std::cout << "Invalid input." << std::endl;
                            exit(0);
                        }
                    }
                }
                break;
            }
            case(2):{
                blockheaderbuffer1.updateRbnLinks();
                blockheaderbuffer1.createIndexFile();
                std::cin.clear();
                fflush(stdin);
                blockheaderbuffer1.recordCommandLine();
                blockheaderbuffer1.outputMemoryContents();
                blockheaderbuffer1.writeMemoryFile();
                break;
            }
            case(3):{
                blockheaderbuffer1.recordInsertion();
                break;
            }
            case(4):{
                blockheaderbuffer1.recordDeletion();
                break;
            }
            case(5):{
                blockheaderbuffer1.outputFormattedHeader();
                break;
            }
            default:{
                std::cout << "Goodbye.\n";
                exit(0);
            }
        }
    }
}