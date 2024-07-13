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

#include "misc.h"

int main(int argc, char **argv) {
  /* NO OPT passed ?
     If nothing is passed in argv, default is -s.
     If a username is passed in argv, default is -l.
        If username is not found write it to stderr.
  */

  int opt;
  // I can either have -opt, or -opt <optarg> for each of "slpm"
  while ((opt = getopt(argc, argv, "lmpsh")) != -1) {
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

  // If both sflag and lflag are specified, lflag wins.
  if (sflag && lflag) {
    sflag = 0;
  }

  StringSet usernames = {NULL, 0};

  // Find usernames passed in command line
  while (optind < argc) {
    if (argv[optind][0] != '-') {
      // -l option is set by default if username is passed in parameters
      allusersflag = 0;
      if (sflag == 0) {
        lflag = 1;
      }

      struct utmp utmp_record;

      bool found_user = 0;
      int fd = open(_PATH_UTMP, O_RDONLY);
      int record_size = sizeof(utmp_record);
      while (read(fd, &utmp_record, record_size) == record_size) {
        if (utmp_record.ut_type == USER_PROCESS) {
          if (strcmp(utmp_record.ut_user, argv[optind]) == 0) {
            found_user = 1;
            print_header(); // only print the header if a user has been found
            if (sflag == 1) {
              get_user_info(argv[optind], utmp_record);
              printf("\n");
            } else if (set_add(&usernames, utmp_record.ut_user)) {
              get_user_info(argv[optind], utmp_record);
              printf("\n");
            }
          } else {
            // Check for user by Name only if mflag is not set
            char username_buff[80];
            int found_name =
                real_name_matches_user(argv[optind], username_buff);

            if (found_name) {
              found_user = 1;
              if (mflag == 0) {
                print_header(); // only print the header if a user has been
                                // found
                if (set_add(&usernames, username_buff)) {
                  get_user_info(username_buff, utmp_record);
                  printf("\n");
                }
              }
            }
          }
        }
      }

      if (!found_user) {
        fprintf(stderr, "%s: %s: no such user\n", PROGRAM_NAME, argv[optind]);
      }
    }

    optind++;
  }
  set_free(&usernames);

  if (allusersflag == 1) {
    if (lflag == 0) {
      sflag = 1;
    }
    // check finger for all users
    print_header();
    get_all_users();
  }
}
