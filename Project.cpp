#include "sha256.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

class Block;
class Blockchain;
class User;
class UserList;
class TransactionPool;
class BalanceHashTable;

class BalanceHashTable
{
private:
    struct Node
    {
        string address;
        float balance;
        Node* next;

        Node(string addr, float bal) : address(addr), balance(bal), next(NULL) {}
    };

    int TABLE_SIZE;
    Node** table;

    int hashFunction(string key)
    {
        int hash = 0;
        for (int i = 0; i < (int)key.length(); i++)
        {
            hash = (hash * 31 + key[i]) % TABLE_SIZE;
        }
        return hash;
    }

public:
    BalanceHashTable()
    {
        TABLE_SIZE = 100;
        table = new Node*[TABLE_SIZE];
        for (int i = 0; i < TABLE_SIZE; i++)
        {
            table[i] = NULL;
        }
    }

    ~BalanceHashTable()
    {
        for (int i = 0; i < TABLE_SIZE; i++)
        {
            Node* current = table[i];
            while (current != NULL)
            {
                Node* temp = current;
                current = current->next;
                delete temp;
            }
        }
        delete[] table;
    }

    void updateBalance(string address, float amount)
    {
        int index = hashFunction(address);
        Node* current = table[index];

        while (current != NULL)
        {
            if (current->address == address)
            {
                current->balance += amount;
                return;
            }
            current = current->next;
        }

        Node* newNode = new Node(address, amount);
        newNode->next = table[index];
        table[index] = newNode;
    }

    float getBalance(string address)
    {
        int index = hashFunction(address);
        Node* current = table[index];

        while (current != NULL)
        {
            if (current->address == address)
            {
                return current->balance;
            }
            current = current->next;
        }

        return 0.0;
    }

    void setBalance(string address, float balance)
    {
        int index = hashFunction(address);
        Node* current = table[index];

        while (current != NULL)
        {
            if (current->address == address)
            {
                current->balance = balance;
                return;
            }
            current = current->next;
        }

        Node* newNode = new Node(address, balance);
        newNode->next = table[index];
        table[index] = newNode;
    }

    void clear()
    {
        for (int i = 0; i < TABLE_SIZE; i++)
        {
            Node* current = table[i];
            while (current != NULL)
            {
                Node* temp = current;
                current = current->next;
                delete temp;
            }
            table[i] = NULL;
        }
    }
};

class Transaction 
{
public:
    string fromAddress;
    string toAddress;
    float amount;
    Transaction* next;

    Transaction(string fromAddress, string toAddress, float amount) 
    {
        this->fromAddress = fromAddress;
        this->toAddress = toAddress;
        this->amount = amount;
        this->next = NULL;
    }

    void display()
    {
        cout << "From: " << fromAddress 
             << " -> To: " << toAddress 
             << " | Amount: $" << amount << endl;
    }

    string convertToString()
    {
        return fromAddress + toAddress + to_string(amount);
    }
};

class TransactionPool
{
public:
    Transaction* head;
    int count;

    TransactionPool() : head(NULL), count(0) {}

    ~TransactionPool()
    {
        Transaction* current = head;
        while (current != NULL)
        {
            Transaction* temp = current;
            current = current->next;
            delete temp;
        }
    }

    void addTransaction(Transaction* tx)
    {
        if (head == NULL)
        {
            head = tx;
        }
        else
        {
            Transaction* temp = head;
            while (temp->next != NULL)
            {
                temp = temp->next;
            }
            temp->next = tx;
        }
        count++;
    }

    Transaction* getAll()
    {
        return head;
    }

    void clear()
    {
        Transaction* current = head;
        while (current != NULL)
        {
            Transaction* temp = current;
            current = current->next;
            delete temp;
        }
        head = NULL;
        count = 0;
    }

    void display()
    {
        if (count == 0)
        {
            cout << "No pending transactions.\n";
            return;
        }

        cout << "\n========== PENDING TRANSACTIONS ==========\n";
        cout << "Total: " << count << "\n\n";
        
        Transaction* temp = head;
        int index = 1;
        while (temp != NULL)
        {
            cout << index << ". ";
            temp->display();
            temp = temp->next;
            index++;
        }
        cout << "==========================================\n\n";
    }
};

class Block 
{
public:
    string timestamp;
    Transaction* transactions;
    int transactionCount;
    Block* next;
    string previousHash;
    string hash;
    int nonce;
    string txHash;

    Block(string timestamp, string previousHash) 
        : next(NULL), nonce(0), transactionCount(0), transactions(NULL)
    {
        this->timestamp = timestamp;
        this->previousHash = previousHash;
        this->hash = calculateHash();
    }

    ~Block()
    {
        Transaction* current = transactions;
        while (current != NULL)
        {
            Transaction* temp = current;
            current = current->next;
            delete temp;
        }
    }

    void addTransaction(Transaction* tx)
    {
        if (transactions == NULL)
        {
            transactions = tx;
        }
        else
        {
            Transaction* temp = transactions;
            while (temp->next != NULL)
            {
                temp = temp->next;
            }
            temp->next = tx;
        }
        transactionCount++;
    }

    string calculateTxHash()
    {
        string data = "";
        Transaction* temp = transactions;
        while (temp != NULL)
        {
            data += temp->convertToString();
            temp = temp->next;
        }
        return sha256(data);
    }

    void finalizeTransactions()
    {
        txHash = calculateTxHash();
    }

    string calculateHash()
    {
        return sha256(timestamp + previousHash + to_string(nonce));
    }

    void recalculateHash() 
    {
        hash = calculateHash();
    }

    void mineBlock(int difficulty, bool silent = false)
    {
        string target = string(difficulty, '0');
        if (!silent)
            cout << "Mining block..." << endl;

        while (hash.substr(0, difficulty) != target)
        {
            nonce++;
            hash = calculateHash();
        }
        
        if (!silent)
            cout << "Block mined! Nonce: " << nonce << endl;
    }
};

class Blockchain 
{
public:
    int difficulty;
    Block* chain;
    BalanceHashTable* balanceTable;

    Blockchain() : difficulty(2), chain(NULL)
    {
        balanceTable = new BalanceHashTable();
        chain = createGenesisBlock();
    }

    ~Blockchain() 
    {
        Block* current = chain;
        while (current != NULL) 
        {
            Block* temp = current;
            current = current->next;
            delete temp;
        }
        delete balanceTable;
    }

    Block* createGenesisBlock() 
    {
        Block* genesis = new Block("01/01/2025", "0");
        Transaction* genesisTx = new Transaction("System", "Network", 0);
        genesis->addTransaction(genesisTx);
        genesis->finalizeTransactions();
        genesis->mineBlock(difficulty, true);
        return genesis; 
    }

    void addBlock(string timestamp, Transaction* transactionList, bool silent = false)
    {
        Block* last = getLatestBlock();
        Block* newBlock = new Block(timestamp, last->hash);
        
        Transaction* temp = transactionList;
        while (temp != NULL)
        {
            Transaction* newTx = new Transaction(temp->fromAddress, 
                                                 temp->toAddress, 
                                                 temp->amount);
            newBlock->addTransaction(newTx);
            
            if (temp->fromAddress != "System")
            {
                balanceTable->updateBalance(temp->fromAddress, -temp->amount);
            }
            balanceTable->updateBalance(temp->toAddress, temp->amount);
            
            temp = temp->next;
        }
        newBlock->finalizeTransactions();
        
        newBlock->mineBlock(difficulty, silent);
        last->next = newBlock;
    }

    bool isChainValid()
    {
        if (chain == NULL) 
        {
            return false;
        }

        Block* current = chain->next;
        Block* previous = chain;

        if (previous->hash != previous->calculateHash()) 
        {
            return false;
        }

        while (current != NULL) 
        {
            if (current->hash != current->calculateHash()) 
            {
                return false;
            }

            if (current->txHash != current->calculateTxHash())
            {
                return false;
            }

            if (current->previousHash != previous->hash) 
            {
                return false;
            }

            previous = current;
            current = current->next;
        }

        return true;
    }

    float getBalance(string address)
    {
        return balanceTable->getBalance(address);
    }

    Block* getLatestBlock()
    {
        Block* temp = chain;
        while (temp->next != NULL) 
        {
            temp = temp->next;
        }
        return temp;
    }

    void display()
    {
        Block* temp = chain;
        int blockIndex = 0;

        while (temp != NULL)
        {
            cout << "\n==============================================\n";
            cout << "                BLOCK #" << blockIndex << "\n";
            cout << "==============================================\n";

            if (temp->previousHash == "0")
            {
                cout << " Type: GENESIS BLOCK\n";
                cout << " Timestamp: " << temp->timestamp << "\n";
                cout << " Nonce: " << temp->nonce << "\n"; 
                cout << " Hash: " << temp->hash.substr(0, 32) << "...\n";
                temp = temp->next; 
                blockIndex++;
                continue;
            }

            cout << " Timestamp: " << temp->timestamp << "\n";
            cout << " Transactions: " << temp->transactionCount << "\n";
            cout << " Nonce: " << temp->nonce << "\n"; 
            cout << " Prev Hash: " << temp->previousHash.substr(0, 32) << "...\n";
            cout << " Hash: " << temp->hash.substr(0, 32) << "...\n"; 

            cout << "\n-------------- TRANSACTIONS ---------------\n";
            Transaction* txTemp = temp->transactions;
            int txIndex = 0;
            while (txTemp != NULL)
            {
                cout << " [" << txIndex << "] ";
                txTemp->display();
                txTemp = txTemp->next;
                txIndex++;
            }

            temp = temp->next;
            blockIndex++;
        }
        cout << "==============================================\n\n";
    }

    void copyFrom(Blockchain* source)
    {
        Block* current = chain;
        while (current != NULL) 
        {
            Block* temp = current;
            current = current->next;
            delete temp;
        }

        balanceTable->clear();

        Block* sourceBlock = source->chain;
        chain = NULL;
        Block* lastCopied = NULL;

        while (sourceBlock != NULL)
        {
            Block* newBlock = new Block(sourceBlock->timestamp, sourceBlock->previousHash);
            newBlock->nonce = sourceBlock->nonce;
            
            Transaction* sourceTx = sourceBlock->transactions;
            while (sourceTx != NULL)
            {
                Transaction* newTx = new Transaction(sourceTx->fromAddress, 
                                                     sourceTx->toAddress, 
                                                     sourceTx->amount);
                newBlock->addTransaction(newTx);
                
                if (sourceTx->fromAddress != "System")
                {
                    balanceTable->updateBalance(sourceTx->fromAddress, -sourceTx->amount);
                }
                balanceTable->updateBalance(sourceTx->toAddress, sourceTx->amount);
                
                sourceTx = sourceTx->next;
            }
            newBlock->finalizeTransactions();
            newBlock->hash = sourceBlock->hash;
            newBlock->txHash = sourceBlock->txHash;

            if (chain == NULL)
            {
                chain = newBlock;
                lastCopied = newBlock;
            }
            else
            {
                lastCopied->next = newBlock;
                lastCopied = newBlock;
            }

            sourceBlock = sourceBlock->next;
        }
    }

    int getBlockCount()
    {
        int count = 0;
        Block* temp = chain;
        while (temp != NULL)
        {
            count++;
            temp = temp->next;
        }
        return count;
    }
};

class User 
{
public:
    string address;
    string name;
    Blockchain* localBlockchain;
    bool isActive;
    User* next;

    User(string address, string name) 
        : address(address), name(name), localBlockchain(NULL), isActive(true), next(NULL)
    {
        localBlockchain = new Blockchain();
    }

    ~User()
    {
        if (localBlockchain != NULL)
        {
            delete localBlockchain;
        }
    }

    bool voteOnBlock(Block* proposedBlock, Block* previousBlock)
    {
        if (!isActive || localBlockchain == NULL) 
        {
            return false;
        }

        if (!localBlockchain->isChainValid())
        {
            cout << "[" << name << "] Vote: REJECT - My blockchain is invalid\n";
            return false;
        }

        if (proposedBlock->previousHash != previousBlock->hash) 
        {
            cout << "[" << name << "] Vote: REJECT - Previous hash mismatch\n";
            return false;
        }

        if (proposedBlock->hash != proposedBlock->calculateHash()) 
        {
            cout << "[" << name << "] Vote: REJECT - Invalid hash\n";
            return false;
        }

        if (proposedBlock->txHash != proposedBlock->calculateTxHash())
        {
            cout << "[" << name << "] Vote: REJECT - Transaction hash mismatch\n";
            return false;
        }

        string hash = proposedBlock->hash;
        int difficulty = localBlockchain->difficulty;
        string target = string(difficulty, '0');
        
        if (hash.substr(0, difficulty) != target) 
        {
            cout << "[" << name << "] Vote: REJECT - Insufficient proof of work\n";
            return false;
        }

        cout << "[" << name << "] Vote: ACCEPT\n";
        return true;
    }

    bool voteOnUser(User* newUser)
    {
        if (!isActive) 
        {
            return false;
        }

        cout << "[" << name << "] Vote: ACCEPT new user " << newUser->name << "\n";
        return true;
    }

    void display()
    {
        cout << "User: " << name << " | Address: " << address 
             << " | Status: " << (isActive ? "ACTIVE" : "INACTIVE");
        
        if (localBlockchain != NULL)
        {
            cout << " | Balance: $" << localBlockchain->getBalance(address);
        }
        cout << endl;
    }
};

class UserList
{
public:
    User* head;
    int count;

    UserList() : head(NULL), count(0) {}

    ~UserList()
    {
        User* current = head;
        while (current != NULL)
        {
            User* temp = current;
            current = current->next;
            delete temp;
        }
    }

    void addUser(User* user)
    {
        if (head == NULL)
        {
            head = user;
        }
        else
        {
            User* temp = head;
            while (temp->next != NULL)
            {
                temp = temp->next;
            }
            temp->next = user;
        }
        count++;
    }

    User* getUserAt(int index)
    {
        if (index < 0 || index >= count)
            return NULL;
        
        User* temp = head;
        for (int i = 0; i < index; i++)
        {
            temp = temp->next;
        }
        return temp;
    }

    User* getUserByAddress(string address)
    {
        User* temp = head;
        while (temp != NULL)
        {
            if (temp->address == address)
                return temp;
            temp = temp->next;
        }
        return NULL;
    }

    bool isEmpty() { return head == NULL; }

    void display()
    {
        cout << "\n========== NETWORK USERS ==========\n";
        cout << "Total Users: " << count << "\n\n";
        
        User* temp = head;
        int index = 1;
        while (temp != NULL)
        {
            cout << index << ". ";
            temp->display();
            temp = temp->next;
            index++;
        }
        cout << "===================================\n\n";
    }
};

UserList networkUsers;
TransactionPool txPool;

bool consensusOnBlock(Block* proposedBlock, Block* previousBlock, bool silent = false) 
{
    if (networkUsers.isEmpty()) 
    {
        if (!silent)
            cout << "[CONSENSUS] No users in network. Block rejected.\n";
        return false;
    }

    if (!silent)
    {
        cout << "\n========== CONSENSUS: Block Validation ==========\n";
        cout << "Broadcasting to " << networkUsers.count << " users...\n\n";
    }
    int votesFor = 0;
    int activeUsers = 0;

    User* temp = networkUsers.head;
    while (temp != NULL)
    {
        if (temp->isActive)
        {
            activeUsers++;
            if (temp->voteOnBlock(proposedBlock, previousBlock))
            {
                votesFor++;
            }
        }
        temp = temp->next;
    }

    if (!silent)
    {
        cout << "\n--- Voting Results ---\n";
        cout << "Active Users: " << activeUsers << "\n";
        cout << "Votes FOR: " << votesFor << "\n";
        cout << "Votes AGAINST: " << (activeUsers - votesFor) << "\n";
    }

    bool consensus = (votesFor > activeUsers / 2);
    
    if (!silent)
    {
        if (consensus)
        {
            cout << "CONSENSUS REACHED: Block ACCEPTED\n";
        }
        else
        {
            cout << "CONSENSUS FAILED: Block REJECTED\n";
        }
        cout << "================================================\n\n";
    }
    
    return consensus;
}

bool consensusOnNewUser(User* newUser, bool silent = false) 
{
    if (networkUsers.isEmpty()) 
    {
        if (!silent)
            cout << "[CONSENSUS] First user joining network. Auto-accepted.\n";
        networkUsers.addUser(newUser);
        
        Transaction* bonusTx = new Transaction("System", newUser->address, 100);
        newUser->localBlockchain->addBlock("Joining Bonus", bonusTx, true);
        if (!silent)
            cout << ">>> " << newUser->name << " received $100 joining bonus!\n";
        
        return true;
    }

    if (!silent)
    {
        cout << "\n========== CONSENSUS: New User Request ==========\n";
        cout << "New User: " << newUser->name << "\n";
        cout << "Address: " << newUser->address << "\n";
        cout << "Broadcasting to " << networkUsers.count << " users...\n\n";
    }

    int votesFor = 0;
    int activeUsers = 0;

    User* temp = networkUsers.head;
    while (temp != NULL)
    {
        if (temp->isActive) 
        {
            activeUsers++;
            if (temp->voteOnUser(newUser)) 
            {
                votesFor++;
            }
        }
        temp = temp->next;
    }

    if (!silent)
    {
        cout << "\n--- Voting Results ---\n";
        cout << "Active Users: " << activeUsers << "\n";
        cout << "Votes FOR: " << votesFor << "\n";
        cout << "Votes AGAINST: " << (activeUsers - votesFor) << "\n";
    }

    bool consensus = (votesFor > activeUsers / 2);
    
    if (consensus) 
    {
        if (!silent)
            cout << "CONSENSUS REACHED: User ACCEPTED\n";
        
        User* firstUser = networkUsers.head;
        if (firstUser->localBlockchain != NULL) 
        {
            newUser->localBlockchain->copyFrom(firstUser->localBlockchain);
        }
        
        networkUsers.addUser(newUser);
        
        Transaction* bonusTx = new Transaction("System", newUser->address, 100);
        User* userTemp = networkUsers.head;
        while (userTemp != NULL)
        {
            Transaction* txCopy = new Transaction(bonusTx->fromAddress, 
                                                  bonusTx->toAddress, 
                                                  bonusTx->amount);
            userTemp->localBlockchain->addBlock("User Joining", txCopy, true);
            userTemp = userTemp->next;
        }
        
        if (!silent)
            cout << ">>> " << newUser->name << " received $100 joining bonus!\n";
    } 
    else 
    {
        if (!silent)
            cout << "CONSENSUS FAILED: User REJECTED\n";
    }
    
    if (!silent)
        cout << "================================================\n\n";
    
    return consensus;
}

void displayNetworkUsers() 
{
    networkUsers.display();
}

void displayAllTransactions()
{
    if (txPool.count == 0)
    {
        cout << "\nNo pending transactions in pool.\n";
    }
    else
    {
        cout << "\n--- PENDING TRANSACTIONS ---\n";
        txPool.display();
    }

    if (networkUsers.isEmpty())
    {
        cout << "\nNo users/confirmed transactions available.\n";
        return;
    }

    User* first = networkUsers.head;
    if (first == NULL || first->localBlockchain == NULL)
    {
        cout << "\nNo confirmed transactions found.\n";
        return;
    }

    cout << "\n--- CONFIRMED TRANSACTIONS (from first user's blockchain) ---\n";
    Block* b = first->localBlockchain->chain;
    int blockIdx = 0;
    while (b != NULL)
    {
        if (b->transactions != NULL)
        {
            cout << "Block " << blockIdx << " (" << b->timestamp << ") - Transactions: " << b->transactionCount << "\n";
            Transaction* t = b->transactions;
            while (t != NULL)
            {
                t->display();
                t = t->next;
            }
        }
        b = b->next;
        blockIdx++;
    }
    cout << "-----------------------------------------------\n";
}

bool mineBlock(User* miner, string timestamp, bool silent = false) 
{
    if (miner == NULL || !miner->isActive)
    {
        if (!silent)
            cout << "Invalid miner or miner is inactive!\n";
        return false;
    }

    Transaction* rewardTx = new Transaction("System", miner->address, 50);
    
    Block* lastBlock = miner->localBlockchain->getLatestBlock();
    Block* proposedBlock = new Block(timestamp, lastBlock->hash);
    
    proposedBlock->addTransaction(rewardTx);
    
    Transaction* temp = txPool.head;
    while (temp != NULL)
    {
        Transaction* newTx = new Transaction(temp->fromAddress, 
                                             temp->toAddress, 
                                             temp->amount);
        proposedBlock->addTransaction(newTx);
        temp = temp->next;
    }
    proposedBlock->finalizeTransactions();
    if (!silent)
        cout << "\n" << miner->name << " is mining the block...\n";
    proposedBlock->mineBlock(miner->localBlockchain->difficulty, silent);
    
    if (consensusOnBlock(proposedBlock, lastBlock, silent)) 
    {
        User* userTemp = networkUsers.head;
        while (userTemp != NULL)
        {
            Block* userLastBlock = userTemp->localBlockchain->getLatestBlock();
            Block* newBlock = new Block(timestamp, userLastBlock->hash);
            
            Transaction* txTemp = proposedBlock->transactions;
            while (txTemp != NULL)
            {
                Transaction* txCopy = new Transaction(txTemp->fromAddress, 
                                                      txTemp->toAddress, 
                                                      txTemp->amount);
                newBlock->addTransaction(txCopy);
                
                if (txTemp->fromAddress != "System")
                {
                    userTemp->localBlockchain->balanceTable->updateBalance(txTemp->fromAddress, -txTemp->amount);
                }
                userTemp->localBlockchain->balanceTable->updateBalance(txTemp->toAddress, txTemp->amount);
                
                txTemp = txTemp->next;
            }
            newBlock->nonce = proposedBlock->nonce;
            newBlock->hash = proposedBlock->hash;
            newBlock->txHash = proposedBlock->txHash;
            
            userLastBlock->next = newBlock;
            
            userTemp = userTemp->next;
        }
        
        if (!silent)
            cout << "\n>>> " << miner->name << " earned $50 mining reward!\n";
        
        txPool.clear();
        delete proposedBlock;
        return true;
    } 
    else 
    {
        delete proposedBlock;
        return false;
    }
}

void displayMenu()
{
    cout << "\n========== BLOCKCHAIN MENU ==========\n";
    cout << "1.  Add New User (With Consensus)\n";
    cout << "2.  Display Network Users\n";
    cout << "3.  Toggle User Status\n";
    cout << "4.  Add Transaction to Pool\n";
    cout << "5.  View Transaction Pool\n";
    cout << "6.  Mine Block (With Consensus & Reward)\n";
    cout << "7.  Display User's Blockchain\n";
    cout << "8.  Validate User's Blockchain\n";
    cout << "9.  Check Balance\n";
    cout << "10. Set Difficulty\n";
    cout << "11. Tamper with User's Block\n";
    cout << "12. Display All Transactions\n";
    cout << "0.  Exit\n";
    cout << "=====================================\n";
    cout << "Enter choice: ";
}

int main()
{
    system ("color F0");
    srand(time(0));
    cout << "========== Simple Blockchain Simulation ==========\n\n";
    
    User* huzaif = new User("@huzaif", "Huzaif");
    User* hardeep = new User("@hardeep", "Hardeep");
    User* kazim = new User("@kazim", "Kazim");
    User* ali = new User("@ali", "Ali");
    User* ahmed = new User("@ahmed", "Ahmed");
    User* ansh = new User("@ansh", "Ansh");
    User* abdullah = new User("@abdullah", "Abdullah");
    User* arshad = new User("@arshad", "Arshad");
    User* manav = new User("@manav", "Manav");
    User* sanaullah = new User("@sanaullah", "Sanaullah");
    
        consensusOnNewUser(huzaif, true);
        consensusOnNewUser(hardeep, true);
        consensusOnNewUser(kazim, true);
    
        cout << "Generating 10 blocks...\n";
        cout.flush();

        Transaction* tx1 = new Transaction("@huzaif", "@hardeep", 50);
        if (networkUsers.getUserByAddress(tx1->fromAddress) != NULL &&
            networkUsers.getUserByAddress(tx1->fromAddress)->localBlockchain->getBalance(tx1->fromAddress) >= tx1->amount)
        {
            txPool.addTransaction(tx1);
        }
        else
        {
            delete tx1;
        }

        mineBlock(hardeep, "Block_1", true);

        Transaction* tx2 = new Transaction("@hardeep", "@kazim", 50);
        if (networkUsers.getUserByAddress(tx2->fromAddress) != NULL &&
            networkUsers.getUserByAddress(tx2->fromAddress)->localBlockchain->getBalance(tx2->fromAddress) >= tx2->amount)
        {
            txPool.addTransaction(tx2);
        }
        else
        {
            delete tx2;
        }

        mineBlock(kazim, "Block_2", true);

        Transaction* tx3 = new Transaction("@kazim", "@huzaif", 50);
        if (networkUsers.getUserByAddress(tx3->fromAddress) != NULL &&
            networkUsers.getUserByAddress(tx3->fromAddress)->localBlockchain->getBalance(tx3->fromAddress) >= tx3->amount)
        {
            txPool.addTransaction(tx3);
        }
        else
        {
            delete tx3;
        }

        mineBlock(huzaif, "Block_3", true);

        mineBlock(hardeep, "Block_4", true);
        mineBlock(kazim, "Block_5", true);
        mineBlock(huzaif, "Block_6", true);
        mineBlock(hardeep, "Block_7", true);
        mineBlock(kazim, "Block_8", true);
        mineBlock(huzaif, "Block_9", true);
        mineBlock(hardeep, "Block_10", true);

        cout << "\n10 blocks generated successfully!\n";
        cout << "Total blocks in blockchain: " << huzaif->localBlockchain->getBlockCount() << "\n\n";
        cout.flush();
    
    cout << "Press Enter to continue to the menu...";
    string dummy;
    getline(cin, dummy);
    
    int choice = -1;
    
    do {
        displayMenu();
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input! Please enter a number.\n";
            continue;
        }
        
        cin.ignore(10000, '\n');
        
        switch(choice) {
            case 1: {
                string address, name;
                cout << "User address: ";
                getline(cin, address);
                cout << "User name: ";
                getline(cin, name);
                
                User* newUser = new User(address, name);
                if (!consensusOnNewUser(newUser))
                {
                    delete newUser;
                }
                break;
            }
            case 2:
                displayNetworkUsers();
                break;
            case 3: {
                displayNetworkUsers();
                int idx;
                cout << "User number: ";
                cin >> idx;
                cin.ignore(10000, '\n');
                
                User* user = networkUsers.getUserAt(idx - 1);
                if (user != NULL)
                {
                    user->isActive = !user->isActive;
                    cout << user->name << " is now "
                         << (user->isActive ? "ACTIVE" : "INACTIVE") << endl;
                }
                else
                {
                    cout << "Invalid user number!\n";
                }
                break;
            }
            case 4: {
                string from, to;
                float amount;
                cout << "From Address: ";
                getline(cin, from);
                cout << "To Address: ";
                getline(cin, to);
                cout << "Amount: ";
                cin >> amount;
                cin.ignore(10000, '\n');
                
                User* sender = networkUsers.getUserByAddress(from);
                if (sender == NULL)
                {
                    cout << "Sender not found!\n";
                    break;
                }
                
                if (sender->localBlockchain->getBalance(from) < amount)
                {
                    cout << "Insufficient balance! Balance: $"
                         << sender->localBlockchain->getBalance(from) << "\n";
                    break;
                }
                
                Transaction* tx = new Transaction(from, to, amount);
                txPool.addTransaction(tx);
                cout << "Transaction added to pool!\n";
                break;
            }
            case 5:
                txPool.display();
                break;
            case 6: {
                displayNetworkUsers();
                int minerIdx;
                cout << "Select miner (user number): ";
                cin >> minerIdx;
                cin.ignore(10000, '\n');
                
                User* miner = networkUsers.getUserAt(minerIdx - 1);
                if (miner == NULL)
                {
                    cout << "Invalid user!\n";
                    break;
                }
                
                string timestamp;
                cout << "Timestamp: ";
                getline(cin, timestamp);
                
                mineBlock(miner, timestamp);
                break;
            }
            case 7: {
                displayNetworkUsers();
                int idx;
                cout << "User number: ";
                cin >> idx;
                cin.ignore(10000, '\n');
                
                User* user = networkUsers.getUserAt(idx - 1);
                if (user != NULL && user->localBlockchain != NULL)
                {
                    cout << "\n=== " << user->name << "'s Local Blockchain ===\n";
                    user->localBlockchain->display();
                }
                else
                {
                    cout << "Invalid user!\n";
                }
                break;
            }
            case 8: {
                displayNetworkUsers();
                int idx;
                cout << "User number: ";
                cin >> idx;
                cin.ignore(10000, '\n');
                
                User* user = networkUsers.getUserAt(idx - 1);
                if (user != NULL && user->localBlockchain != NULL)
                {
                    cout << "\nValidating " << user->name << "'s blockchain...\n";
                    if (user->localBlockchain->isChainValid())
                    {
                        cout << "Blockchain is valid!\n";
                    }
                    else
                    {
                        cout << "Blockchain is INVALID!\n";
                    }
                }
                else
                {
                    cout << "Invalid user!\n";
                }
                break;
            }
            case 9: {
                displayNetworkUsers();
                int idx;
                cout << "User number: ";
                cin >> idx;
                cin.ignore(10000, '\n');
                
                User* user = networkUsers.getUserAt(idx - 1);
                if (user == NULL)
                {
                    cout << "Invalid user!\n";
                    break;
                }
                
                string address;
                cout << "Enter address to check: ";
                getline(cin, address);
                
                float balance = user->localBlockchain->getBalance(address);
                cout << "Balance of " << address << " (in " << user->name
                     << "'s blockchain): $" << balance << endl;
                break;
            }
            case 10: {
                displayNetworkUsers();
                int idx;
                cout << "User number: ";
                cin >> idx;
                cin.ignore(10000, '\n');
                
                User* user = networkUsers.getUserAt(idx - 1);
                if (user != NULL && user->localBlockchain != NULL)
                {
                    int newDifficulty;
                    cout << "Current difficulty: " << user->localBlockchain->difficulty << "\n";
                    cout << "Enter new difficulty (1-5): ";
                    cin >> newDifficulty;
                    cin.ignore(10000, '\n');
                    
                    if (newDifficulty >= 1 && newDifficulty <= 5)
                    {
                        user->localBlockchain->difficulty = newDifficulty;
                        cout << "Difficulty set to " << newDifficulty << " for " << user->name << "\n";
                    }
                    else
                    {
                        cout << "Invalid difficulty! Must be between 1 and 5.\n";
                    }
                }
                else
                {
                    cout << "Invalid user!\n";
                }
                break;
            }
            case 11: {
                displayNetworkUsers();
                int idx;
                cout << "User number: ";
                cin >> idx;
                cin.ignore(10000, '\n');
                
                User* user = networkUsers.getUserAt(idx - 1);
                if (user == NULL || user->localBlockchain == NULL)
                {
                    cout << "Invalid user!\n";
                    break;
                }
                
                int blockCount = user->localBlockchain->getBlockCount();
                cout << "Total blocks: " << blockCount << "\n";
                cout << "Enter block number to tamper (1-" << (blockCount - 1) << "): ";
                int blockNum;
                cin >> blockNum;
                cin.ignore(10000, '\n');
                
                if (blockNum < 1 || blockNum >= blockCount)
                {
                    cout << "Invalid block number!\n";
                    break;
                }
                
                Block* targetBlock = user->localBlockchain->chain;
                for (int i = 0; i < blockNum; i++)
                {
                    targetBlock = targetBlock->next;
                }
                
                cout << "\n=== Tampering Options ===\n";
                cout << "1. Change timestamp\n";
                cout << "2. Change transaction amount\n";
                cout << "3. Change nonce\n";
                cout << "Enter option: ";
                int tamperOption;
                cin >> tamperOption;
                cin.ignore(10000, '\n');
                
                switch(tamperOption)
                {
                    case 1: {
                        string newTimestamp;
                        cout << "Current timestamp: " << targetBlock->timestamp << "\n";
                        cout << "Enter new timestamp: ";
                        getline(cin, newTimestamp);
                        targetBlock->timestamp = newTimestamp;
                        cout << "Timestamp changed!\n";
                        break;
                    }
                    case 2: {
                        if (targetBlock->transactions != NULL)
                        {
                            float newAmount;
                            cout << "Current amount: " << targetBlock->transactions->amount << "\n";
                            cout << "Enter new amount: ";
                            cin >> newAmount;
                            cin.ignore(10000, '\n');
                            targetBlock->transactions->amount = newAmount;
                            cout << "Transaction amount changed!\n";
                        }
                        else
                        {
                            cout << "No transactions in this block!\n";
                        }
                        break;
                    }
                    case 3: {
                        int newNonce;
                        cout << "Current nonce: " << targetBlock->nonce << "\n";
                        cout << "Enter new nonce: ";
                        cin >> newNonce;
                        cin.ignore(10000, '\n');
                        targetBlock->nonce = newNonce;
                        cout << "Nonce changed!\n";
                        break;
                    }
                    default:
                        cout << "Invalid option!\n";
                }
                
                cout << "\n>>> Block has been tampered with!\n";
                cout << ">>> Run validation to see the effect.\n";
                break;
            }
            case 12: {
                displayAllTransactions();
                break;
            }
            case 0:
                cout << "\n========== EXITING BLOCKCHAIN NETWORK ==========\n";
                cout << "Thank you for using the blockchain system!\n";
                break;
            default:
                cout << "Invalid choice! Please try again.\n";
        }
    } while (choice != 0);
    
    return 0;
}
