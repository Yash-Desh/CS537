// Date : 08-09-2024

/*
The program you will build is called letter-boxed. This program plays the New York Times puzzle Letter Boxed.
It takes two command line arguments -- a board file and a dictionary file. Then, it reads words from standard
input (STDIN) until the board is solved, or until an invalid solution is attempted.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// function to check if the input word does not have an invalid letter
bool validWord(char *input_str, int *board_hash, char lastChar)
{
    if (lastChar != input_str[0])
    {
        printf("First letter of word does not match last letter of previous word\n");
        exit(0);
    }

    for (int i = 0; input_str[i] != '\0'; i++)
    {
        if (board_hash[input_str[i] - 97] != 1)
        {
            printf("Used a letter not present on the board\n");
            exit(0);
        }
    }
    return true;
}

// ###########################################################
// If the solution is correct, print Correct\n, and exit with
// return code 0.
// ###########################################################
void updateInputHash(char *input, int *input_hash)
{
    for (int i = 0; input[i] != '\0'; i++)
    {
        input_hash[input[i] - 97]++;
    }
}

bool checkInputHash(int *inputHash, int *boardHash)
{
    int correct_cnt = 0;
    for (int i = 0; i < 26; i++)
    {
        if (((inputHash[i] > 0) && (boardHash[i] > 0)) || ((inputHash[i] == 0) && (boardHash[i] == 0)))
        {
            correct_cnt++;
        }
    }
    if (correct_cnt == 26)
    {
        printf("Correct\n");
        return true;
    }
    else
    {
        return false;
    }
}

bool wordInDictionary(char *input)
{

    // create a file pointer
    FILE *dictPointer = NULL;

    // create a string to store file content
    char str[100];

    dictPointer = fopen("../dict.txt", "r");
    int wordcnt =0;
    while (fgets(str, 100, dictPointer))
    {
        wordcnt++;
        printf("%d\n", wordcnt);
        if(strcmp(input, str) == 0)
        {
            // fclose(dictPointer);
            printf("found word\n");
            return true;
        }
        
    }
    // fclose(dictPointer);
    printf("Word not found in dictionary\n");
    return false;
}

bool adjacentCheck(char *input, int *sideStartHash, char *board)
{
    // for each letter of input except last
    for (int i = 0; i < sizeof(input) - 1; i++)
    {
        // determine position of letter in board
        char letter = input[i];
        int position;
        for (int j = 0; board[j] != '\0'; j++)
        {
            if (board[j] == input[i])
            {
                position = j;
                // printf("board position of %c is %d\n", letter, position);
                // printf("start of side for %c is %d\n", letter, sideStartHash[position]);
                break;
            }
        }

        // traverse the side of the board match next letter
        int startPosition = sideStartHash[position];
        for (int k = sideStartHash[position]; (sideStartHash[k] == sideStartHash[position]); k++)
        {
            // printf("Entered loop for %c\n", letter);
            if (input[i + 1] == board[k])
            {
                printf("Same-side letter used consecutively\n");
                return true;
            }
        }
        // printf("%c\n", letter);
    }
    return false;
}

int main(int argc, char **argv)
{
    char board[26];

    // hashmap of the board letters
    int board_hash[26] = {0};

    // hashmap of the user input
    int input_hash[26] = {0};

    // hashes that store the start
    int sideStartHash[26] = {0};

    // variables that store the position of the start of the side w.r.t board
    int startOfSide = 0;

    // counter to count number of sides
    int sides_cnt = 0;

    // total character counter
    int char_cnt = 0;

    // Opening the board files
    // currently hardcoded name
    FILE *board_ptr = NULL;
    board_ptr = fopen("../tests/tests/1.board", "r");

    // Reading & Printing what is written in file
    // character by character using loop.
    char ch;
    while ((ch = fgetc(board_ptr)) != EOF)
    {
        // counting the number of sides
        if (ch == '\n')
        {
            // increment side_cnt on new line character
            sides_cnt++;

            // increment start of board-side variable
            startOfSide = char_cnt;
        }
        // counting only the letters
        // storing the board as a single string
        else
        {
            // increment the character counter
            char_cnt++;

            // append ch to board
            board[char_cnt - 1] = ch;

            // update sideStartHash
            sideStartHash[char_cnt - 1] = startOfSide;
        }

        // printf("%c", ch);

        // ###################################################################
        // If a board contains a letter more than once, print Invalid board\n
        // and exit with return code 1.
        // ###################################################################

        // check if alphabets are repeated
        if (ch != '\n' && board_hash[ch - 97] >= 1)
        {
            printf("Invalid board_1\n");
            return 1;
        }
        else
        {
            // updating the hash table
            board_hash[ch - 97]++;
        }
    }

    // Add null terminator to the board string
    board[char_cnt] = '\0';
    // printf("board is %s\n", board);

    // for (int i = 0; i < 26; i++)
    // {
    //     printf("%d", board_hash[i]);
    // }
    // printf("\n");
    // for (int i = 0; i < 26; i++)
    // {
    //     printf("%d", sideStartHash[i]);
    // }

    // printf("\nnumber of characters in board %d\n", char_cnt);
    // printf("sides = %d\n", sides_cnt);

    // ###################################################################
    // If the board is invalid (less than 3 sides), print Invalid board\n
    // and exit with return code 1.
    // ###################################################################
    if (sides_cnt < 3)
    {
        printf("Invalid board_2\n");
        return 1;
    }

    // Taking string input as large as the total letter count
    // dynamic allocation since word size is not known
    // Q. what if the input is larger than the number of characters in the board ??
    // char *input = (char *)malloc(char_cnt * sizeof(char));
    char input[500];

    // Variables to store first & last letter
    char last_letter;
    int input_size;

    // printf("Enter input word : ");
    scanf("%s", input);

    last_letter = input[0]; // Initialize last letter = first letter for the 1st input
    // printf("size of input %d\n", input_size);
    // printf("last letter %c\n", last_letter);

    // #####################################################################
    // If the solution uses a letter not present on the board, print Used a
    // letter not present on the board\n and exit with return code 0.
    // #####################################################################

    // #####################################################################
    // If the first character of a word in the solution is not the same as
    // the last character in the preceding word, print First letter of word
    // does not match last letter of previous word\n and exit with return code 0.
    // #####################################################################

    while (validWord(input, board_hash, last_letter))
    {
        // call function to update a hash map of the user inputs
        updateInputHash(input, input_hash);

        // check if the hash map has matched
        if (checkInputHash(input_hash, board_hash))
        {
            return 0;
        }

        // check for adjacent letter
        if (adjacentCheck(input, sideStartHash, board))
        {
            return 0;
        }

        // call function to check if word is in dictionary
        // if(!wordInDictionary(input))
        // {
        //     printf("\n\nEntered dictionary function\n\n");
        //     return 0;
        // }

        input_size = strlen(input);
        last_letter = input[input_size - 1];
        // printf("Enter input word : ");
        scanf("%s", input);
        // validWord(input, board_hash);
    }
    fclose(board_ptr);
    return 0;
}