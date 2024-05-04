#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FEST_PIC_LIST_LEN_INIT 5

enum sysexits
{
  EX_OK = 0,
  EX_USAGE = 64,
  EX_NOINPUT = 66,
  EX_OSERR = 71,
  EX_IOERR = 74
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

struct fest_state
{
  size_t pic_list_cap;
  size_t pic_list_len;
  const char **pic_list;

  int pic_cur_id;

  const char *pic_cur_id_path;
};

static void show_help (void);
static inline void show_version (void);
static inline bool is_arg_help (const char *arg);
static inline bool is_arg_version (const char *arg);

static inline void fest_load_pic_list (struct fest_state *fest,
                                       const char *pic_list_path);
static inline void fest_load_pic_cur_id (struct fest_state *fest,
                                         const char *pic_cur_id_path);

static inline struct option get_option (const char *arg);
static inline void fest_goto_id (struct fest_state *fest, int id);
static inline void fest_goto_next (struct fest_state *fest);
static inline void fest_goto_prev (struct fest_state *fest);

static inline void fest_write_id (struct fest_state *fest);

static void fest_pic_list_append (struct fest_state *fest,
                                  const char *pic_path);
static void fest_pic_list_free (struct fest_state *fest);

int
main (int argc, char **argv)
{
  struct fest_state fest = { 0 };

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
    case 4:
      {
        fest_load_pic_list (&fest, argv[1]);
        fest_load_pic_cur_id (&fest, argv[2]);
        struct option opt = get_option (argv[3]);
        switch (opt.type)
          {
          case OPT_ID:
            fest_goto_id (&fest, opt.id);
            break;
          case OPT_NEXT:
            fest_goto_next (&fest);
            break;
          case OPT_PREV:
            fest_goto_prev (&fest);
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

  fest_pic_list_free (&fest);
}

void
show_help (void)
{
  fprintf (stderr, "fest - FEh background SeTter\n"
                   "\n"
                   "Usage:\n"
                   "  fest <pic_list_path> <pic_cur_id_path> (--id=<id> | "
                   "--next | --prev)\n"
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

void
show_version (void)
{
  fprintf (
      stderr, PACKAGE_STRING
      "\n"
      "License MIT\n"
      "\n"
      "This is free software: you are free to change and redistribute it.\n"
      "There is NO WARRANTY, to the extent permitted by law.\n"
      "\n"
      "Written by cygishere.\n");
}

bool
is_arg_help (const char *arg)
{
  return (strcmp (arg, "--help") == 0) || (strcmp (arg, "-h") == 0);
}

bool
is_arg_version (const char *arg)
{
  return (strcmp (arg, "--version") == 0);
}

void
fest_load_pic_list (struct fest_state *fest, const char *pic_list_path)
{
  FILE *f = fopen (pic_list_path, "r");
  if (!f)
    {
      fprintf (stderr, "ERROR: failed to open %s: %s\n", pic_list_path,
               strerror (errno));
      exit (EX_NOINPUT);
    }

  char *line = NULL;
  size_t n = 0;
  ssize_t s = 0;
  while (true)
    {
      line = NULL;
      s = getline (&line, &n, f);
      if (s == -1)
        {
          free (line);
          break;
        }
      line[s - 1] = 0;
      fest_pic_list_append (fest, line);
    }
  if (errno == ENOMEM)
    {
      fprintf (stderr, "ERROR: failed to get line: %s\n", strerror (errno));
      errno = 0;
      exit (EX_OSERR);
    }

  fclose (f);
}

void
fest_load_pic_cur_id (struct fest_state *fest, const char *pic_cur_id_path)
{
  FILE *f = fopen (pic_cur_id_path, "r");
  if (!f)
    {
      if (errno == ENOENT)
        {
          fest->pic_cur_id = 0;
          return;
        }
      else
        {
          fprintf (stderr, "ERROR: failed to open %s: (%d) %s\n",
                   pic_cur_id_path, errno, strerror (errno));
          exit (EX_NOINPUT);
        }
    }

  char *line = NULL;
  size_t n = 0;
  ssize_t s = getline (&line, &n, f);
  if (s == -1)
    {
      free (line);
      fest->pic_cur_id = 0;
      return;
    }

  char *end = NULL;
  long id = strtol (line, &end, 10);
  fest->pic_cur_id = (int)id;

  free (line);

  fest->pic_cur_id_path = pic_cur_id_path;
}

struct option
get_option (const char *arg)
{
  struct option opt = { 0 };

  if (strncmp (arg, "--id=", strlen ("--id=")) == 0)
    {
      long id = strtol (arg + strlen ("--id="), NULL, 10);
      opt.type = OPT_ID;
      opt.id = (int)id;
    }
  else if (strncmp (arg, "-i", strlen ("-i")) == 0)
    {
      long id = strtol (arg + strlen ("-i"), NULL, 10);
      opt.type = OPT_ID;
      opt.id = (int)id;
    }
  else if (strcmp (arg, "--next") == 0 || strcmp (arg, "-n") == 0)
    {
      opt.type = OPT_NEXT;
    }
  else if (strcmp (arg, "--prev") == 0 || strcmp (arg, "-p") == 0)
    {
      opt.type = OPT_PREV;
    }
  else
    {
      fprintf (stderr, "ERROR: unknown option %s\n", arg);
      exit (EX_USAGE);
    }

  return opt;
}

void
fest_goto_id (struct fest_state *fest, int id)
{
  if (id < 0 || (size_t)id >= fest->pic_list_len)
    {
      fprintf (stderr, "ERROR: id (%d) out of bounds [0, %zu)\n", id,
               fest->pic_list_len);
      exit (EX_USAGE);
    }

  char *cmd = NULL;
  asprintf (&cmd, "/bin/feh --bg-max \"%s\"", fest->pic_list[id]);
  fprintf (stderr, "Executing cmd: %s\n", cmd);
  if (system (cmd) == 0)
    {
      fest->pic_cur_id = id;
      fest_write_id (fest);
    }
  free (cmd);
}

void
fest_goto_next (struct fest_state *fest)
{
  int id = fest->pic_cur_id + 1;
  id %= fest->pic_list_len;
  fest_goto_id (fest, id);
}

void
fest_goto_prev (struct fest_state *fest)
{
  int id = fest->pic_cur_id - 1;
  id %= fest->pic_list_len;
  fest_goto_id (fest, id);
}

void
fest_pic_list_append (struct fest_state *fest, const char *pic_path)
{
  size_t len = fest->pic_list_len + 1;
  size_t cap = fest->pic_list_cap;
  if (len > cap)
    {
      cap = len * 2 > FEST_PIC_LIST_LEN_INIT ? len * 2
                                             : FEST_PIC_LIST_LEN_INIT;
      fest->pic_list = realloc (fest->pic_list, sizeof *fest->pic_list * cap);
      fest->pic_list_cap = cap;
    }

  fest->pic_list[len - 1] = pic_path;
  fest->pic_list_len = len;
}

void
fest_pic_list_free (struct fest_state *fest)
{
  if (!fest->pic_list)
    {
      return;
    }
  for (size_t i = 0; i != fest->pic_list_len; ++i)
    {
      fprintf (stderr, "freeing \"%s\" ... ", fest->pic_list[i]);
      fflush (stderr);
      free ((char *)fest->pic_list[i]);
      fputs ("done\n", stderr);
    }
  free (fest->pic_list);
  fest->pic_list = NULL;
}

void
fest_write_id (struct fest_state *fest)
{
  FILE *f = fopen (fest->pic_cur_id_path, "w");
  if (!f)
    {
      fprintf (stderr, "ERROR: failed to write to %s\n",
               fest->pic_cur_id_path);
      exit (EX_IOERR);
    }

  fprintf (f, "%d\n", fest->pic_cur_id);
  fclose (f);
}
