#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Define the size of the hash table
#define TABLE_SIZE 100

// Structure to represent a user with username and hashed password
struct User {
    char username[50];
    char password[64];  // Assuming a 64-character hash (e.g., SHA-256)
};

// Define the hash table as an array of user structures
struct User hashTable[TABLE_SIZE];


struct Book {
    long long isbn; // Store ISBN as a long long integer
    char title[100];
    char author[100];
    char genre[50];
    int quantity;
    struct Book* left;
    struct Book* right;
};

struct IssueDetails {
    char srn[50];
    char name[100];
    long long isbn;
    char dateOfIssue[11];
    struct IssueDetails* next;
};


// Function to compute a simple hash based on the username
int hash(char *username) {
    int hash_value = 0;
    for (int i = 0; username[i] != '\0'; i++) {
        hash_value += username[i];
    }
    return hash_value % TABLE_SIZE;
}

// Function to register a new user with a password
void registerUser(char *username, char *password) {
    int index = hash(username);
    strcpy(hashTable[index].username, username);
    strcpy(hashTable[index].password, password);
}

// Function to validate a user's login
bool login(char *username, char *password) {
    int index = hash(username);
    if (strcmp(username, hashTable[index].username) == 0 && strcmp(password, hashTable[index].password) == 0) {
        return true;  // Username and password match
    }
    return false;  // Username or password is incorrect
}
// Create a new book node with the given data
struct Book* createBookNode(long long isbn, const char title[], const char author[], const char genre[], int quantity) {
    struct Book* newNode = (struct Book*)malloc(sizeof(struct Book));
    newNode->isbn = isbn;
    strcpy(newNode->title, title);
    strcpy(newNode->author, author);
    strcpy(newNode->genre, genre);
    newNode->quantity = quantity;
    newNode->left = newNode->right = NULL;
    return newNode;
}

// Insert a book node into the binary search tree
struct Book* insertBook(struct Book* root, long long isbn, const char title[], const char author[], const char genre[], int quantity) {
    if (root == NULL) {
        return createBookNode(isbn, title, author, genre, quantity);
    }

    if (isbn < root->isbn) {
        root->left = insertBook(root->left, isbn, title, author, genre, quantity);
    } else if (isbn > root->isbn) {
        root->right = insertBook(root->right, isbn, title, author, genre, quantity);
    }

    return root;
}

// Inorder traversal to print the tree
void printTree(struct Book* root) {
    if (root != NULL) {
        printTree(root->left);
        printf("ISBN: %lld\n", root->isbn);
        printf("Title: %s\n", root->title);
        printf("Author: %s\n", root->author);
        printf("Genre: %s\n", root->genre);
        printf("Quantity: %d\n\n", root->quantity);
        printTree(root->right);
    }
}

// Function to display the list of books
void listBooks(struct Book* root) {
    printf("List of Books:\n");
    printTree(root);
}

// Function to add a book to the tree and update the text file
void addBookAndUpdateFile(struct Book** root, long long isbn, const char title[], const char author[], const char genre[], int quantity) {
    *root = insertBook(*root, isbn, title, author, genre, quantity);

    FILE* file = fopen("output.txt", "a");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fprintf(file, "%lld, %s, %s, %s, %d\n", isbn, title, author, genre, quantity);
    fclose(file);
}

// Function to search for a book by ISBN
struct Book* searchByISBN(struct Book* root, long long isbn) {
    if (root == NULL) {
        return NULL;
    }

    if (isbn == root->isbn) {
        return root;
    } else if (isbn < root->isbn) {
        return searchByISBN(root->left, isbn);
    } else {
        return searchByISBN(root->right, isbn);
    }
}

// Function to issue a book
void issueBook(struct Book* root, struct IssueDetails** issueList) {
    printf("Enter your SRN: ");
    char srn[50];
    scanf(" %s", srn);

    printf("Enter your Name: ");
    char name[100];
    scanf(" %s", name);

    printf("Enter the Book ISBN: ");
    long long isbn;
    scanf("%lld", &isbn);

    struct Book* foundBook = searchByISBN(root, isbn);
    if (foundBook != NULL) {
        if (foundBook->quantity > 0) {
            // Update the quantity in memory
            foundBook->quantity--;
            printf("\nIssued 1 copy of Book by ISBN:\n");
            printf("ISBN: %lld\n", foundBook->isbn);
            printf("Title: %s\n", foundBook->title);
            printf("Author: %s\n", foundBook->author);
            printf("New Quantity: %d\n", foundBook->quantity);

            // Save issue details to "issue.txt" file
            FILE* issueFile = fopen("issue.txt", "a");
            if (issueFile == NULL) {
                perror("Error opening issue file");
                return;
            }

            struct IssueDetails* newIssue = (struct IssueDetails*)malloc(sizeof(struct IssueDetails));
            strcpy(newIssue->srn, srn);
            strcpy(newIssue->name, name);
            newIssue->isbn = foundBook->isbn;

            // Get the current date
            time_t now;
            time(&now);
            struct tm* tm_info = localtime(&now);
            strftime(newIssue->dateOfIssue, 11, "%Y-%m-%d", tm_info);

            newIssue->next = NULL;

            // Append the new issue details to the linked list
            if (*issueList == NULL) {
                *issueList = newIssue;
            } else {
                struct IssueDetails* current = *issueList;
                while (current->next != NULL) {
                    current = current->next;
                }
                current->next = newIssue;
            }

            fprintf(issueFile, "%s, %s, %lld, %s\n", srn, name, foundBook->isbn, newIssue->dateOfIssue);
            fclose(issueFile);

            // Update the quantity in "output.txt" file
            FILE* outputFile = fopen("output.txt", "r");
            FILE* tempFile = fopen("temp.txt", "w");

            if (outputFile == NULL || tempFile == NULL) {
                perror("Error opening file");
                return;
            }

            char line[256];

            // Update the quantity in "output.txt" file
            while (fgets(line, sizeof(line), outputFile)) {
                long long fileISBN;
                int fileQuantity;
                char fileTitle[100], fileAuthor[100], fileGenre[50];
                sscanf(line, "%lld, %99[^,], %99[^,], %49[^,], %d", &fileISBN, fileTitle, fileAuthor, fileGenre, &fileQuantity);

                if (fileISBN == isbn) {
                    fprintf(tempFile, "%lld, %s, %s, %s, %d\n", fileISBN, fileTitle, fileAuthor, fileGenre, foundBook->quantity);
                } else {
                    fprintf(tempFile, "%s", line);
                }
            }

            fclose(outputFile);
            fclose(tempFile);

            // Rename the temporary file to replace the original "output.txt" file
            if (remove("output.txt") == 0) {
                if (rename("temp.txt", "output.txt") != 0) {
                    perror("Error renaming file");
                }
            } else {
                perror("Error removing file");
            }
        } else {
            printf("\nNo available copies of this book.\n");
        }
    } else {
        printf("\nBook with ISBN not found.\n");
    }
}

// Function to insert issue details into the linked list
void insertIssueDetails(struct IssueDetails** issueList, const char srn[], const char name[], long long isbn, const char dateOfIssue[]) {
    struct IssueDetails* newIssue = (struct IssueDetails*)malloc(sizeof(struct IssueDetails));
    if (newIssue == NULL) {
        perror("Error allocating memory for issue details");
        return;
    }

    strcpy(newIssue->srn, srn);
    strcpy(newIssue->name, name);
    newIssue->isbn = isbn;
    strcpy(newIssue->dateOfIssue, dateOfIssue);
    newIssue->next = NULL;

    if (*issueList == NULL) {
        *issueList = newIssue;
    } else {
        struct IssueDetails* current = *issueList;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newIssue;
    }
}

// Function to list issued book details from the linked list
void listIssueDetails(struct IssueDetails* issueList) {
    printf("List of Issued Books:\n");
    struct IssueDetails* current = issueList;
    while (current != NULL) {
        printf("SRN: %s\n", current->srn);
        printf("Name: %s\n", current->name);
        printf("ISBN: %lld\n", current->isbn);
        printf("Date of Issue: %s\n\n", current->dateOfIssue);
        current = current->next;
    }
}

// Function to load issue details from a file into the linked list
void loadIssueDetailsFromFile(struct IssueDetails** issueList) {
    FILE* issueFile = fopen("issue.txt", "r");
    if (issueFile == NULL) {
        perror("Error opening issue file");
        return;
    }

    char line[256];
    char srn[50], name[100], dateOfIssue[11];
    long long isbn;
    while (fgets(line, sizeof(line), issueFile)) {
        if (sscanf(line, "%49[^,], %99[^,], %lld, %10s", srn, name, &isbn, dateOfIssue) == 4) {
            insertIssueDetails(issueList, srn, name, isbn, dateOfIssue);
        } else {
            perror("Error parsing issue file");
        }
    }

    fclose(issueFile);
}

// Helper function to find the node with the minimum value
struct Book* findMinNode(struct Book* node) {
    struct Book* current = node;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}

// Function to remove a book from the binary search tree by ISBN
struct Book* removeBookByISBN(struct Book* root, long long isbn) {
    if (root == NULL) {
        return root;
    }

    if (isbn < root->isbn) {
        root->left = removeBookByISBN(root->left, isbn);
    } else if (isbn > root->isbn) {
        root->right = removeBookByISBN(root->right, isbn);
    } else {
        if (root->left == NULL) {
            struct Book* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            struct Book* temp = root->left;
            free(root);
            return temp;
        }

        struct Book* temp = findMinNode(root->right);
        root->isbn = temp->isbn;
        strcpy(root->title, temp->title);
        strcpy(root->author, temp->author);
        strcpy(root->genre, temp->genre);
        root->quantity = temp->quantity;

        root->right = removeBookByISBN(root->right, temp->isbn);
    }

    return root;
}


// Function to remove a book by ISBN and update the "output.txt" file
void removeBookAndUpdateFile(struct Book** root, long long isbn) {
    *root = removeBookByISBN(*root, isbn);

    // Update the "output.txt" file
    FILE* outputFile = fopen("output.txt", "r");
    FILE* tempFile = fopen("temp.txt", "w");

    if (outputFile == NULL || tempFile == NULL) {
        perror("Error opening file");
        return;
    }

    char line[256];

    // Update the "output.txt" file
    while (fgets(line, sizeof(line), outputFile)) {
        long long fileISBN;
        int fileQuantity;
        char fileTitle[100], fileAuthor[100], fileGenre[50];
        sscanf(line, "%lld, %99[^,], %99[^,], %49[^,], %d", &fileISBN, fileTitle, fileAuthor, fileGenre, &fileQuantity);

        if (fileISBN == isbn) {
            // Skip the line to remove it from "output.txt"
        } else {
            fprintf(tempFile, "%s", line);
        }
    }

    fclose(outputFile);
    fclose(tempFile);

    // Rename the temporary file to replace the original "output.txt" file
    if (remove("output.txt") == 0) {
        if (rename("temp.txt", "output.txt") != 0) {
            perror("Error renaming file");
        }
    } else {
        perror("Error removing file");
    }
}

// Function to deposit (return) a book and update the issued file
void depositBook(struct IssueDetails** issueList, struct Book* root, long long isbn) {
    struct IssueDetails* current = *issueList;
    struct IssueDetails* prev = NULL;

    while (current != NULL) {
        if (current->isbn == isbn) {
            if (prev == NULL) {
                *issueList = current->next;
            } else {
                prev->next = current->next;
            }

            free(current);

            // Update the issued file
            FILE* issueFile = fopen("issue.txt", "w");
            if (issueFile == NULL) {
                perror("Error opening issue file");
                return;
            }

            current = *issueList;
            while (current != NULL) {
                fprintf(issueFile, "%s, %s, %lld, %s\n", current->srn, current->name, current->isbn, current->dateOfIssue);
                current = current->next;
            }
            fclose(issueFile);

            // Update the quantity in the output.txt file
            FILE* outputFile = fopen("output.txt", "r");
            FILE* tempFile = fopen("temp.txt", "w");

            if (outputFile == NULL || tempFile == NULL) {
                perror("Error opening file");
                return;
            }

            char line[256];

            while (fgets(line, sizeof(line), outputFile)) {
                long long fileISBN;
                int fileQuantity;
                char fileTitle[100], fileAuthor[100], fileGenre[50];
                sscanf(line, "%lld, %99[^,], %99[^,], %49[^,], %d", &fileISBN, fileTitle, fileAuthor, fileGenre, &fileQuantity);

                if (fileISBN == isbn) {
                    fprintf(tempFile, "%lld, %s, %s, %s, %d\n", fileISBN, fileTitle, fileAuthor, fileGenre, fileQuantity + 1);
                } else {
                    fprintf(tempFile, "%s", line);
                }
            }

            fclose(outputFile);
            fclose(tempFile);

            // Rename the temporary file to replace the original "output.txt" file
            if (remove("output.txt") == 0) {
                if (rename("temp.txt", "output.txt") != 0) {
                    perror("Error renaming file");
                }
            } else {
                perror("Error removing file");
            }

            // Update the quantity in the tree
            struct Book* returnedBook = searchByISBN(root, isbn);
            if (returnedBook != NULL) {
                returnedBook->quantity++;
            }

            printf("Book with ISBN %lld has been deposited.\n", isbn);
            return;
        }
        prev = current;
        current = current->next;
    }

    printf("No issued book found with ISBN %lld.\n", isbn);
}

// Function to update the quantity of a book by ISBN in the binary search tree and the file
void updateBookQuantity(struct Book* root, long long isbn, int newQuantity) {
    struct Book* foundBook = searchByISBN(root, isbn);
    if (foundBook != NULL) {
        int oldQuantity = foundBook->quantity;
        foundBook->quantity = newQuantity;

        // Update the quantity in the output.txt file
        FILE* outputFile = fopen("output.txt", "r");
        FILE* tempFile = fopen("temp.txt", "w");

        if (outputFile == NULL || tempFile == NULL) {
            perror("Error opening file");
            return;
        }

        char line[256];

        while (fgets(line, sizeof(line), outputFile)) {
            long long fileISBN;
            int fileQuantity;
            char fileTitle[100], fileAuthor[100], fileGenre[50];
            sscanf(line, "%lld, %99[^,], %99[^,], %49[^,], %d", &fileISBN, fileTitle, fileAuthor, fileGenre, &fileQuantity);

            if (fileISBN == isbn) {
                fprintf(tempFile, "%lld, %s, %s, %s, %d\n", fileISBN, fileTitle, fileAuthor, fileGenre, newQuantity);
            } else {
                fprintf(tempFile, "%s", line);
            }
        }

        fclose(outputFile);
        fclose(tempFile);

        // Rename the temporary file to replace the original "output.txt" file
        if (remove("output.txt") == 0) {
            if (rename("temp.txt", "output.txt") != 0) {
                perror("Error renaming file");
            }
        } else {
            perror("Error removing file");
        }

        printf("Book quantity updated successfully:\n");
        printf("ISBN: %lld\n", foundBook->isbn);
        printf("Title: %s\n", foundBook->title);
        printf("Author: %s\n", foundBook->author);
        printf("Genre: %s\n", foundBook->genre);
        printf("Old Quantity: %d\n", oldQuantity);
        printf("New Quantity: %d\n", foundBook->quantity);
    } else {
        printf("Book with ISBN %lld not found. Quantity update failed.\n", isbn);
    }
}


int main() {
    printf("----------------------------------------Library Management System-------------------------------------------\n") ;

    registerUser("user1", "password123");
    registerUser("user2", "secret_password");

    char username[50];
    char password[64];

    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    if (login(username, password)) {
        printf("Welcome, %s!\n", username);
        FILE* file = fopen("output.txt", "r");
        if (file == NULL) {
            perror("Error opening file");
            return 1;
        }

        struct Book* root = NULL;
        struct IssueDetails* issueList = NULL;  // Declare the issueList

        char line[256];

        while (fgets(line, sizeof(line), file)) {
            long long isbn;
            int quantity;
            char title[100], author[100], genre[50];
            sscanf(line, "%lld, %99[^,], %99[^,], %49[^,], %d", &isbn, title, author, genre, &quantity);

            root = insertBook(root, isbn, title, author, genre, quantity);
        }

        loadIssueDetailsFromFile(&issueList);

        fclose(file);
        int choice;
        do {
            printf("\nMenu Options:\n");
            printf("1. Add Book\n");
            printf("2. Issue Book\n");
            printf("3. List of Books\n");
            printf("4. List Issued Books\n"); // Added option to list issued books
            printf("5. Search by ISBN\n");
            printf("6. Remove Book\n") ;
            printf("7. Deposit Book\n") ;
            printf("8. Update Quantity\n") ;
            printf("9. Exit\n");
            printf("Enter your choice (1-9): ");
            scanf("%d", &choice);

            long long newISBN;
            int newQuantity;
            char newTitle[100], newAuthor[100], newGenre[50];


            switch (choice) {
                case 1:
                    // Add Book
                    printf("Enter ISBN: ");
                    scanf("%lld", &newISBN);
                    printf("Enter Title: ");
                    scanf(" %99[^\n]", newTitle); // Read the whole line for the title
                    printf("Enter Author: ");
                    scanf(" %99s", newAuthor);
                    printf("Enter Genre: ");
                    scanf(" %49s", newGenre);
                    printf("Enter Quantity: ");
                    scanf("%d", &newQuantity);
                    addBookAndUpdateFile(&root, newISBN, newTitle, newAuthor, newGenre, newQuantity);
                    break;
                case 2:
                    // Issue Book
                    issueBook(root, &issueList);
                    break;
                case 3:
                    // List of Books
                    listBooks(root);
                    break;
                case 4:
                    // List Issued Books
                    listIssueDetails(issueList);
                    break;
                case 5:
                    // Search by ISBN
                    printf("Enter the ISBN to search for: ");
                    long long searchISBN;
                    scanf("%lld", &searchISBN);

                    struct Book* foundBook = searchByISBN(root, searchISBN);
                    if (foundBook != NULL) {
                        printf("Book found:\n");
                        printf("ISBN: %lld\n", foundBook->isbn);
                        printf("Title: %s\n", foundBook->title);
                        printf("Author: %s\n", foundBook->author);
                        printf("Genre: %s\n", foundBook->genre);
                        printf("Quantity: %d\n", foundBook->quantity);
                    } else {
                        printf("Book with ISBN %lld not found.\n", searchISBN);
                    }
                    break;
                case 6:
                    // Remove Book by ISBN
                    printf("Enter the ISBN to remove: ");
                    long long isbnToRemove;
                    scanf("%lld", &isbnToRemove);
                    removeBookAndUpdateFile(&root, isbnToRemove);
                    printf("Book removed successfully.\n");
                    break;
                case 7:
                    printf("Enter the ISBN to deposit: ");
                    long long depositISBN;
                    scanf("%lld", &depositISBN);
                    depositBook(&issueList, root, depositISBN);
                    break;
                case 8:
                    // Deposit Book by ISBN
                    printf("Enter the ISBN to update: ");

                    int newQuantity;
                    long long isbnToUpdate;
                    if (scanf("%lld", &isbnToUpdate) != 1) {
                        printf("Invalid input for ISBN.\n");
                        return 1;
                    }

                    printf("Enter the new quantity: ");
                    if (scanf("%d", &newQuantity) != 1) {
                        printf("Invalid input for quantity.\n");
                        return 1;
                    }

                    // Call the function to update the book quantity
                    updateBookQuantity(root, isbnToUpdate, newQuantity);
                    break;
                case 9:
                    printf("Exiting the program.\n");
                    break;
                default:
                    printf("Invalid choice. Please select a valid option.\n");
            }
        } while (choice != 9);

        // Free the memory used by the binary search tree and issueList here

    } else {
        printf("Login failed. Please check your username and password.\n");
    }
    return 0;
}