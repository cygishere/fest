#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define FEST_PIC_LIST_LEN_INIT 5

enum sysexits
{
  EX_OK = 0,
  EX_USAGE = 64,
  EX_NOINPUT = 66,
  EX_OSERR = 71,
  EX_IOERR = 74
};

struct fest_state
{
  char id_path[PATH_MAX];      /* id is a file named `profile.id` in tmp dir
                                  containing the id of current image of
                                  corresponding profile */
  char session_path[PATH_MAX]; /* session is a file in tmp dir containing the
                                  abs path of the current profile */
  char profile_path[PATH_MAX]; /* profile is a file in config dir containing a
                                  list of path to images */
  char *home_dir;              /* $HOME */
  char *cache_dir;              /* $XDG_CACHE_HOME */
  char *profile;      /* name of current profile */
  int pic_id;
};

static void show_help (void);
static inline void show_version (void);

/* if `profile` is NULL, fest will find current profile in session file,
   if there is no session file, fest will fail.
*/
static inline void fest_init (struct fest_state *fest, const char *profile);

static inline void fest_goto_id (struct fest_state *fest, int id);
static inline void fest_goto_next (struct fest_state *fest);
static inline void fest_goto_prev (struct fest_state *fest);

static inline void fest_write_id (struct fest_state *fest);

static void fest_pic_list_append (struct fest_state *fest,
                                  const char *pic_path);
static void fest_pic_list_free (struct fest_state *fest);

static void str_trim (char *str);

int
main (int argc, char **argv)
{
  struct fest_state fest = { 0 };

  switch (argc)
    {
    case 2:
      {
        if (0 == strcmp (argv[1], "-h") ||
            0 == strcmp (argv[1], "--help")
            )
          {
            show_help ();
            exit (EX_OK);
          }
        else if (0 == strcmp (argv[1], "-v") ||
                 0 == strcmp (argv[1], "--version"))
          {
            show_version ();
            exit (EX_OK);
          }
        else if (0 == strcmp (argv[1], "next"))
          {
            fest_goto_next(&fest);
            exit (EX_OK);
          }
        else if (0 == strcmp (argv[1], "prev"))
          {
            fest_goto_prev(&fest);
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
        fest_select_profile(&fest, argv[2]);
        exit (EX_OK);
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
  fprintf (
      stderr,
      "Usage:\n"
      "  fest select <profile>\n"
      "  fest next\n"
      "  fest prev\n"
      "  fest -v | --version\n"
      "  fest -h | --help\n"
      "\n"
      "Subcommands:\n"
      "  select <profile>  set current profile to <profile>,\n"
      "                    which is a file name in ~/.config/fest.\n"
      "  next              set current bg to the next pic of current profile\n"
      "  prev              set current bg to the previous pic of current "
      "profile\n"
      "\n"
      "Options:\n"
      "  -h     --help    Show this screen.\n"
      "  -v     --version Show version.\n"
  );
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

void
fest_init (struct fest_state *fest, const char *profile)
{
  assert(0 && "TODO");
  FILE *f = fopen (profile, "r");
  if (!f)
    {
      fprintf (stderr, "ERROR: failed to open %s: %s\n", profile,
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

      str_trim (line);

      if (line[0] == '#' || line[0] == 0)
        {
          free (line);
          continue;
        }

      if (line[0] == '~')
	{
          const char *home_path = getenv ("HOME");
          const size_t home_path_len = strlen (home_path);
          size_t line_len = strlen (line);
          char *line_expand = malloc (sizeof *line_expand * (home_path_len + line_len + 1));
	  char *temp = stpncpy (line_expand, home_path, home_path_len + 1);
	  stpncpy (temp, line + 1, line_len + 1 - 1); // + 1 to skip the '~'
	  free (line);
	  line = line_expand;
	}

      fest_pic_list_append (fest, line);
      fprintf (stderr, "INFO: pic list append: %s\n", line);
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
  if (id < 0)
    {
      id = fest->pic_list_len - 1;
    }
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

void
str_trim (char *str)
{
  char *cur = str;
  while (*cur)
    {
      ++cur;
    }
  --cur;

  while (isspace (*cur))
    {
      --cur;
    }
  cur[1] = 0;

  char *cpy = str;
  while (*cpy && isspace (*cpy))
    {
      ++cpy;
    }

  cur = str;
  while (*cpy)
    {
      *cur = *cpy;
      ++cur;
      ++cpy;
    }
  *cur = 0;
}
