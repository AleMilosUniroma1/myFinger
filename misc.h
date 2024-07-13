#ifndef MISC_H
#define MISC_H

#include <time.h>

// Flags
extern int lflag, mflag, pflag, sflag;
extern int sheader;
extern int allusersflag;
extern int headerflag;

#define PROGRAM_NAME "myFinger"

/* Header Titles whitespace offsets */
#define H_LOGIN 13
#define H_NAME 12
#define H_TTY 11
#define H_IDLE 10
#define H_LOGIN_TIME 18
#define H_OFFICE 14
#define H_OFFICE_PHONE 12

// Mem wrappers utils to make allocations safe

/* A helper function called by each memory wrapper function to manage errors
 * or return values safely.
 */
void *memcheck(const char *name, void *mem);

/* Allocate memory with malloc and call memcheck to verify the allocation. */
void *xmalloc(size_t size);

/* Reallocate memory with realloc and call memcheck to verify the allocation. */
void *xrealloc(void *old, size_t size);

/* Duplicate a string with strdup and call memcheck to verify the allocation. */
void *xstrdup(const char *s);

/* A struct definition that represents a set of strings, including an array
 * of char array entries and the number of entries saved.
 */
typedef struct {
  char **entries;
  size_t length;

} StringSet;

// StringSet Methods

/* Add an entry to the set if and only if it is not already present */
int set_add(StringSet *set, char *entry);

/* Free the memory allocated for the set. */
void set_free(StringSet *set);

/* Check if an input name matches a logged user in the utmp file.
 * If a user with the given name in GECOS is found, return 1.
 * Return 0 otherwise.
 */
int real_name_matches_user(char *name, char username_buffer[80]);

// Functions to fetch file data

/* Get plan information from a user's home directory if a .plan file is found.
 */
void get_plan(char *homedir);

/* Get forward information from a user's home directory if a .forward file is
 * found. */
void get_forward(char *homedir);

/* Get project information from a user's home directory if a .project file is
 * found. */
void get_project(char *homedir);

/* Get PGP key information from a user's home directory if a .pgpkey file is
 * found. */
void get_pgpkey(char *homedir);

/* Check if there is a /var/mail/USERNAME file. If not, write "No Mail." to
 * stdout. */
void get_mail(char *username);

// Formatters

/* Format the phone numbers according to the man finger specification:
 * 11 digits are formatted as "+N-NNN-NNN-NNNN"
 * 5 digits are formatted as "xN-NNNN"
 * 4 digits are formatted as "xNNNN"
 * otherwise they are formatted as they appear.
 */
char *format_phone_number(char *phone_number);

/* Get the input time as hours and minutes, formatted, and set it to the input
 * buffer. */
void get_hour_minutes(char *buffer, time_t time_s);

/* Format the login time according to the man finger specification:
   "Login time is displayed as month, day, hours and minutes, unless more than
   six months ago, in which case the year is displayed rather than the hours and
   minutes"
 */
void get_format_time(char *buffer, int time_s);

// Functions to fetch user informations

/* Get informations about a user given the username and the utmp record */
void get_user_info(char *username, struct utmp record);

/* Get all user information and write it to stdout. */
void get_all_users();

/* Get Idle time and tty write status */
void get_idle(char tty[32], time_t *idle, bool *writable);

/* Extract all the informations from gecos and manage the write to stdout,
   by craeting an array with these informations
   0: Real Name
   1: Building and Room number
   2: Cellphone
   3: Homephone
   4: Other
*/
void extract_gecos(char *gecos, int called);

/* Print the finger header with appropriate spacing between header titles. */
void print_header();
#endif