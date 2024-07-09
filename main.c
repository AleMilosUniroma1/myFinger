#include <fcntl.h>
#include <getopt.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>

char PROGRAM_NAME[9] = "myFinger";

int lflag, mflag, pflag, sflag;

int sheader;
int allusersflag = 1;

int headerflag;
int H_LOGIN = 13;
int H_NAME = 12;
int H_TTY = 11;
int H_IDLE = 10;
int H_LOGIN_TIME = 18;
int H_OFFICE = 14;
int H_OFFICE_PHONE = 12;

int real_name_matches_user(char *name, char *username_buffer) {
  // Check for each USER_PROCESS if the gecos name is equal to name.
  // YES ? -> return 1 and set username_buffer to the found username.
  // NO ? return 0;
}

void *memcheck(const char *name, void *mem) {
  if (!mem) {
    perror(name);
    exit(EXIT_FAILURE);
  }
  return mem;
}

void *xmalloc(size_t size) { return memcheck("malloc", malloc(size)); }

void *xrealloc(void *old, size_t size) {
  return memcheck("realloc", realloc(old, size));
}

void *xstrdup(const char *s) { return memcheck("strdup", strdup(s)); }

typedef struct {
  char **entries;
  size_t length;

} StringSet;

int set_add(StringSet *set, char *entry) {
  for (size_t i = 0; i < set->length; i++) {
    if (strcmp(set->entries[i], entry) == 0) {
      return 0;
    }
  }

  if (set->length % 16 == 0) {
    set->entries = realloc(set->entries, (set->length + 16) * sizeof(char *));
  }

  // Malloc + memcpy
  set->entries[set->length] = xstrdup(entry);
  set->length++;

  return 1;
}

void set_free(StringSet *set) {

  if (set->entries == NULL) {
    return;
  }

  for (size_t i = 0; i < set->length; i++) {
    free(set->entries[i]);
  }

  free(set->entries);
  set->entries = NULL;
  set->length = 0;
}

/**
 * 11 digits are formatted as "+N-NNN-NNN-NNNN"
 * 5 digits are formatted as "xN-NNNN"
 * 4 digits are formatted as "xNNNN"
 * otherwise they are formatted as they appear.
 */
char *format_phone_number(char *phone_number) {
  char *buf;
  int phone_idx = 0;
  if (strlen(phone_number) == 11) {

    buf = xmalloc(5 + strlen(phone_number));
    buf[0] = '+';
    for (int i = 1; i < 4 + strlen(phone_number); i++) {
      if (i == 2 || i == 6 || i == 10) {
        buf[i] = '-';
      } else {
        buf[i] = phone_number[phone_idx++];
      }
    }

    buf[strlen(phone_number) + 4] = '\0';
  } else if (strlen(phone_number) == 10) {

    buf = xmalloc(3 + strlen(phone_number));
    for (int i = 0; i < 2 + strlen(phone_number); i++) {
      if (i == 3 || i == 7) {
        buf[i] = '-';
      } else {
        buf[i] = phone_number[phone_idx++];
      }
    }

    buf[strlen(phone_number) + 2] = '\0';
  } else if (strlen(phone_number) == 5) {
    buf = xmalloc(3 + strlen(phone_number));
    buf[0] = 'x';
    for (int i = 1; i < 2 + strlen(phone_number); i++) {
      if (i == 2) {
        buf[i] = '-';
      } else {
        buf[i] = phone_number[phone_idx++];
      }
    }

    buf[strlen(phone_number) + 2] = '\0';
  } else if (strlen(phone_number) == 4) {
    buf = xmalloc(2 + strlen(phone_number));
    buf[0] = 'x';
    for (int i = 1; i < 2 + strlen(phone_number); i++) {
      buf[i] = phone_number[phone_idx++];
    }

    buf[strlen(phone_number) + 1] = '\0';
  } else {
    buf = xmalloc(1 + strlen(phone_number));
    for (int i = 0; i < strlen(phone_number); i++) {
      buf[i] = phone_number[phone_idx++];
    }
    buf[strlen(phone_number)] = '\0';
  }

  return buf;
}

void get_plan(char *homedir) {
  struct FILE *plan;

  char *filename = xmalloc(7 + strlen(homedir));
  sprintf(filename, "%s/.plan", homedir);
  plan = fopen(filename, "r");
  free(filename);

  if (plan == NULL) {
    printf("No Plan.\n");
    return;
  } else {
    char ch;

    while ((ch = fgetc(plan)) != EOF) {
      printf("%c", ch);
    }
    printf("\n");
  }

  fclose(plan);
}

void get_forward(char *homedir) {
  struct FILE *forward;

  char *filename = xmalloc(10 + strlen(homedir));
  sprintf(filename, "%s/.forward", homedir);
  forward = fopen(filename, "r");
  free(filename);

  if (forward == NULL) {
    return;
  } else {
    char ch;

    while ((ch = fgetc(forward)) != EOF) {
      printf("%c", ch);
    }
    printf("\n");
  }

  fclose(forward);
}

void get_project(char *homedir) {
  struct FILE *project;

  char *filename = xmalloc(10 + strlen(homedir));
  sprintf(filename, "%s/.project", homedir);
  project = fopen(filename, "r");
  free(filename);

  if (project == NULL) {
    return;
  } else {
    char ch;

    while ((ch = fgetc(project)) != EOF) {
      printf("%c", ch);
    }
  }

  fclose(project);
}

void get_pgpkey(char *homedir) {
  struct FILE *pgpkey;

  char *filename = xmalloc(9 + strlen(homedir));
  sprintf(filename, "%s/.pgpkey", homedir);
  pgpkey = fopen(filename, "r");
  free(filename);

  if (pgpkey == NULL) {
    return;
  } else {
    char ch;

    while ((ch = fgetc(pgpkey)) != EOF) {
      printf("%c", ch);
    }
    printf("\n");
  }
  fclose(pgpkey);
}

void get_mail(char *username) {
  char *buf = xmalloc(strlen(_PATH_MAILDIR) + strlen(username));
  snprintf(buf, 1024, "%s/%s", _PATH_MAILDIR, username);

  struct FILE *mail;

  mail = fopen(buf, "r");
  if (mail == NULL) {
    printf("No mail.\n");
    return;
  } else {
    printf("There is a MAILLLLL!\n");
  }
  free(buf);

  fclose(mail);
}

void get_idle(char tty[32], time_t *idle, bool *writable) {
  /* terminal device for the user */
  struct stat f_info;

  char *buf = xmalloc(6 + strlen(tty));
  sprintf(buf, "/dev/%s", tty);
  int ret = stat(buf, &f_info);
  free(buf);

  if (ret < 0) {
    *idle = 0;
    *writable = 0;
    return;
  }

#define TALKABLE 0220
  *writable = ((f_info.st_mode & TALKABLE) == TALKABLE);

  time_t now = time(NULL);
  *idle = now < f_info.st_atime
              ? 0
              : now - f_info.st_atime - 60 * 60; // adjust by an hour
}

/*
    Create a char array with the following content.
    // 0: Nome Account
    // 1: Edificio e Numero di stanza
    // 2: Numero Cellulare
    // 3: Numero Casa
    // 4: Altro (potrebbe non esserci)
*/
void extract_gecos(char *gecos, int called) {

  char fields[5][80] = {"", "", "", "", ""};

  int field_index = 0;
  int word_index = 0;

  char word[80] = "";
  for (int i = 0; i < strlen(gecos); i++) {
    if (gecos[i] == ',') {
      // Assign the gecos substring to the correct fields index
      strcpy(fields[field_index], word);

      // Clear the string
      memset(word, 0, sizeof(word));

      // Update indexes
      word_index = 0;
      field_index++;
    } else {
      // Append the character
      word[word_index++] = gecos[i];
    }
  }

  strcpy(fields[field_index], word); // add the last one

  char *name = fields[0];
  char *building_and_room = fields[1];
  char *cellphone = fields[2];
  cellphone = format_phone_number(cellphone);
  char *homephone = fields[3];
  homephone = format_phone_number(homephone);
  char *other = fields[4];

  if (called == 0 && sflag == 1) {
    printf("%-*s", H_NAME, name);
  }

  if (called == 0 && lflag == 1) {
    printf("Name: %s\n", name);
  }

  if (called == 1 && sflag == 1) {
    printf("%-*s", H_OFFICE, building_and_room);
    if (strlen(cellphone) > 0) {
      cellphone = format_phone_number(cellphone);
      printf("%-*s", H_OFFICE_PHONE, cellphone);
    }
  }

  if (called == 1 && lflag == 1) {
    int c = 0;
    int tot = 0;
    if (strlen(building_and_room) > 0) {
      tot += strlen(building_and_room);
      c++;
    }

    if (strlen(cellphone) > 0) {
      tot += strlen(cellphone);
      c++;
    }
    if (strlen(other) > 0) {
      tot += strlen(other);
      c++;
    }

    if (c > 0) {
      printf("Office: ");
    }

    if (strlen(building_and_room) > 0) {
      printf("%s", building_and_room);
      if (--c > 0) {
        printf(", ");
      }
    }
    if (strlen(cellphone) > 0) {
      printf("%s", cellphone);
      if (--c > 0) {
        printf(", ");
      }
    }

    if (strlen(other) > 0) {
      printf("%s", other);
    }

    if (strlen(homephone) > 0) {
      if (tot < 30) {
        printf("%-15sHome Phone: %s\n", "", homephone);
      } else {
        printf("\nHome Phone: %s\n", homephone);
      }
    }
  }
}

void get_hour_minutes(char *buffer, time_t time_s) {
  struct tm *timeinfo;
  timeinfo = localtime(&time_s);

  char *format = "%I:%M";
  strftime(buffer, 80, format, timeinfo);
}

void get_format_time(char *buffer, int time_s) {
  struct tm *timeinfo;
  time_t login_time = (time_t)time_s;
  timeinfo = localtime(&login_time);

  time_t now = time(0);
  time_t six_months_s = 60 * 60 * 24 * 30 * 6;
  time_t passed = now - login_time;

  if (passed <= six_months_s) {
    // Login time is displayed as month, day, hours and minutes
    // If the time passed from last login is less than 6 months.
    char *format;
    if (sflag == 1) {
      char *format = "%b  %d %H:%M";
      strftime(buffer, 80, format, timeinfo);
    } else {
      char *format = "%a %b  %d %H:%M";
      strftime(buffer, 80, format, timeinfo);
    }
  } else {
    //  unless more than six months ago, in which case the year
    //  is displayed rather than the hours and minutes.
    char *format = "%Y";
    strftime(buffer, 80, format, timeinfo);
  }
}

void get_user_info(char *username, struct utmp record) {
  struct passwd *pwd = getpwnam(username);

  if (pwd == NULL) {
    fprintf(stderr, "%s: %s: no such user\n", PROGRAM_NAME, username);
    exit(EXIT_FAILURE);
  }

  time_t *idle;
  bool *writable;
  get_idle(record.ut_line, &idle, &writable);

  int time_s;
  time_s = record.ut_tv.tv_sec;
  time_t login_time = (time_t)time_s;
  char buffer_time[80];

  if (sflag == 1) {

    printf("%-*s", H_LOGIN, username);
    extract_gecos(pwd->pw_gecos, 0);
    if (writable) {
      printf("%-*s", H_TTY, record.ut_line);
    } else {
      printf("*%-*s", H_TTY, record.ut_line);
    }
    if (idle > 0) {
      char buffer[80];
      get_hour_minutes(buffer, idle);
      printf("%-*s", H_IDLE, buffer);
    } else {
      printf("%-*s", H_IDLE, "");
    }

    get_format_time(buffer_time, login_time);
    printf("%-*s", H_LOGIN_TIME, buffer_time);

    extract_gecos(pwd->pw_gecos, 1);
  }

  if (lflag == 1) {
    printf("Login: %-30s", username);
    extract_gecos(pwd->pw_gecos, 0);
    printf("Directory: %-26sShell: %s\n", pwd->pw_dir, pwd->pw_shell);
    extract_gecos(pwd->pw_gecos, 1);

    get_format_time(buffer_time, time_s);

    time_t t = time(NULL);
    struct tm lt = {0};
    localtime_r(&t, &lt);

    printf("On Since %s (%s) on %s\n", buffer_time, lt.tm_zone, record.ut_line);

    get_mail(username);
    if (pflag == 0) {
      get_plan(pwd->pw_dir);
      get_project(pwd->pw_dir);
      get_pgpkey(pwd->pw_dir);
    }
    get_forward(pwd->pw_dir);
  }
}

void get_all_users() {
  struct utmp record;

  int fd = open(_PATH_UTMP, O_RDONLY);
  int record_size = sizeof(record);

  if (fd < 0) {
    perror("Cannot open utmp file.");
    exit(1);
  }

  StringSet usernames = {NULL, 0};

  while (read(fd, &record, record_size) == record_size) {
    if (record.ut_type == USER_PROCESS) {
      if (lflag == 1) {
        if (set_add(&usernames, (char *)record.ut_user)) {
          get_user_info((char *)record.ut_user, record);
          printf("\n");
        }
      } else {
        get_user_info((char *)record.ut_user, record);
        printf("\n");
      }
    }
  }

  set_free(&usernames);
}

void print_header() {
  if (lflag == 0 && headerflag == 0) {
    headerflag = 1;
    printf("Login%8sName%8sTty%8sIdle%6sLogin Time%8sOffice%8sOffice Phone\n",
           "", "", "", "", "", "");
  }
}

int main(int argc, char **argv) {
  // NO OPT passed ?
  // If nothing is passed in argv, default is -s.
  // If a username is passed in argv, default is -l.
  //    If username is not found -> sterr.

  int opt;
  // I can either have -opt, or -opt <optarg> for each of "slpm"
  while ((opt = getopt(argc, argv, "lmpsh")) != EOF) {
    switch (opt) {
    case 'l':
      lflag = 1; // lflag wins over sflag
      break;
    case 's':
      sflag = 1;
      break;
    case 'p':
      pflag = 1;
      break;
    case 'm':
      mflag = 1;
      break;

    case 'h':
    default:
      printf("usage: %s [-lmps] [name or username]\n", argv[0]);
      return 1;
    }
  }

  StringSet usernames = {NULL, 0};

  // Find usernames passed in command line
  while (optind < argc) {
    // Here we
    if (argv[optind][0] != '-') {
      // -l option is set by default if username is passed in parameters
      allusersflag = 0;
      if (sflag == 0) {
        lflag = 1;
      }
      print_header();

      struct utmp utmp_record;

      int fd = open(_PATH_UTMP, O_RDONLY);
      int record_size = sizeof(utmp_record);
      while (read(fd, &utmp_record, record_size) == record_size) {
        if (utmp_record.ut_type == USER_PROCESS) {
          if (strcmp(utmp_record.ut_user, argv[optind]) == 0) {
            if (sflag == 1) {
              get_user_info(argv[optind], utmp_record);
              printf("\n");
            } else if (set_add(&usernames, utmp_record.ut_user)) {
              get_user_info(argv[optind], utmp_record);
              printf("\n");
            }
          } else {
            // Check for user by Name
          }
        }
      }
      // Se ci sono usernames qui dentro, eseguire myFinger esclusivamente su
      // questi. Altrimenti possiamo procedere su tutti i logged in users in
      // utmp.
    }

    optind++;
    set_free(&usernames);
  }

  if (allusersflag == 1) {
    if (lflag == 0) {
      sflag = 1;
    }
    // check finger for all users
    print_header();
    get_all_users();
  }
}
