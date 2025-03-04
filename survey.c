
/* =============================================================================

SENG 265 -- Assignment 1
Matthew Laforce
V01019219
September 22, 2024

============================================================================= */

/*    INCLUSIONS    */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
// ----------------------------------------------------------------------------|

/*    QUESTION CATEGORY SIZING    */
#define C_QUEST 8       // Number of entries in the 'C' question category
#define I_QUEST 10      // Number of entries in the 'I' question category
#define G_QUEST 10      // Number of entries in the 'G' question category
#define U_QUEST 6       // Number of entries in the 'U' question category
#define P_QUEST 4       // Number of entries in the 'P' question category
// ----------------------------------------------------------------------------|

/*    QUESTION / LIKERT ARRAY VARIABLES    */
#define Q_LEN 300                       // Size for buffering one question
#define Q_COUNT (C_QUEST + I_QUEST + G_QUEST + U_QUEST + P_QUEST) // Total Question Number
#define Q_BUFFER (Q_COUNT * Q_LEN)      // Size for a question array buffer
#define L_LEN 40                        // Size for buffering one Likert
#define L_COUNT 6                       // Total Likert string count
#define L_BUFFER (L_COUNT * L_LEN)      // Size for a Likert array buffer
int unused_inputs = 3;                  // Junk input count in student answers
// ----------------------------------------------------------------------------|

/*    OTHER BUFFER SIZING    */
#define JUNK_LEN 300        // Extra buffer space for unused student inputs
#define MAX_STUDENTS 100    // Upper bound on the count of students polled
// ----------------------------------------------------------------------------|

/*    TEST BITS    */
int bit_1 = 0;  // Test 1: Print header, response count, questions, likerts
int bit_2 = 0;  // Test 2: Test 1 stuff, plus student count, response percents
int bit_3 = 0;  // Test 3: Prints header, student count, average scores
int bit_4 = 0;  // Test 4: Prints overall averages
// ----------------------------------------------------------------------------|

/*    QUESTION SCORES    */
float c_score = 0;
float i_score = 0;
float g_score = 0;
float u_score = 0;
float p_score = 0;
// ----------------------------------------------------------------------------|

/*    GLOBAL STORAGE ARRAYS    */
char questions[Q_COUNT * Q_LEN];        // Array containing question strings
int reversals[Q_COUNT * sizeof(int)];   // Array containing binary Likert directions
char likerts[Q_COUNT * L_LEN];          // Array which containing Likert strings
int *answers;                           // Array of Likert integers

/* =============================================================================

    HELPER FUNCTIONS

============================================================================= */

// Skips past comment lines in 'stdin'
void skip_comments () {
    /*
    When called, this function skips an arbitrary number of 'comment lines' in
    the input text by removing lines beginning with '#' from stdin.
    */
    int byte;
    // Use 'getchar()' to consume lines until reaching a non-comment line
    while ((byte = getchar()) == '#') {
        while (byte != '\n' && byte != EOF) {
            byte = getchar();
        }
    }
    // Push the final character back to avoid consuming actual input
    ungetc(byte, stdin);
    return;
}

// Handling for the test byte line
void test_bytes () {
    /* 
    This function sets the test bits; it simply checks for any active bits 
    in input text using getchar(), switching the global bit integers to a 
    '1' if they are set.
    */
    char byte;
    // Set the bit if the character is 1
    if ((byte = getchar()) == '1') {
        bit_1 = 1;
    }
    // Discard the seperation character ',' then repeat for other values
    byte = getchar();
    if ((byte = getchar()) == '1') {
        bit_2 = 1;
    }
    byte = getchar();
    if ((byte = getchar()) == '1') {
        bit_3 = 1;
    }
    byte = getchar();
    if ((byte = getchar()) == '1') {
        bit_4 = 1;
    }
    byte = getchar();
    return;
}

// Logic for placing raw text input into a global array
void fill_array(char *array, int count, int longest, int buffer_len, char special) {
    /*
    Accepts a global array, a count of items to insert, the size of a single item,
    the size of the overall array, and a seperation character such as a semicolon.
    Uses this information to populate '*array' as part of input handling.
    */
    // Use 'fgets' to place the input for 'fill_array' into 'buffer'
    char buffer[buffer_len];
    fgets(buffer, buffer_len, stdin);
    // Create a termination string, call 'strtok' to seperate the input
    char seperator[2] = {special, '\0'};
    char *token = strtok(buffer, seperator);
    int current = 0;
    // Loop until reaching 'count' or emptying 'buffer'
    while (current < count && token != NULL) {

        // Use 'offset' to track insertion position; null-terminate the string
        int offset = (current * longest);
        strncpy(&array[offset], token, longest - 1);
        array[offset + strlen(token)] = '\0';
        // Handle newlines that sneak into the final array position
        if (token[strlen(token) - 1] == '\n') {
            array[offset + strlen(token) - 1] = '\0';
        }
        // Generate a new token, loop again
        token = strtok(NULL, seperator);
        current++;

    }
    return;
}

// Detects and stores forward/reverse question status as a binary string
void get_reversals () {
    /*
    Similar to how 'fill_array()' populates string arrays, 'get_reversals()' 
    populates an array of binary integers. This allows the direct/reverse
    status of Likert questions to be easily accessed.
    */
    int special = ';';
    for (int index = 0; index < Q_COUNT; index++) {
        // Final input handling - sections end in a '\n' character
        if ((index + 1) == Q_COUNT) {
            special = '\n';
        }
        int byte = getchar();
        // Track reversals by assigning direct questions '0', reversed '1'
        if (byte == 'D') {
            reversals[index] = 0;
        } else {
            reversals[index] = 1;
        }
        // Proceed until reaching 'special', then repeat
        while (byte != special) {
            byte = getchar();
            if (byte == EOF) {
                break;
            }
        }
    }
    return;
}

// Process the answers of a single student
void process_student (int student_offset) {
    /*
    Accepts as input 'student_offset', an integer representing the current array
    position to hold the answers of this stdent. Fills array 'answers' with
    integers representing the direct Likert value of the student's answers.

    This function handles the answers of a single student. It does not account for 
    the direct / reversed status of a given question, rather it treats each answer
    as if it were direct.
    */
    char this_answer[L_LEN];
    int skipped_inputs = 0;
    // Buffer the student input, stopping at a newline
    char buffer[(L_LEN * Q_COUNT) + JUNK_LEN];
    fgets(buffer, sizeof(buffer), stdin);
    int current = 0;
    // Make a seperator for 'strtok'
    char seperator[2] = {',', '\0'};
    // Create a token to use in 'buffer'
    char *token = strtok(buffer, seperator);
    // Skip unused junk input like 'date' and 'major'
    while (token != NULL && skipped_inputs < unused_inputs) {
        token = strtok(NULL, ",");
        skipped_inputs++;
    }
    // Loop until reaching 'count' or emptying the input buffer
    while (current < Q_COUNT && token != NULL) {

        int likert_val = 0;
        int match_found = 0;
        int match_position = 0;
        // Copy and null-terminate student answers from 'token'
        strncpy(this_answer, token, L_LEN - 1);
        this_answer[strlen(token)] = '\0';
        // Remove junk '\n' characters from the final answer of each student
        int final = strlen(this_answer);
        if (final > 0 && this_answer[final - 1] == '\n') {
            this_answer[final - 1] = '\0';
        }
        // Check this current student answer
        for (int index = 0; index < L_COUNT; index++) {

            // Compare the answer to each Likert value to find a match
            int offset = (index * L_LEN);
            if (strncmp(this_answer, &likerts[offset], L_LEN) == 0) {
                match_found = 1;
                match_position = index;
                break;
            }

        }
        // When a match is found, set 'likert_val' then break
        if (match_found == 1) {
            likert_val = (match_position + 1);
        }
        // Set the array value to 'likert_val', repeat
        answers[student_offset + current] = likert_val;
        token = strtok(NULL, seperator);
        current++;
    }
    return;
}

/* =============================================================================

    MAIN FUNCTION

============================================================================= */

int main() {

    /* ==============================
    #######   INPUT HANDLING   #######    
    ============================== */

    char byte;
    int section = 0;
    int num_students = 0;
    // Use a loop plus 'section' accumulator to control input flow handling
    while ((byte = getchar()) != EOF) {
        // Return valid input to stdin; handle comments using 'skip_comments()'
        ungetc(byte, stdin);
        skip_comments();

        /*=== SECTION 0 -- Test byte logic ================================== */
        if (section == 0) {
            test_bytes();
            section++;
        }
        /*=== SECTION 1 -- Question Array =================================== */
        else if (section == 1) {
            // Populate global array to hold the question strings
            fill_array(questions, Q_COUNT, Q_LEN, Q_BUFFER, ';');
            section++;
        }
        /*=== SECTION 2 -- Reversal Array =================================== */
        else if (section == 2) {
            // A binary string representing direct / reversed status for questions
            get_reversals(reversals);
            section++;
        }
        /*=== SECTION 3 -- Likert Array ===================================== */
        else if (section == 3) {
            // Populate global array to hold Likert strings
            fill_array(likerts, L_COUNT, L_LEN, L_BUFFER, ',');
            section++;
        }
        /*=== SECTION 4 -- Answer Array ===================================== */
        else {
            // Skip this section completely if bits 2, 3, and 4 are off
            if (bit_2 == 0 && bit_3 == 0 && bit_4 == 0) {
                break;
            }
            // Size an integer array 'answers'
            answers = (int *) malloc(Q_COUNT * MAX_STUDENTS * sizeof(int));
            // Iterate until every student's answers have been examined
            while ((byte = getchar()) != EOF) {
                int student_offset = (num_students * Q_COUNT);
                process_student(student_offset);
                num_students++;
            }
            break;
        }
    }

    /* ===============================
    #######   OUTPUT HANDLING   #######    
    =============================== */

    printf("Examining Science and Engineering Students' Attitudes Towards ");
    printf("Computer Science\n");
    printf("SURVEY RESPONSE STATISTICS\n\n");
    printf("NUMBER OF RESPONDENTS: %d\n\n", num_students);

    /* === QUESTIONS AND LIKERTS =============================================== */
    // If bits 1 or 2 are set, print questions and Likert statements
    if (bit_1 == 1 || bit_2 == 1) {
        printf("FOR EACH QUESTION BELOW, RELATIVE PERCENTUAL FREQUENCIES ");
        printf("ARE COMPUTED FOR EACH LEVEL OF AGREEMENT\n");
        for (int index = 0; index < Q_COUNT; index++) {

            // Print the current question
            printf("\n");
            int question_offset = (index * Q_LEN);
            char byte;
            while ((byte = questions[question_offset]) != '\0') {
                printf("%c", byte);
                question_offset++;
            }
            printf("\n");

            // Print the Likert statement, the response rates
            for (int likert = 0; likert < L_COUNT; likert++) {
                float percent = 0.0;
                if (bit_2 == 1) {

                // Calculate and print the response rates
                    float responses = 0;
                    for (int nested = 0; nested < num_students; nested++) {
                        // Match stored likert integers with definition values
                        int likert_offset = (nested * Q_COUNT);
                        if (answers[index + likert_offset] == (likert+1)) {
                            responses++;
                        }
                    }
                    percent = (100 * responses / num_students);
                }
                printf("%.2f: ", percent);

                // Print the matching Likert statement
                int like_offset = (likert * L_LEN);
                while ((byte = likerts[like_offset]) != '\0') {
                    printf("%c", byte);
                    like_offset++;
                }
                printf("\n");
            }
        }
    }

    /* === STUDENT SCORES AND AVERAGES ========================================= */
    // If bits 3 or 4 are set, print student scores and/or averages
    if (bit_3 == 1 || bit_4 == 1) {
        if (bit_3 == 1) {
            if (bit_2 == 1) {
                printf("\n");
            }
            printf("SCORES FOR ALL THE RESPONDENTS\n\n");
        }
        // Calculate average score for each student
        for (int index = 0; index < num_students; index++) {

            // Initialize variables to hold student data
            float this_c = 0.0;
            float this_i = 0.0;
            float this_g = 0.0;
            float this_u = 0.0;
            float this_p = 0.0;
            // Offset here represents 'student-sized' jumps in array position
            int offset = (index * Q_COUNT);
            // Handle each student's answers
            for (int question = 0; question < Q_COUNT; question++) {

                int reversed = 0;
                float cur_score = 0.00;
                reversed = reversals[question];
                cur_score = answers[question + offset];
                // For indirect questions, invert the score with 'L_COUNT'
                if (reversed == 1) {
                    cur_score = (L_COUNT + 1 - cur_score);
                }
                // Track scores of this student, the sum of all student scores
                if (question < C_QUEST) {
                    c_score += cur_score;
                    this_c += cur_score;
                } else if (question < C_QUEST + I_QUEST) {
                    i_score += cur_score;
                    this_i += cur_score;
                } else if (question < C_QUEST + I_QUEST + G_QUEST) {
                    g_score += cur_score;
                    this_g += cur_score;
                } else if (question < C_QUEST + I_QUEST + G_QUEST + U_QUEST) {
                    u_score += cur_score;
                    this_u += cur_score;
                } else {
                    p_score += cur_score;
                    this_p += cur_score;
                }
            }
        // If 'bit_3' is set, print the average scores for each student
            if (bit_3 == 1) {
                this_c /= C_QUEST;
                this_i /= I_QUEST;
                this_g /= G_QUEST;
                this_u /= U_QUEST;
                this_p /= P_QUEST;
                printf("C:%.2f,I:%.2f,G:%.2f,", this_c, this_i, this_g);
                printf("U:%.2f,P:%.2f\n", this_u, this_p);
            }
        }
        // If 'bit_4' is set, print the overall student average score
        if (bit_4 == 1) {
            printf("\nAVERAGE SCORES PER RESPONDENT\n\n");
            c_score /= (C_QUEST * num_students);
            i_score /= (I_QUEST * num_students);
            g_score /= (G_QUEST * num_students);
            u_score /= (U_QUEST * num_students);
            p_score /= (P_QUEST * num_students);
            printf("C:%.2f,I:%.2f,G:%.2f,", c_score, i_score, g_score);
            printf("U:%.2f,P:%.2f\n", u_score, p_score);
        }
    }
    exit(0);
}
/* ============================================================================= */

