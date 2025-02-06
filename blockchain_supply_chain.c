
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

// ANSI Colors for CLI Formatting
#define GREEN "\033[1;32m"
#define RED "\033[1;31m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define RESET "\033[0m"

#define MAX_HASH_SIZE 64
#define MAX_TRANSACTIONS 5
#define DIFFICULTY 4
#define BLOCKCHAIN_FILE "blockchain.dat"

// Transaction Structure
typedef struct {
    int item_id;
    char description[256];
    char signature[256];
} Transaction;

// Block Structure
typedef struct Block {
    int index;
    time_t timestamp;
    Transaction transactions[MAX_TRANSACTIONS];
    int transaction_count;
    char prev_hash[MAX_HASH_SIZE + 1];
    char hash[MAX_HASH_SIZE + 1];
    int nonce;
    struct Block *next;
} Block;

// Blockchain Structure
typedef struct {
    Block *head;
    int size;
} Blockchain;

// Function Prototypes
void generate_hash(const char *input, char *output);
void add_transaction(Blockchain *chain, int item_id, const char *description, const char *signature);
void mine_block(Blockchain *chain);
void print_blockchain(const Blockchain *chain);
void save_blockchain(Blockchain *chain);
void load_blockchain(Blockchain *chain);
void *mine_block_thread(void *args);

/**
 * Generates a SHA-256 hash for a given input string.
 */
void generate_hash(const char *input, char *output) {
    unsigned char raw_hash[EVP_MAX_MD_SIZE];
    unsigned int length;
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    const EVP_MD *md = EVP_sha256();

    if (!ctx) {
        fprintf(stderr, RED "❌ Error: Failed to initialize hash context.\n" RESET);
        exit(EXIT_FAILURE);
    }

    EVP_DigestInit_ex(ctx, md, NULL);
    EVP_DigestUpdate(ctx, input, strlen(input));
    EVP_DigestFinal_ex(ctx, raw_hash, &length);
    EVP_MD_CTX_free(ctx);

    for (unsigned int i = 0; i < length; i++) {
        sprintf(output + (i * 2), "%02x", raw_hash[i]);
    }
    output[length * 2] = '\0';
}

/**
 * Adds a new transaction to the latest block.
 */
void add_transaction(Blockchain *chain, int item_id, const char *description, const char *signature) {
    if (chain->head == NULL) {
        printf(RED "❌ Error: No blocks exist yet. Mine a block first.\n" RESET);
        return;
    }

    Block *latest_block = chain->head;
    if (latest_block->transaction_count >= MAX_TRANSACTIONS) {
        printf(RED "❌ Error: Block is full. Mine a new block first.\n" RESET);
        return;
    }

    Transaction *tx = &latest_block->transactions[latest_block->transaction_count++];
    tx->item_id = item_id;
    strncpy(tx->description, description, sizeof(tx->description) - 1);
    strncpy(tx->signature, signature, sizeof(tx->signature) - 1);
    printf(GREEN "✅ Transaction added successfully!\n" RESET);
}

/**
 * Mines a new block using Proof-of-Work.
 */
void mine_block(Blockchain *chain) {
    Block *new_block = (Block *)malloc(sizeof(Block));
    new_block->index = (chain->head) ? chain->head->index + 1 : 0;
    new_block->timestamp = time(NULL);
    new_block->transaction_count = 0;
    new_block->nonce = 0;
    memset(new_block->hash, 0, sizeof(new_block->hash));

    // Link to previous block
    if (chain->head) {
        strncpy(new_block->prev_hash, chain->head->hash, MAX_HASH_SIZE);
    } else {
        strcpy(new_block->prev_hash, "00000000000000000000000000000000");  // Genesis Block
    }

    // Mine block (Multithreading for efficiency)
    pthread_t thread;
    pthread_create(&thread, NULL, mine_block_thread, new_block);
    pthread_join(thread, NULL);

    // Add block to blockchain
    new_block->next = chain->head;
    chain->head = new_block;
    chain->size++;

    save_blockchain(chain);  // Save after mining
    printf(GREEN "✅ Block %d mined and added to blockchain!\n" RESET, new_block->index);
}

/**
 * Prints the entire blockchain.
 */
void print_blockchain(const Blockchain *chain) {
    Block *current = chain->head;

    if (!current) {
        printf(RED "❌ No blocks found in the blockchain.\n" RESET);
        return;
    }

    while (current) {
        printf(BLUE "📜 Block %d\n" RESET, current->index);
        printf("⏳ Timestamp: %ld\n", current->timestamp);
        printf("🔗 Previous Hash: %s\n", current->prev_hash);
        printf("🔗 Hash: %s\n", current->hash);
        printf("🔢 Nonce: %d\n", current->nonce);
        printf("📝 Transactions:\n");

        for (int i = 0; i < current->transaction_count; i++) {
            printf("   - [%d] %s (Signed: %s)\n", current->transactions[i].item_id,
                   current->transactions[i].description, current->transactions[i].signature);
        }
        printf("\n");

        current = current->next;
    }
}

/**
 * Saves the blockchain to a binary file.
 */
void save_blockchain(Blockchain *chain) {
    FILE *file = fopen(BLOCKCHAIN_FILE, "wb");
    if (!file) {
        printf(RED "❌ Error: Unable to save blockchain.\n" RESET);
        return;
    }

    Block *current = chain->head;
    while (current) {
        fwrite(current, sizeof(Block), 1, file);
        current = current->next;
    }

    fclose(file);
    printf(GREEN "✅ Blockchain saved to %s\n" RESET, BLOCKCHAIN_FILE);
}

/**
 * Loads blockchain from a binary file.
 */
// void load_blockchain(Blockchain *chain) {
//     FILE *file = fopen(BLOCKCHAIN_FILE, "rb");
//     if (!file) {
//         printf(RED "❌ No blockchain file found. Starting new blockchain.\n" RESET);
//         return;
//     }

//     while (!feof(file)) {
//         Block *block = (Block *)malloc(sizeof(Block));
//         if (fread(block, sizeof(Block), 1, file)) {
//             block->next = chain->head;
//             chain->head = block;
//             chain->size++;
//         }
//     }

//     fclose(file);
//     printf(GREEN "✅ Blockchain loaded from %s\n" RESET, BLOCKCHAIN_FILE);
// }

void load_blockchain(Blockchain *chain) {
    FILE *file = fopen(BLOCKCHAIN_FILE, "rb");
    if (!file) {
        printf(RED "❌ No blockchain file found. Starting new blockchain.\n" RESET);
        return;
    }

    chain->head = NULL;  // ✅ Clear existing chain before loading

    while (!feof(file)) {
        Block *block = (Block *)malloc(sizeof(Block));
        if (fread(block, sizeof(Block), 1, file)) {
            block->next = chain->head;
            chain->head = block;
            chain->size++;
        }
    }

    fclose(file);
    printf(GREEN "✅ Blockchain reloaded from file!\n" RESET);
}


/**
 * Multithreaded block mining function.
 */
void *mine_block_thread(void *args) {
    Block *block = (Block *)args;
    char hash[MAX_HASH_SIZE + 1];
    char input[1024];
    int nonce = 0;

    while (1) {
        snprintf(input, sizeof(input), "%d%ld%s%d",
                 block->index, block->timestamp, block->prev_hash, nonce);
        generate_hash(input, hash);

        if (strncmp(hash, "0000", DIFFICULTY) == 0) {
            strcpy(block->hash, hash);
            block->nonce = nonce;
            printf(GREEN "✅ Block mined! Nonce: %d, Hash: %s\n" RESET, nonce, hash);
            pthread_exit(NULL);
        }
        nonce++;
    }
}

/**
 * CLI Menu for Blockchain Interaction
 */
int main() {
    Blockchain blockchain = {NULL, 0};
    load_blockchain(&blockchain);  // Load blockchain at startup

    int choice, item_id;
    char description[256], signature[128];

    while (1) {
        printf("\n" BLUE "===== Supply Chain Blockchain Menu =====\n" RESET);
        printf(GREEN "1. Add Transaction\n2. Mine Block\n3. Print Blockchain\n4. Save Blockchain\n5. Load Blockchain\n6. Exit\n" RESET);
        printf(YELLOW "Enter choice: " RESET);
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                printf(YELLOW "Enter item ID: " RESET);
                scanf("%d", &item_id);
                getchar();
                printf(YELLOW "Enter description: " RESET);
                fgets(description, 256, stdin);
                printf(YELLOW "Enter digital signature: " RESET);
                fgets(signature, 128, stdin);
                add_transaction(&blockchain, item_id, description, signature);
                break;
            case 2:
                mine_block(&blockchain);
                break;
            case 3:
                print_blockchain(&blockchain);
                break;
            case 4:
                save_blockchain(&blockchain);
                break;
            case 5:
                load_blockchain(&blockchain);
                break;
            case 6:
                exit(0);
            default:
                printf(RED "❌ Invalid choice.\n" RESET);
        }
    }
}
