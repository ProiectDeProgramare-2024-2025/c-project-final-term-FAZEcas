#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

//  The application provides a variety of movies (title, description, duration, etc.).
// The user can search and select movies they have watched. Additionally,
// besides the "Watched List," the user can add movies to a "To Watch" list.
// The application allows searching for movies, adding
// them to the desired list, and deleting them from a list.

// ANSI Color codes for terminal colors
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

typedef struct Movie {
    char title[100];
    char description[256];
    int duration;
    struct Movie *next;
} Movie;

Movie *watchedList = NULL;
Movie *toWatchList = NULL;

const char* WATCHED_FILE = "watched_movies.txt";
const char* TO_WATCH_FILE = "to_watch_movies.txt";

// Function declarations
void addMovie(Movie **head, const char *title, const char *description, int duration);
void saveMoviesToFile(Movie *list, const char *filename);
bool validateTitle(char *title);
bool validateDescription(char *description);
bool validateDuration(int duration);
void getValidInput(char *buffer, int size, const char *prompt, bool (*validator)(char*));
int getValidInteger(const char *prompt, bool (*validator)(int));

void clearScreen() {
#ifdef __APPLE__
    system("clear");
#endif
}


void waitForEnter() {
    printf("\nPress Enter to continue...");
    getchar(); // Consume any remaining newline
    getchar(); // Wait for Enter
}

// Validation functions
bool validateTitle(char *title) {
    if (strlen(title) == 0 || strlen(title) > 99) {
        printf(RED "Error: Title must be between 1 and 99 characters.\n" RESET);
        return false;
    }
    return true;
}

bool validateDescription(char *description) {
    if (strlen(description) == 0 || strlen(description) > 255) {
        printf(RED "Error: Description must be between 1 and 255 characters.\n" RESET);
        return false;
    }
    return true;
}

bool validateDuration(int duration) {
    if (duration <= 0 || duration > 600) {  // 600 minutes = 10 hours (reasonable max duration)
        printf(RED "Error: Duration must be between 1 and 600 minutes.\n" RESET);
        return false;
    }
    return true;
}

// Function to get valid string input
void getValidInput(char *buffer, int size, const char *prompt, bool (*validator)(char*)) {
    bool valid;
    do {
        printf("%s", prompt);
        fgets(buffer, size, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline
        valid = validator(buffer);
    } while (!valid);
}

// Function to get valid integer input
int getValidInteger(const char *prompt, bool (*validator)(int)) {
    int value;
    char input[20];
    bool valid;
    do {
        printf("%s", prompt);
        fgets(input, sizeof(input), stdin);

        // Check if input is a valid integer
        bool isNumber = true;
        for (int i = 0; i < strlen(input) - 1; i++) {
            if (!isdigit(input[i])) {
                isNumber = false;
                break;
            }
        }

        if (!isNumber) {
            printf(RED "Error: Please enter a valid number.\n" RESET);
            valid = false;
        } else {
            value = atoi(input);
            valid = validator(value);
        }
    } while (!valid);

    return value;
}

// Function to save movies to a file
void saveMoviesToFile(Movie *list, const char *filename) {
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        printf(RED "Error: Could not open file %s for writing!\n" RESET, filename);
        return;
    }

    Movie *current = list;
    while (current != NULL) {
        fprintf(file, "%s|%s|%d\n", current->title, current->description, current->duration);
        current = current->next;
    }

    fclose(file);
}

void readMoviesFromFile(Movie **list, const char *filename) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return;
    }

    char title[100];
    char description[256];
    int duration;

    while (fscanf(file, "%99[^|]|%255[^|]|%d\n", title, description, &duration) == 3) {
        addMovie(list, title, description, duration);
    }
    fclose(file);
}

void showMainMenu() {
    clearScreen();
    printf("\n%s=== MOVIE TRACKER - MAIN MENU ===%s\n", BOLD BLUE, RESET);
    printf("1. %sWatched Movies Menu%s\n", CYAN, RESET);
    printf("2. %sTo Watch Movies Menu%s\n", CYAN, RESET);
    printf("3. %sSearch Movie%s\n", CYAN, RESET);
    printf("4. %sExit%s\n", RED, RESET);
    printf("Enter your choice (1-4): ");
}

void showWatchedMenu() {
    clearScreen();
    printf("\n%s=== WATCHED MOVIES MENU ===%s\n", BOLD GREEN, RESET);
    printf("1. %sAdd Movie to 'Watched'%s\n", CYAN, RESET);
    printf("2. %sRemove Movie from 'Watched'%s\n", CYAN, RESET);
    printf("3. %sDisplay 'Watched' List%s\n", CYAN, RESET);
    printf("4. %sReturn to Main Menu%s\n", YELLOW, RESET);
    printf("Enter your choice (1-4): ");
}

void showToWatchMenu() {
    clearScreen();
    printf("\n%s=== TO WATCH MOVIES MENU ===%s\n", BOLD MAGENTA, RESET);
    printf("1. %sAdd Movie to 'To Watch'%s\n", CYAN, RESET);
    printf("2. %sRemove Movie from 'To Watch'%s\n", CYAN, RESET);
    printf("3. %sDisplay 'To Watch' List%s\n", CYAN, RESET);
    printf("4. %sReturn to Main Menu%s\n", YELLOW, RESET);
    printf("Enter your choice (1-4): ");
}

void removeMovie(Movie **list, char *title) {
    Movie *current = *list;
    Movie *previous = NULL;

    if (current != NULL && strcmp(current->title, title) == 0) {
        *list = current->next;
        free(current);
        printf(GREEN "Movie successfully removed!\n" RESET);

        // Save changes to appropriate file
        if (list == &watchedList) {
            saveMoviesToFile(watchedList, WATCHED_FILE);
        } else if (list == &toWatchList) {
            saveMoviesToFile(toWatchList, TO_WATCH_FILE);
        }
        return;
    }

    while (current != NULL && strcmp(current->title, title) != 0) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        printf(RED "Movie not found!\n" RESET);
        return;
    }

    previous->next = current->next;
    free(current);
    printf(GREEN "Movie successfully removed!\n" RESET);

    // Save changes to appropriate file
    if (list == &watchedList) {
        saveMoviesToFile(watchedList, WATCHED_FILE);
    } else if (list == &toWatchList) {
        saveMoviesToFile(toWatchList, TO_WATCH_FILE);
    }
}

void displayMovies(Movie *list) {
    if (list == NULL) {
        printf(YELLOW "No movies to display.\n" RESET);
        return;
    }

    Movie *current = list;
    int count = 1;

    while (current != NULL) {
        printf("%d. Title: %s%s%s\n", count++, BOLD GREEN, current->title, RESET);
        printf("   Description: %s\n", current->description);
        printf("   Duration: %s%d minutes%s\n\n", YELLOW, current->duration, RESET);

        current = current->next;
    }
}

void addMovie(Movie **head, const char *title, const char *description, int duration) {
    Movie *newMovie = (Movie *)malloc(sizeof(Movie));
    if (newMovie == NULL) {
        printf(RED "Error: Memory allocation failed!\n" RESET);
        return;
    }

    strcpy(newMovie->title, title);
    strcpy(newMovie->description, description);
    newMovie->duration = duration;
    newMovie->next = NULL;

    if (*head == NULL) {
        *head = newMovie;
    } else {
        Movie *temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newMovie;
    }

    printf(GREEN "Movie successfully added!\n" RESET);

    // Save changes to appropriate file
    if (head == &watchedList) {
        saveMoviesToFile(watchedList, WATCHED_FILE);
    } else if (head == &toWatchList) {
        saveMoviesToFile(toWatchList, TO_WATCH_FILE);
    }
}

Movie* searchMovie(Movie *list, const char *title) {
    Movie *current = list;
    while (current != NULL) {
        if (strcmp(current->title, title) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void handleWatchedMenu() {
    int choice;
    char title[100], description[256];
    int duration;
    char input[10];

    do {
        showWatchedMenu();
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        switch (choice) {
            case 1: // Add movie to 'Watched'
                clearScreen();
                printf("%s=== ADD MOVIE TO WATCHED ===%s\n", BOLD GREEN, RESET);
                getValidInput(title, sizeof(title), "Enter the movie title: ", validateTitle);
                getValidInput(description, sizeof(description), "Enter the description: ", validateDescription);
                duration = getValidInteger("Enter the duration in minutes: ", validateDuration);

                addMovie(&watchedList, title, description, duration);
                waitForEnter();
                break;

            case 2: // Remove movie from 'Watched'
                clearScreen();
                printf("%s=== REMOVE MOVIE FROM WATCHED ===%s\n", BOLD RED, RESET);
                getValidInput(title, sizeof(title), "Enter the movie title to remove: ", validateTitle);

                removeMovie(&watchedList, title);
                waitForEnter();
                break;

            case 3: // Display 'Watched' List
                clearScreen();
                printf("%s=== WATCHED MOVIES LIST ===%s\n", BOLD GREEN, RESET);
                displayMovies(watchedList);
                waitForEnter();
                break;

            case 4: // Return to Main Menu
                break;

            default:
                printf(RED "Invalid choice! Please enter a number between 1 and 4.\n" RESET);
                waitForEnter();
                break;
        }
    } while (choice != 4);
}

void handleToWatchMenu() {
    int choice;
    char title[100], description[256];
    int duration;
    char input[10];

    do {
        showToWatchMenu();
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        switch (choice) {
            case 1: // Add movie to 'To Watch'
                clearScreen();
                printf("%s=== ADD MOVIE TO WATCH ===%s\n", BOLD MAGENTA, RESET);
                getValidInput(title, sizeof(title), "Enter the movie title: ", validateTitle);
                getValidInput(description, sizeof(description), "Enter the description: ", validateDescription);
                duration = getValidInteger("Enter the duration in minutes: ", validateDuration);

                addMovie(&toWatchList, title, description, duration);
                waitForEnter();
                break;

            case 2: // Remove movie from 'To Watch'
                clearScreen();
                printf("%s=== REMOVE MOVIE FROM TO WATCH ===%s\n", BOLD RED, RESET);
                getValidInput(title, sizeof(title), "Enter the movie title to remove: ", validateTitle);

                removeMovie(&toWatchList, title);
                waitForEnter();
                break;

            case 3: // Display 'To Watch' List
                clearScreen();
                printf("%s=== TO WATCH MOVIES LIST ===%s\n", BOLD MAGENTA, RESET);
                displayMovies(toWatchList);
                waitForEnter();
                break;

            case 4: // Return to Main Menu
                // Just break the loop to return
                break;

            default:
                printf(RED "Invalid choice! Please enter a number between 1 and 4.\n" RESET);
                waitForEnter();
                break;
        }
    } while (choice != 4);
}

void handleSearch() {
    char title[100];

    clearScreen();
    printf("%s=== SEARCH MOVIE ===%s\n", BOLD BLUE, RESET);
    getValidInput(title, sizeof(title), "Enter the movie title to search for: ", validateTitle);

    Movie *found = searchMovie(watchedList, title);
    if (found != NULL) {
        printf(GREEN "Movie found in 'Watched' List:\n" RESET);
        printf("Title: %s%s%s\nDescription: %s\nDuration: %s%d minutes%s\n",
               BOLD GREEN, found->title, RESET, found->description, YELLOW, found->duration, RESET);
    } else {
        found = searchMovie(toWatchList, title);
        if (found != NULL) {
            printf(MAGENTA "Movie found in 'To Watch' List:\n" RESET);
            printf("Title: %s%s%s\nDescription: %s\nDuration: %s%d minutes%s\n",
                   BOLD MAGENTA, found->title, RESET, found->description, YELLOW, found->duration, RESET);
        } else {
            printf(RED "Movie not found in either list.\n" RESET);
        }
    }

    waitForEnter();
}

int validateMenuChoice(int choice) {
    if (choice < 1 || choice > 4) {
        printf(RED "Error: Please enter a number between 1 and 4.\n" RESET);
        return false;
    }
    return true;
}

int main(void) {
    int choice;
    char input[10];

    readMoviesFromFile(&watchedList, WATCHED_FILE);
    readMoviesFromFile(&toWatchList, TO_WATCH_FILE);

    do {
        showMainMenu();
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        switch (choice) {
            case 1: // Watched Movies Menu
                handleWatchedMenu();
                break;

            case 2: // To Watch Movies Menu
                handleToWatchMenu();
                break;

            case 3: // Search Movie
                handleSearch();
                break;

            case 4: // Exit
                clearScreen();
                printf(YELLOW "Saving data and exiting...\n" RESET);
                saveMoviesToFile(watchedList, WATCHED_FILE);
                saveMoviesToFile(toWatchList, TO_WATCH_FILE);

                // Free memory before exiting
                Movie *current, *next;

                // Free watched list
                current = watchedList;
                while (current != NULL) {
                    next = current->next;
                    free(current);
                    current = next;
                }

                // Free to-watch list
                current = toWatchList;
                while (current != NULL) {
                    next = current->next;
                    free(current);
                    current = next;
                }
                break;

            default:
                printf(RED "Invalid choice! Please enter a number between 1 and 4.\n" RESET);
                waitForEnter();
                break;
        }
    } while (choice != 4);

    return 0;
}