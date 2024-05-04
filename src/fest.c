#include "config.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum sysexits
{
  EX_OK = 0,
  EX_USAGE = 64,
  EX_NOINPUT = 66
};

enum opt_type
{
  OPT_ERROR,
  OPT_ID,
  OPT_NEXT,
  OPT_PREV
};

struct option
{
  enum opt_type type;
  int id;
};

static void show_help (void);
static inline void show_version (void);
static inline bool is_arg_help (const char *arg);
static inline bool is_arg_version (const char *arg);

static inline void assert_arg_path (const char *arg);

static inline struct option get_option (const char *arg);
static inline void fest_goto_id (int id);
static inline void fest_goto_next (void);
static inline void fest_goto_prev (void);

int
main (int argc, char **argv)
{
  switch (argc)
    {
    case 1:
      {
        show_help ();
        exit (EX_USAGE);
      }
      break;
    case 2:
      {
        if (is_arg_help (argv[1]))
          {
            show_help ();
            exit (EX_OK);
          }
        else if (is_arg_version (argv[1]))
          {
            show_version ();
            exit (EX_OK);
          }
        else
          {
            show_help ();
            exit (EX_USAGE);
          }
      }
      break;
    case 3:
      {
        assert_arg_path (argv[1]);
        struct option opt = get_option (argv[2]);
        switch (opt.type)
          {
          case OPT_ID:
            fest_goto_id (opt.id);
            break;
          case OPT_NEXT:
            fest_goto_next ();
            break;
          case OPT_PREV:
            fest_goto_prev ();
            break;
          case OPT_ERROR:
            break;
          default:
            fprintf (stderr, "ERROR: option type not recognised\n");
            break;
          }
      }
      break;
    default:
      {
        show_help ();
        exit (EX_USAGE);
      }
      break;
    }
}

void
show_help (void)
{
  perror ("fest - FEh background SeTter\n"
          "\n"
          "Usage:\n"
          "  fest <pic_list_path> (--id=<id> | --next | --prev)\n"
          "  fest --version\n"
          "  fest --help\n"
          "\n"
          "Options:\n"
          "  -h     --help    Show this screen.\n"
          "         --version Show version.\n"
          "  -i<id> --id=<id> Set background to pic with id <id> in "
          "<pic_list_path>\n"
          "  -n     --next    Set background to next pic\n"
          "  -p     --prev    Set background to previous pic\n");
}

inline void
show_version (void)
{
  perror (
      PACKAGE_STRING
      "License MIT\n"
      "\n"
      "This is free software: you are free to change and redistribute it.\n"
      "There is NO WARRANTY, to the extent permitted by law.\n"
      "\n"
      "Written by cygishere.\n");
}

inline bool
is_arg_help (const char *arg)
{
  assert (0 && "not imp\n");
  return false;
}

inline bool
is_arg_version (const char *arg)
{
  assert (0 && "not imp\n");
  return false;
}

inline void
assert_arg_path (const char *arg)
{
  assert (0 && "not imp\n");
}

inline struct option
get_option (const char *arg)
{
  assert (0 && "not imp\n");
  return (struct option){ 0 };
}

inline void
fest_goto_id (int id)
{
  assert (0 && "not imp\n");
}

inline void
fest_goto_next (void)
{
  assert (0 && "not imp\n");
}

inline void
fest_goto_prev (void)
{
  assert (0 && "not imp\n");
}
