/* Authors:
* Andrew Hill
* Rui Tu
* Haley Whitman
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define CACHE_MEMORY_SIZE 10
char INST_SET[100][5][15] = {0};
int INST_SET_LENGTH;
int PC;

int REGISTER_FILE[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int MEMORY[1000];
int CACHE_MEMORY[CACHE_MEMORY_SIZE][2];
int CACHE_SAVE_INDEX = 0;
int TOTAL_HIT_COUNTER = 0;
int TOTAL_MISS_COUNTER = 0;

struct CACHE_OBJECT{
    int valid;
    int tag;
    int data;
}CACHE_ARRAY[CACHE_MEMORY_SIZE];

struct InstructionObject{
    char instruction[15], destRegister1[15], register2[15], register3[15];
    int valueR1, valueR2, valueR3;
    int ALU_output;
    char ALU_CODE[20];
    int memory_op;
    int memory_op_address;
    int tempPC;
};

pthread_t tid[5];
pthread_mutex_t lock;

char* registerNames[32] = {"$0", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2",
                           "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5","$s6", "$s7", "$t8", "$t9", "$k0", "$k1",
                           "$gp", "$sp", "$fp","$ra"};
/*=========================BEGIN HELPER FUNCTIONS====================*/
int isBlankLine(const char *line) {
    const char accept[]=" \t\r\n"; // any white spaces or simply \n.
    return (strspn(line, accept) == strlen(line));
}

int charToNumber(char *number){
    char *ptr;
    return (int) (strtol(number, &ptr, 10));
}
int findIndex(char *register_name){
    int i;
    for(i = 0; i < 32; i = i+1){
        if (strcmp(registerNames[i], register_name)==0){
            return i;
        }
    }
    return -1;
}

int findLabel(char *label){
    int i;
    for(i=0; i< INST_SET_LENGTH; i++){
        if(strncmp(INST_SET[i][0], label, strlen(label)) == 0){
            return i;
        }
    }
    return -1;
}

int findValue(char *register_name) {
    return REGISTER_FILE[findIndex(register_name)];
}
/*=========================PC========================================*/
void advancePC(struct InstructionObject x){
    if(strcmp(INST_SET[x.tempPC][1], "j") == 0){
        PC = findLabel(INST_SET[x.tempPC][2]);
    } else if(strcmp(INST_SET[x.tempPC][1], "jal") == 0){
        PC = findLabel(INST_SET[x.tempPC][2]);
    } else if(strcmp(INST_SET[x.tempPC][1], "jr") == 0){
        PC = findValue("$ra");
    } else if(strcmp(INST_SET[x.tempPC][1], "beq") == 0){
        if(x.valueR3 == 1){
            PC = findLabel(INST_SET[x.tempPC][4]);
        } else {
            PC++;
        }
    } else if(strcmp(INST_SET[x.tempPC][1], "bne") == 0){
        if(x.valueR3 == 1){
            PC++;
        } else {
            PC = findLabel(INST_SET[x.tempPC][4]);
        }
    } else {
        PC++;
    }
    return;
}
/*=========================Write To Register=========================*/
void write_data_to_register(char* register_name, int data){
    int register_file_index = findIndex(register_name);
    REGISTER_FILE[register_file_index] = data;
}

struct InstructionObject writeBack(struct InstructionObject x){
    if(strcmp(x.instruction, "jr")==0){
        return x;
    } else if(strcmp(x.instruction, "jal") == 0) {
        write_data_to_register("$ra", x.tempPC+1);
        return x;
    }else if(strcmp(x.instruction, "j")==0){
        return x;
    }else if(strcmp(x.instruction,"sw")==0){
        return x;
    }else if(strcmp(x.instruction, "bne")==0){
        return x;
    }else if(strcmp(x.instruction, "beq")==0){
        return x;
    }
    write_data_to_register(x.destRegister1, x.valueR1);
    return x;
}
/*=========================Write to Memory===========================*/
int memory_load(int address) {
    int i;
    for (i = 0; i < CACHE_MEMORY_SIZE; i++) {
        if (CACHE_ARRAY[i].tag == address) {
            TOTAL_HIT_COUNTER++;
            return CACHE_ARRAY[i].data;
        }
    }

    TOTAL_MISS_COUNTER++;
    return MEMORY[address];
}

void memory_store(int address, int data) {
    MEMORY[address] = data;
    int in_cache_flag = 0;
    int i;
    for (i = 0; i < CACHE_MEMORY_SIZE; i++) {
        if (CACHE_ARRAY[i].tag == address && CACHE_ARRAY[i].data != data) {
            CACHE_ARRAY[i].data = data;
            in_cache_flag = 1;
            CACHE_SAVE_INDEX++;
        }
    }

    if (in_cache_flag == 0) {
        CACHE_ARRAY[CACHE_SAVE_INDEX].tag = address;
        CACHE_ARRAY[CACHE_SAVE_INDEX].data = data;
        CACHE_SAVE_INDEX = (CACHE_SAVE_INDEX + 1) % CACHE_MEMORY_SIZE;
    }
}

struct InstructionObject mem(struct InstructionObject x){
    if (x.memory_op == 0) { //for load word
        x.ALU_output = memory_load(x.memory_op_address);
        memory_store( x.memory_op_address, x.ALU_output );
        x.memory_op = -1;
    } else if (x.memory_op == 1) { // for save word
        int data = findValue(x.destRegister1);
        memory_store(x.memory_op_address, data);
        x.memory_op = -1;
    } else
        ;
    return x;
}
/*=========================BEGIN ALU ================================*/
struct InstructionObject execute(struct InstructionObject x){
    if(strcmp(x.ALU_CODE, "ALU_ADD")==0){
        x.valueR1 = x.valueR2 + x.valueR3;
        return x;
    } else if(strcmp(x.ALU_CODE, "ALU_SUB")==0){
        x.valueR1 = x.valueR2 - x.valueR3;
        return x;
    } else if(strcmp(x.ALU_CODE, "ALU_MULT")==0){
        x.valueR1 = x.valueR2 * x.valueR3;
        return x;
    } else if(strcmp(x.ALU_CODE, "ALU_AND")==0){
        x.valueR1 = x.valueR2 & x.valueR3;
        return x;
    } else if(strcmp(x.ALU_CODE, "ALU_OR")==0){
        x.valueR1 = x.valueR2 | x.valueR3;
        return x;
    } else if(strcmp(x.ALU_CODE, "ALU_NOR")==0){
        x.valueR1 = ~(x.valueR2 | x.valueR3);
        return x;
    } else if(strcmp(x.ALU_CODE, "ALU_SLT")==0){
        x.valueR1 = x.valueR2 < x.valueR3;
        return x;
    } else if(strcmp(x.ALU_CODE, "ALU_SLTU")==0){
        x.valueR1 = x.valueR2 <  x.valueR3;
        return x;
    } else if(strcmp(x.ALU_CODE, "ALU_EQUAL")==0){
        x.valueR3 = (x.valueR1 == x.valueR2);
        return x;
    } else if(strcmp(x.ALU_CODE, "ALU_NOP")==0){
        return x;
    } else {
        return x;
    }
}
/*=========================BEGIN DECODE =============================*/
struct InstructionObject decode(struct InstructionObject x){
    if(strcmp(x.instruction, "add")==0){
        strcpy(x.ALU_CODE, "ALU_ADD");
        x.valueR1 = findValue(x.destRegister1);
        x.valueR2 = findValue(x.register2);
        x.valueR3 = findValue(x.register3);
    } else if(strcmp(x.instruction, "addi")==0){
        strcpy(x.ALU_CODE, "ALU_ADD");
        x.valueR1 = findValue(x.destRegister1);
        x.valueR2 = findValue(x.register2);
        x.valueR3 = charToNumber(x.register3);
    } else if(strcmp(x.instruction, "sub")==0){
        strcpy(x.ALU_CODE, "ALU_SUB");
        x.valueR1 = findValue(x.destRegister1);
        x.valueR2 = findValue(x.register2);
        x.valueR3 = findValue(x.register3);
    } else if(strcmp(x.instruction, "mult")==0){
        strcpy(x.ALU_CODE, "ALU_MULT");
        x.valueR1 = findValue(x.destRegister1);
        x.valueR2 = findValue(x.register2);
        x.valueR3 = findValue(x.register3);
    } else if(strcmp(x.instruction, "slt")==0){
        strcpy(x.ALU_CODE, "ALU_SLT");
        x.valueR1 = findValue(x.destRegister1);
        x.valueR2 = findValue(x.register2);
        x.valueR3 = findValue(x.register3);
    } else if(strcmp(x.instruction, "sltu")==0){
        strcpy(x.ALU_CODE, "ALU_SLTU");
        x.valueR1 = findValue(x.destRegister1);
        x.valueR2 = findValue(x.register2);
        x.valueR3 = findValue(x.register3);
    } else if(strcmp(x.instruction, "lw")==0){
        strcpy(x.ALU_CODE, "ALU_NOP");
        x.memory_op = 0;
        x.valueR1 = findValue(x.destRegister1);
        int base_address = findValue(INST_SET[x.tempPC][3])/4;
        int off_set      = charToNumber(INST_SET[x.tempPC][4]) / 4;
        x.memory_op_address = off_set + base_address;
        x.valueR1 = memory_load(x.memory_op_address);
    } else if(strcmp(x.instruction, "sw")==0){
        strcpy(x.ALU_CODE, "ALU_NOP");
        x.memory_op = 1;
        x.valueR1 = findValue(x.destRegister1);
        int base_address = findValue(INST_SET[x.tempPC][3])/4;
        int off_set      = charToNumber(INST_SET[x.tempPC][4]) / 4;
        x.memory_op_address = off_set + base_address;
        memory_store(x.memory_op_address, x.valueR1);
    } else if(strcmp(x.instruction, "beq")==0){
        strcpy(x.ALU_CODE, "ALU_EQUAL");
        x.valueR1 = findValue(x.destRegister1);
        x.valueR2 = findValue(x.register2);
    } else if(strcmp(x.instruction, "bne")==0){
        strcpy(x.ALU_CODE, "ALU_EQUAL");
        x.valueR1 = findValue(x.destRegister1);
        x.valueR2 = findValue(x.register2);
    } else if(strcmp(x.instruction, "j")==0){
        strcpy(x.ALU_CODE, "ALU_NOP");
    } else if(strcmp(x.instruction, "jal")==0){
        strcpy(x.ALU_CODE, "ALU_NOP");
        REGISTER_FILE[31] = x.tempPC+1;
    } else if(strcmp(x.instruction, "jr")==0){
        strcpy(x.ALU_CODE, "ALU_NOP");
    } else {
        ;
    }
    return x;
}
/*=========================BEGIN CONTROL LOGIC=======================*/
struct InstructionObject fetch(struct InstructionObject x){

    if(strcmp(INST_SET[x.tempPC][1], "\0")==0){//if it is a label with no instruction continue.
        return x;
    } else {
        strcpy(x.instruction, INST_SET[x.tempPC][1]);
        strcpy(x.destRegister1, INST_SET[x.tempPC][2]);
        strcpy(x.register2, INST_SET[x.tempPC][3]);
        strcpy(x.register3, INST_SET[x.tempPC][4]);
    }
    return x;
}
/*=========================READING FILE====================*/
void parseFile(char * file) {
    // read the misp file and store instructions to the memory property
    char buffer[100];
    FILE *pointerFile;
    char *token;
    char *delimiter = " ,\t\r\n";

    pointerFile = fopen(file, "r");

    if(pointerFile == NULL){
        printf("file not found");
    }

    fseek(pointerFile, 0, SEEK_SET);
    int i = 0;
    int p = 1;
    int lineHasLabel = 0;
    int weHaveSomething = 0;
    char *registerChange;
    char *registerIndexChange;
    while(i < 100 && fgets(buffer, 100, pointerFile)){
        if(buffer[0] == '\n' || buffer[0] == '#' || buffer[0] == '0' || buffer[0] =='\r'){
            continue;
        }
        if(buffer[0] == '\t' && strlen(buffer) == 0){
            continue;
        }
        if(isBlankLine(buffer)){
            continue;
        }
        for(token = strtok(buffer, delimiter); token; token=strtok(NULL, delimiter)){
            if(token[0] == '#'){ // a comment
                break;
            }
            if(strcmp(token, "$zero") == 0){
                token = "$0";
            }
            else if(token[strlen(token)-1] == ':'){ //labels end with :
                lineHasLabel = 1;
                strcpy(INST_SET[i][0], token);
                weHaveSomething++;
                continue;
            }
            else if(token[strlen(token)-1] == ')'){
                registerIndexChange = strtok(token, "(");
                registerChange = strtok(NULL, "()");
                strcpy(INST_SET[i][p++], registerChange);
                strcpy(INST_SET[i][p++], registerIndexChange);
            }
            strcpy(INST_SET[i][p++], token);
            weHaveSomething++;

        }
        if(lineHasLabel == 0){
            strcpy(INST_SET[i][0], "NULL");
        }
        if(weHaveSomething == 0){
            i--;
        }
        i++;
        p=1;
        lineHasLabel = 0;
        weHaveSomething = 0;
    }

    INST_SET_LENGTH = i;
    fclose(pointerFile);
}

int is_hazard(struct InstructionObject instruction1, struct InstructionObject instruction2) {

    int flag = 0;

    if (strcmp(instruction1.destRegister1, instruction2.destRegister1) == 0 ||
        strcmp(instruction1.destRegister1, instruction2.register2) == 0 ||
        strcmp(instruction1.destRegister1, instruction2.register3) == 0 ||
        strcmp(instruction1.register2, instruction2.destRegister1) == 0 ||
        strcmp(instruction1.register3, instruction2.destRegister1) == 0) {

        flag = 1;

    } else if (
            strcmp(instruction2.instruction, "sw") == 0   ||
            strcmp(instruction2.instruction, "lw") == 0   ||
            strcmp(instruction2.instruction, "j") == 0    ||
            strcmp(instruction2.instruction, "jr")  == 0  ||
            strcmp(instruction2.instruction, "jal") == 0  ||
            strcmp(instruction2.instruction, "beq") == 0  ||
            strcmp(instruction2.instruction, "bne") == 0  ||
            strcmp(instruction2.instruction, "slt") == 0)  {
        flag = 1;
        return flag;
    }  else {
        flag = 0;
    }
    return flag;
}

void *ThreadedInstructions(void *arg){
    pthread_mutex_lock(&lock);
    long tid;
    tid = (long)arg;

    struct InstructionObject instFirst;
    struct InstructionObject instSecond;
    instFirst.tempPC = PC;
    instSecond.tempPC = PC+1;
    instFirst = fetch(instFirst);
    instSecond = fetch(instSecond);

    if(is_hazard(instFirst, instSecond) == 1){ //if Hazard, do not unlock the next thread.Continue as normal.
        instFirst = decode(instFirst);
        instFirst = execute(instFirst);
        instFirst = mem(instFirst);
        instFirst = writeBack(instFirst);
        advancePC(instFirst);
        pthread_mutex_unlock(&lock);
        return NULL;
    } else if(is_hazard(instFirst, instSecond) == 0){
        instFirst = decode(instFirst);
        instFirst = execute(instFirst);
        advancePC(instFirst);
        pthread_mutex_unlock(&lock); //begin multithreading.
        instFirst = mem(instFirst);
        instFirst = writeBack(instFirst);
        return NULL;
    }
    return NULL;
}

void create_cache() {
    int i;
    for (i = 0; i < CACHE_MEMORY_SIZE; i++) {
        CACHE_ARRAY[i].tag = 0;
        CACHE_ARRAY[i].data = 0;
        CACHE_ARRAY[i].valid = 0;
    }
}

int main()
{
    create_cache();
    printf("HAR MIPS Processor v.2\n");
    printf("\nProcessing File into Memory...");
    parseFile("bubble.asm");
    printf("\nCompleted Reading File to Memory.");
    printf("\nInitializing 5 threads to start computing data.\n");

    do{
        long i = 0;
        int err;

        if(pthread_mutex_init(&lock, NULL) != 0){
            printf("\nMutex failed");
            return 1;
        }

        while(i<5){
            err = pthread_create(&(tid[i]), NULL, &ThreadedInstructions, (void *)i);
            if(err != 0)
                printf("\nCan't create thread :[%s]", strerror(err));
            i++;
        }

        pthread_join(tid[0], NULL);
        pthread_join(tid[1], NULL);
        pthread_join(tid[2], NULL);
        pthread_join(tid[3], NULL);
        pthread_join(tid[4], NULL);
        pthread_mutex_destroy(&lock);

    }while(PC<INST_SET_LENGTH);

    int a;
    printf("\nAll non-zero Register Values\n");
    for (a = 0; a <= 31; a++) {
        if(REGISTER_FILE[a] != 0){
            printf("[%s, %d]\n", registerNames[a], REGISTER_FILE[a]);
        }
    }

    int b;
    for(b=0; b<1000; b++){
        if(MEMORY[b] != 0){
            printf("\nMemory[%d] = %d", b, MEMORY[b]);
        }
    }
    printf("\nCache miss total count: %d\n", TOTAL_MISS_COUNTER);
    printf("Cache hit total count: %d\n", TOTAL_HIT_COUNTER);
    return 0;
}