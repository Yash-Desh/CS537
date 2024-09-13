---
Name: Yash Deshpande
CS login: yashd
Wisc ID email: yadeshpande@wisc.edu
Campus ID: 9087158664
---
## Status : Working
## Functions 

Function `validWord()` checks whether 
1. last character of old word matches with first character of new word
2. Checks if each letter of input is present on the board

Function `updateInputHash()` 
* updates a hashmap of size 26 to show which letters have been used by user input until now


Function `checkInputHash()` 
* compares the size=26 hashmaps of user input & board


Function `wordInDictionary()` 
* checks if the user input word is available in the dictionary via linear search


Function `adjacentCheck` 
* verifies whether the adjacent letters are from the same side of the board. 
* It does this through another hash map created in the main function & passed it as an argument


Function `main()` sets variables & arrays which will be used as hash maps
* It reads the file character the character and stores the characters into a c-string for later use.
For every new line character encountered, increment the sides_cnt unless the previous character is also a newline character

* If any character comes twice, its hash value grows larger than one & hence gets detected


* If the number of sides counted is less than 3 then its an invalid board

* Take user input of static size 500

* Run through a while loop till user inputs valid words & doesn't reach end of file

* For each input check above mentioned functions like `validWord()`, `checkInputHash()`, `adjacentCheck()`, `wordInDictionary()` one after the other

* Then take the next input from user & continue the loop till one of the terminating conditions is met.

* Release all dynamic variables & file pointers.
