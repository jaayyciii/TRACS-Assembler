/************************************************************************************************
Authors:         Gwyneth Jugan, Justine Anne Loo, John Carlo Salinas, & Sharina Sodario
File Name:       assembler.c
Description:     A program that interprets TRACS assembly language into a specified format via .txt file
Submission Date: May 11, 2024           
*************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Structures
struct LineInfo {
    char words[10][10];      // Words per Line, Char per Word
    int wordCount; 
};

struct InstructionSet {
    char syntax[10];
    unsigned char instcode;
    int hasOperand;
};

struct Program{
    unsigned char address;
    unsigned int instruction;
    char labelName[10];
    int flag;
};

struct Label{
    char labelName[10];
    unsigned char address;
};

// Functions
int isInstruction(struct InstructionSet tracs[24], const char *words){
    for(int i = 0; i < 24; i++){
        if(strcmp(words,tracs[i].syntax) == 0)
            return i;
    }
    return -1;
}

int isLabel(struct Label labels[500], const char *words){
    for(int i = 0; i < 500; i++){
        if(strcmp(words,labels[i].labelName) == 0)
            return i;
    }
    return -1;
}

void generateTextFile(int progCount, struct Program programs [1000], char destinationFile[300]){
    FILE *file;

    // New File; Destination
    file = fopen(destinationFile, "w");

    if(file != NULL){
        printf("SYSTEM: Printing content to destination file.\n");
        for(int i = 0; i < progCount; i++){
            fprintf(file, "ADDR=0x%02X; BUS=0x%02X; MainMemory();\n", programs[i].address, programs[i].instruction >> 8);
            fprintf(file, "ADDR=0x%02X; BUS=0x%02X; MainMemory();\n", programs[i].address + 1, (programs[i].instruction & 0b11111111));
        }

        fclose(file);
    } else {
        printf("Error: Could not open destination file. Please provide the correct file path.");
        return;
    }

    printf("System: Output has been written to destination file\n");
}   

// Extra Functions
void addEscapeCharacter(char *str) {
    char temp[2 * strlen(str) + 1];
    
    int j = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\\') {
            temp[j++] = '\\';
        }
        temp[j++] = str[i]; // Copy the character
    }
    temp[j] = '\0'; // Null-terminate the string
    
    strcpy(str, temp); // Copy the modified string back to the original string
}

void print_binary(unsigned int value) {
    for (int i = 15; i >= 0; i--) {
        if(i == 3 || i == 7 || i == 11)
            printf(" ");
        printf("%d", (value >> i) & 1);
    }
    printf("\n");
}     

// Foreground
int main() {
    FILE *file;
    struct LineInfo lines[2000];            // Structure Array that stores the lines and its corresponding words from the .txt file; Max 2000 Lines, May be changed
    char lineString[200];                   // Temporary storage for the string found in a pointed line; Max 200 Characters per Line 
    int lineNumber = 0;

    unsigned int address = 0x00;
    unsigned int tempInstruction = 0x00, tempOperand = 0x00;
    int lineCount = 0, progCount = 0, labelCount = 0;
    int instIndex = -1, labelIndex = -1;
    int eopFlag = 0;
    char tempLabel[10];

    char sourceFile[1000];
    char destinationFile[1000];

    // Initialize TRACS Instruction Set
    struct InstructionSet tracs[24];
    strcpy(tracs[0].syntax, "ADD"); tracs[0].instcode = 0b11110; tracs[0].hasOperand = 0;
    strcpy(tracs[1].syntax, "SUB"); tracs[1].instcode = 0b11101; tracs[1].hasOperand = 0;
    strcpy(tracs[2].syntax, "MUL"); tracs[2].instcode = 0b11011; tracs[2].hasOperand = 0;
    strcpy(tracs[3].syntax, "AND"); tracs[3].instcode = 0b11010; tracs[3].hasOperand = 0;
    strcpy(tracs[4].syntax, "OR"); tracs[4].instcode = 0b11001; tracs[4].hasOperand = 0;
    strcpy(tracs[5].syntax, "NOT"); tracs[5].instcode = 0b11000; tracs[5].hasOperand = 0;
    strcpy(tracs[6].syntax, "XOR"); tracs[6].instcode = 0b10111; tracs[6].hasOperand = 0;
    strcpy(tracs[7].syntax, "SHL"); tracs[7].instcode = 0b10110; tracs[7].hasOperand = 0;
    strcpy(tracs[8].syntax, "SHR"); tracs[8].instcode = 0b10101; tracs[8].hasOperand = 0;
    strcpy(tracs[9].syntax, "WM"); tracs[9].instcode = 0b00001; tracs[9].hasOperand = 1;
    strcpy(tracs[10].syntax, "RM"); tracs[10].instcode = 0b00010; tracs[10].hasOperand = 1;
    strcpy(tracs[11].syntax, "RIO"); tracs[11].instcode = 0b00100; tracs[11].hasOperand = 1;
    strcpy(tracs[12].syntax, "WIO"); tracs[12].instcode = 0b00101; tracs[12].hasOperand = 1;
    strcpy(tracs[13].syntax, "WB"); tracs[13].instcode = 0b00110; tracs[13].hasOperand = 1;
    strcpy(tracs[14].syntax, "WIB"); tracs[14].instcode = 0b00111; tracs[14].hasOperand = 1;
    strcpy(tracs[15].syntax, "WACC"); tracs[15].instcode = 0b01001; tracs[15].hasOperand = 0;
    strcpy(tracs[16].syntax, "RACC"); tracs[16].instcode = 0b01011; tracs[16].hasOperand = 0;
    strcpy(tracs[17].syntax, "SWAP"); tracs[17].instcode = 0b01110; tracs[17].hasOperand = 0;
    strcpy(tracs[18].syntax, "BR"); tracs[18].instcode = 0b00011; tracs[18].hasOperand = 1;
    strcpy(tracs[19].syntax, "BRE"); tracs[19].instcode = 0b10100; tracs[19].hasOperand = 1;
    strcpy(tracs[20].syntax, "BRNE"); tracs[20].instcode = 0b10011; tracs[20].hasOperand = 1;
    strcpy(tracs[21].syntax, "BRGT"); tracs[21].instcode = 0b10010; tracs[21].hasOperand = 1;
    strcpy(tracs[22].syntax, "BRLT"); tracs[22].instcode = 0b10001; tracs[22].hasOperand = 1;
    strcpy(tracs[23].syntax, "EOP"); tracs[23].instcode = 0b11111; tracs[23].hasOperand = 0;
    
    struct Label labels[1000];
    struct Program programs[1000];
    
    printf("\n\nSYSTEM: Instruction set has been loaded. Launching assembler\n");
    printf("* note: please add an additional '\\' in every '\\' of the path for escape character purposes *\n");
    printf("---------------------------------------------------------------------------------------------\n");
    printf("SYSTEM: Enter Source File Path: ");
    fgets(sourceFile, sizeof(sourceFile), stdin);
    sourceFile[strcspn(sourceFile, "\n")] = '\0'; // Remove newline character if present
    printf("%s", sourceFile);
    addEscapeCharacter(sourceFile);
    printf("\n%s", sourceFile);

    return 0;

    printf("SYSTEM: Enter Destination File Path: ");
    fgets(destinationFile, sizeof(destinationFile), stdin);
    destinationFile[strcspn(destinationFile, "\n")] = '\0'; // Remove newline character if present
    printf("---------------------------------------------------------------------------------------------\n");
    // Open Source File
    file = fopen(sourceFile, "r");
    
    // scan through the file
    if (file != NULL){
        printf("SYSTEM: Source file has been loaded\n");
        while (fgets(lineString, sizeof(lineString), file)) {
            char *token = strtok(lineString, " ,.\n\t");                                // Skips " ,.\n\t"
            while (token != NULL) {
                if (strcmp(token, ";") == 0) {                                          // Proceeds to next line if encounters ";"
                    break;
                }
                strcpy(lines[lineNumber].words[lines[lineNumber].wordCount], token);
                lines[lineNumber].wordCount++;

                if (lines[lineNumber].wordCount > 3){
                    printf("Error: Too many arguments on Line %d", lineNumber);
                    return 0;
                }     

                token = strtok(NULL, " ,.\n\t");           
            }
            if(lines[lineNumber].wordCount != 0)
                lineNumber++;
        }
        fclose(file);

        printf("SYSTEM: Content of the source file has been scanned and stored successfully.\n");

        /*
        // For checking purposes
        for (int i = 0; i < lineNumber; i++) {
            printf("Line %d (%d): ", i, lines[i].wordCount);
            for (int j = 0; j < lines[i].wordCount; j++) {
                printf("%s ", lines[i].words[j]);
            }
            printf("\n");
        }*/

        if (strcmp(lines[0].words[0], "ORG") == 0){
            if(lines[0].wordCount == 2){
                address = strtoul(lines[0].words[1], NULL, 16);
                lineCount++;
            } else {
                printf("Error: Unrecognized ORG directive. Starting address has not been set.");
                return 0;
            }
        }

        while(lineCount < lineNumber){
            switch(lines[lineCount].wordCount){
                case 1:
                    instIndex = isInstruction(tracs, lines[lineCount].words[0]);
                    if(strcmp("EOP",lines[lineCount].words[0]))
                            eopFlag = 1;
                    if(instIndex == -1){
                        printf("Error: Instruction '%s' could not be identified on line %d.", lines[lineCount].words[0], lineCount);
                        return 0;
                    } else {
                        if(!tracs[instIndex].hasOperand){
                            tempInstruction = tracs[instIndex].instcode;
                            tempInstruction <<= 11;
                            programs[progCount].instruction = tempInstruction;
                            programs[progCount].address = address;
                            programs[progCount].flag = 0;
                            progCount++;
                            address += 2;
                        } else {
                            printf("Error: Instruction '%s' on line %d expected an operand.", lines[lineCount].words[0], lineCount);
                            return 0;
                        }
                    }
                    break;
                case 2:
                    instIndex = isInstruction(tracs, lines[lineCount].words[0]);
                    if(strcmp("EOP",lines[lineCount].words[0]))
                            eopFlag = 1;
                    if(instIndex == -1){    // case for if the first word of the line is a label, followed by an instruction with no operand
                        strcpy(labels[labelCount].labelName, lines[lineCount].words[0]);
                        labels[labelCount].address = address;
                        labelCount++;

                        instIndex = isInstruction(tracs, lines[lineCount].words[1]);
                        if(instIndex == -1){               
                            printf("Error: Instruction '%s' could not be identified on line %d.", lines[lineCount].words[0], lineCount);
                            return 0;
                        } else {
                            if(!tracs[instIndex].hasOperand){
                                tempInstruction = tracs[instIndex].instcode;
                                tempInstruction <<= 11;
                                programs[progCount].instruction = tempInstruction;
                                programs[progCount].address = address;
                                programs[progCount].flag = 0;
                                progCount++;
                                address += 2;
                            } else {
                                printf("Error: Instruction '%s' on line %d expected an operand.", lines[lineCount].words[1], lineCount);
                                return 0;
                            }
                        }
                    } else {    // case for if the first word of the line is an instruction with operand, it initially stores the operand as a label.
                        if(tracs[instIndex].hasOperand){
                            tempInstruction = tracs[instIndex].instcode;
                            tempInstruction <<= 11;
                            programs[progCount].instruction = tempInstruction;
                            programs[progCount].address = address;
                            programs[progCount].flag = 1;
                            strcpy(programs[progCount].labelName,lines[lineCount].words[1]);
                            progCount++;
                            address += 2;
                        } else {
                            printf("Error: Instruction '%s' on line %d does not expect an operand.", lines[lineCount].words[1], lineCount);
                            return 0;
                        }
                    }
                    break;
                case 3:
                    strcpy(labels[labelCount].labelName, lines[lineCount].words[0]);
                    labels[labelCount].address = address;
                    labelCount++;

                    instIndex = isInstruction(tracs, lines[lineCount].words[1]);
                    if(strcmp("EOP",lines[lineCount].words[0]))
                            eopFlag = 1;
                    if(instIndex == -1){
                        printf("Error: Instruction '%s' could not be identified on line %d.", lines[lineCount].words[0], lineCount);
                        return 0;
                    } else { 
                        if(tracs[instIndex].hasOperand){
                            tempInstruction = tracs[instIndex].instcode;
                            tempInstruction <<= 11;
                            programs[progCount].instruction = tempInstruction;
                            programs[progCount].address = address;
                            programs[progCount].flag = 1;
                            strcpy(programs[progCount].labelName,lines[lineCount].words[2]);
                            progCount++;
                            address += 2;
                        } else {
                            printf("Error: Instruction '%s' on line %d does not expect an operand.", lines[lineCount].words[1], lineCount);
                            return 0;
                        }
                    }
                    break;
                default:
                    printf("Error: Unusual program activity");
                    return 0;
                    break;
            }
            lineCount++;
        }

        // Check for EOP
        if(!eopFlag){
            printf("Error: Assembly file does not contain 'EOP'");
            return 0;
        }

        // Manage operands with labels
        for(int i = 0; i < progCount; i++){ 
            if(programs[i].flag == 1){
                for(int j = 0; j < 500; j++){
                    if(strcmp(programs[i].labelName, labels[j].labelName) == 0){
                        programs[i].instruction |= labels[j].address;
                        break;
                    }
                    programs[i].instruction |= strtoul(programs[i].labelName, NULL, 16);
                }
            }
        }
        
        printf("SYSTEM: Source file has been successfully compiled.\n");
        generateTextFile(progCount, programs, destinationFile);
    } else {
        printf("Error: Could not open source file. Please provide the correct file path.");
        return 0;
    }

    return 0;
}