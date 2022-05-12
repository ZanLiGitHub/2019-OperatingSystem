#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <vector>
using namespace std;

#define REFER 1<<(shiftOfRegister-1)

/**
 * represent a trace
 */
struct Trace{
    char operation;
    int pageNumber;
    int pageOffset;
};


/**
 * represent a page
 */
struct Page{
    bool isDirty;
    int pageNumber;
    int idInFile;
    int referNumber;
};


//global variables
string fileName, algorithmName;
int pageSize = 0, pageNumber = 0, interval = 0, shiftOfRegister = 0, windowSize = 0;
vector<Trace> traces;
vector<Page> pageTable;
map<int, int>pageCount;
//global variables about result
int eventsTraced = 0;
int totalDiskRead = 0;
int totalDiskWrite = 0;
int pageFaults = 0;


/**
 * convert char to related decimal value
 */
int convertHex2Int(char c){
    if(c >= '0' && c <= '9'){
        return c-'0';
    }
    else if(c >= 'a' && c <= 'z'){
        return c - 'a' + 10;
    }
    else{
        return c - 'A' + 10;
    }
}

/**
 * convert hex-string to related decimal value
 */
long long int convertHexStr2int(string hexstr){
    long long int value = 0;
    for(int i = 0; i < hexstr.size(); ++i){
        value = value * 16 + convertHex2Int(hexstr[i]);
    }
    return value;
}

/**
 * check if page hit
 */
bool isPageHit(int pageNumberVisited){
    for(int i = 0; i < pageTable.size(); ++i){
        if(pageTable[i].pageNumber == pageNumberVisited){
            return true;
        }
    }
    return false;
}

/**
 * shift all page's refer in page_table
 */
void shiftPageTable(){
    for(int i = 0; i < pageTable.size(); i++){
        pageTable[i].referNumber = pageTable[i].referNumber / 2;
    }
    return;
}

/**
 * add a new page into page table
 */
void addPage(Trace trace, int id = 0){
    Page newPage;
    newPage.isDirty = trace.operation == 'W';
    newPage.pageNumber = trace.pageNumber;
    newPage.idInFile = id;
    newPage.referNumber = REFER;

    pageTable.push_back(newPage);
    return;
}


/**
 * fifo simulate
 */
void fifoWork(){
    //traverse traces
    for(int i = 0; i < traces.size(); ++i){
        //page hit
        if(isPageHit(traces[i].pageNumber)){
            //set page dirty if op is 'w'
            if(traces[i].operation == 'W'){
                for(auto it = pageTable.begin(); it != pageTable.end(); it++){
                    if(it->pageNumber == traces[i].pageNumber){
                        it->isDirty = true;
                        break;
                    }
                }
            }
        }
        //page miss
        else{
            //update totalDiskRead and pageFaults
            totalDiskRead += 1;
            pageFaults += 1;

            //page table isn't full
            if(pageTable.size() < pageNumber){
                //add it into page table
                addPage(traces[i]);
            }
            //page replace
            else{
                //select first one in pageTable
                Page victimPage = pageTable[0];
                pageTable.erase(pageTable.begin());

                //add it into page table
                addPage(traces[i]);

                //update totalDiskWrite if it's dirty
                if(victimPage.isDirty){
                    totalDiskWrite += 1;
                }
            }
        }
    }
    return;
}

/**
 * lru simulate
 */
void lruWork(){
    //traverse traces
    for(int i = 0; i < traces.size(); ++i){
        //page hit
        if(isPageHit(traces[i].pageNumber)){
            //set the hit page as newest
            for(auto it = pageTable.begin(); it != pageTable.end(); it++){
                if(it->pageNumber == traces[i].pageNumber){
                    //set page dirty if op is 'w'
                    if(traces[i].operation == 'W'){
                        it->isDirty = true;
                    }

                    //delete and reinsert it
                    Page victimPage = *it;
                    pageTable.erase(it);
                    pageTable.push_back(victimPage);
                    break;
                }
            }
        }
        //page miss
        else{
            //update totalDiskRead and pageFaults
            totalDiskRead += 1;
            pageFaults += 1;

            //page table isn't full
            if(pageTable.size() < pageNumber){
                //add it into page table
                addPage(traces[i]);
            }
            //page replace
            else{
                //select first one in pageTable for it's oldest
                Page victimPage = pageTable[0];
                pageTable.erase(pageTable.begin());

                //add it into page table
                addPage(traces[i]);

                //update totalDiskWrite if it's dirty
                if(victimPage.isDirty){
                    totalDiskWrite += 1;
                }
            }
        }
    }
    return;
}


/**
 * get victim for arb
 */
int getVictimPageIndexForArb(){
    int miniRefer = -1.0 * 1e9;
    int index = -1;

    //traverse page table to get victim
    for(int j = 0; j < pageTable.size(); j++){
        if(pageTable[j].referNumber < miniRefer || (pageTable[j].referNumber == pageTable[index].referNumber && pageTable[j].idInFile < pageTable[index].idInFile)){
            index = j;
            miniRefer = pageTable[j].referNumber;
        }
    }
    return index;
}

/**
 * arb simulate
 */
void arbWork(){
    //traverse traces
    for(int i = 0; i < traces.size(); i++){

        //time to shift
        if(i % interval == 0 && i){
            shiftPageTable();
        }

        //page hit
        if(isPageHit(traces[i].pageNumber)){
            //update hit page
            for(auto it = pageTable.begin(); it != pageTable.end(); it++){
                if(it->pageNumber == traces[i].pageNumber){
                    //update referNumber
                    it->referNumber = it->referNumber | REFER;

                    //set page dirty if op is 'w'
                    if(traces[i].operation == 'W'){
                        it->isDirty = true;
                    }
                    break;
                }
            }
        }
        //page miss
        else{
            //update totalDiskRead and pageFaults
            totalDiskRead += 1;
            pageFaults += 1;

            //page table isn't full
            if(pageTable.size() < pageNumber){
                //add it into page table
                addPage(traces[i], i);
            }
            //page replace
            else{
                //select victim
                int victimIndex = getVictimPageIndexForArb();

                //add it into page table
                addPage(traces[i], i);

                //update totalDiskWrite if it's dirty
                if(pageTable[victimIndex].isDirty){
                    totalDiskWrite += 1;
                }

                //remove victim from page table
                auto it = pageTable.begin();
                for(int j = 0; j < victimIndex; j++){
                    it++;
                }
                pageTable.erase(it);
            }
        }


    }
    return;
}

/**
 * get victim for wsarb
 */
int getVictimPageIndexForWsarb()
{
    int miniCount = 1e9;
    int index = -1;

    //traverse page table to get victim
    for(int j = 0; j < pageTable.size(); j++){
        if(pageCount[pageTable[j].pageNumber] < miniCount ||
            (pageCount[pageTable[j].pageNumber] == miniCount && pageTable[j].referNumber < pageTable[index].referNumber) ||
            (pageCount[pageTable[j].pageNumber] == miniCount && pageTable[j].referNumber == pageTable[index].referNumber && pageTable[j].idInFile < pageTable[index].idInFile)){
            index = j;
            miniCount = pageCount[pageTable[j].pageNumber];
        }
    }
    return index;
}

/**
 * wsarb simulate
 */
void wsarbWork(){
    //traverse traces
    for(int i = 0; i < traces.size(); i++){
        //time to shift
        if(i % interval == 0 && i){
            shiftPageTable();
        }

        //update pageCount
        pageCount[traces[i].pageNumber] += 1;
        if(i >= windowSize){
            pageCount[traces[i-windowSize].pageNumber] -= 1;
        }

        //page hit
        if(isPageHit(traces[i].pageNumber)){
            //update hit page
            for(auto it = pageTable.begin(); it != pageTable.end(); it++){
                if(it->pageNumber == traces[i].pageNumber){
                    it->referNumber = it->referNumber | REFER;

                    //set page dirty if op is 'w'
                    if(traces[i].operation == 'W'){
                        it->isDirty = true;
                    }
                    break;
                }
            }
        }
        //page miss
        else{
            //update totalDiskRead and pageFaults
            totalDiskRead += 1;
            pageFaults += 1;

            //page table isn't full
            if(pageTable.size() < pageNumber){
                //add it into page table
                addPage(traces[i], i);
            }
            else{
                //select victim
                int victimIndex = getVictimPageIndexForWsarb();

                //update pageCount
                pageCount.erase(pageCount.find(pageTable[victimIndex].pageNumber));

                //add it into page table
                addPage(traces[i], i);

                //update totalDiskWrite if it's dirty
                if(pageTable[victimIndex].isDirty){
                    totalDiskWrite += 1;
                }

                //remove victim from page table
                auto it = pageTable.begin();
                for(int j = 0; j < victimIndex; j++){
                    it++;
                }
                pageTable.erase(it);
            }
        }

    }
    return;
}

//program driver
int main(int argc, char* argv[]){
    //get parameters
    fileName = argv[1];
    pageSize = atoi(argv[2]);
    pageNumber = atoi(argv[3]);
    algorithmName = argv[4];
    if(argc >= 7)
    {
        shiftOfRegister = atoi(argv[5]);
        interval = atoi(argv[6]);
        if(argc >= 8)
        {
            windowSize = atoi(argv[7]);
        }
    }


    //load traces
    ifstream inFile(fileName);
    string fileBuffer = "";
    while (!inFile.eof() ){
        getline(inFile, fileBuffer, '\n');

        //skip comment or other that needn't
        if(fileBuffer[0] != 'W' && fileBuffer[0] != 'R'){
            continue;
        }

        //get new trace
        Trace newTrace;
        newTrace.operation = fileBuffer[0];
        long long int tempValue = convertHexStr2int(fileBuffer.substr(2, 8));
        newTrace.pageNumber = tempValue / pageSize;
        newTrace.pageOffset = tempValue - pageNumber * pageSize;
        traces.push_back(newTrace);
    }
    inFile.close();

    //update eventsTraced
    eventsTraced = traces.size();

    //simulate according to algorithmName
    if(algorithmName == "FIFO"){
        fifoWork();
    }
    else if(algorithmName == "LRU"){
        lruWork();
    }
    else if(algorithmName == "ARB"){
        arbWork();
    }
    else{
        wsarbWork();
    }


    //show final result
    std::cout << "events in trace:    " << eventsTraced << std::endl;
    std::cout << "total disk reads:   " << totalDiskRead << std::endl;
    std::cout << "total disk writes:  " << totalDiskWrite << std::endl;
    std::cout << "page faults:        " << pageFaults << std::endl;
    return 0;
}
