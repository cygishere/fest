#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>

#define FEST_PIC_LIST_LEN_INIT 5

enum sysexits
{
  EX_OK = 0,
  EX_USAGE = 64,
  EX_NOINPUT = 66,
  EX_OSERR = 71,
  EX_IOERR = 74
};

/*
  $HOME
  |-- .config/fest
  |   |-- profile1
  |   +-- profile2
  |
  +-- .local/share/fest
      |-- session        -> contains current activate profile
      |-- profile1.id    -> contains current image id for profile1
      +-- profile2.id
  */

struct fest_state
{
  const char *home_dir;        /* $HOME, should not free this */
  char cache_dir[PATH_MAX];    /* $XDG_CACHE_HOME/fest */
  char session_path[PATH_MAX]; /* `session` is a file in `cache_dir` containing
                                  the abs path of the current profile */
  char profile[PATH_MAX];      /* name of current profile */
  char profile_path[PATH_MAX]; /* profile is a file in config dir containing a
                                  list of path to images */
  char id_path[PATH_MAX];      /* id is a file named `profile.id` in tmp dir
                                  containing the id of current image of
                                  corresponding profile */
  int pic_id;
};

static void show_help (void);
static inline void show_version (void);

/* if `profile` is NULL, fest will find current profile in session file,
   if there is no session file, fest will fail.
*/
static inline void fest_init (struct fest_state *fest, const char *profile);

/* if `id` is larger than total pic number, select the first pic.
   if `id` is less than 0, select the first pic.
   */
static inline void fest_goto_id (struct fest_state *fest, int id);
static inline void fest_goto_next (struct fest_state *fest);
static inline void fest_goto_prev (struct fest_state *fest);

static inline void fest_get_pic_path (struct fest_state *fest, int id, char pic_path[PATH_MAX]);

static inline void fest_write_session (struct fest_state *fest);
static inline void fest_write_id (struct fest_state *fest);

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
            fest_init (&fest, NULL);
            fest_goto_next(&fest);
            exit (EX_OK);
          }
        else if (0 == strcmp (argv[1], "prev"))
          {
            fest_init (&fest, NULL);
            fest_goto_prev(&fest);
            exit (EX_OK);
          }
        else if (0 == strcmp (argv[1], "status"))
          {
            fest_init (&fest, NULL);
            fprintf (stderr, "INFO: from profile %s\n", fest.profile_path);
            char pic_path[PATH_MAX];
            fest_get_pic_path(&fest, fest.pic_id, pic_path);
            fprintf (stderr, "INFO: pic: %s\n", pic_path);
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
        if (0 == strcmp (argv[1], "select") ||
            0 == strcmp (argv[1], "s"))
          {
            fest_init (&fest, argv[2]);
            fest_goto_id (&fest, fest.pic_id);
            fest_write_session (&fest);
            exit (EX_OK);
          }
        else
          {
            show_help ();
            exit (EX_USAGE);
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
  fprintf (
      stderr,
      "Usage:\n"
      "  fest (select | s) <profile>\n"
      "  fest next\n"
      "  fest prev\n"
      "  fest status\n"
      "  fest -v | --version\n"
      "  fest -h | --help\n"
      "\n"
      "Subcommands:\n"
      "  (select | s) <profile>  Set current profile to <profile>,\n"
      "                          which is a file name in ~/.config/fest.\n"
      "  next                    Set current bg to the next pic of current profile\n"
      "  prev                    Set current bg to the previous pic of current profile\n"
      "  status                  Show current profile path and pic path\n"
      "\n"
      "Options:\n"
      "  -h     --help           Show this screen.\n"
      "  -v     --version        Show version.\n");
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
  fest->home_dir = getenv ("HOME");
  assert (fest->home_dir && "you should have a $HOME, bro");
  {
    const char *cache_dir = getenv ("XDG_CACHE_HOME");
    if (!cache_dir)
      {
        snprintf (fest->cache_dir, PATH_MAX, "%s/.local/share/fest",
                  fest->home_dir);
      }
    else
      {
        snprintf (fest->cache_dir, PATH_MAX, "%s/fest", cache_dir);
      }
  }

  if (-1 == mkdir (fest->cache_dir, 0777) && errno != EEXIST)
    {
      fprintf (stderr, "ERROR: cannot mkdir '%s': %s\n", fest->cache_dir,
               strerror(errno));
      exit (-1);
    }

  snprintf (fest->session_path, PATH_MAX, "%s/session", fest->cache_dir);

  if (profile)
    {
      strncpy (fest->profile, profile, PATH_MAX);
      snprintf (fest->profile_path, PATH_MAX, "%s/.config/fest/%s", fest->home_dir, fest->profile);
    }
  else
    {
      /* set .profile and .profile_path, user should validate .profile_path */
      FILE *f = fopen (fest->session_path, "r");
      if (!f)
        {
          fprintf (
              stderr,
                   "ERROR: cannot read session, please select a profile first\n");
          exit (EX_USAGE);
        }

      char *line;
      size_t n;
      getline (&line, &n, f);
      str_trim (line);
      strncpy (fest->profile_path, line, PATH_MAX);
      strncpy (fest->profile, basename (line), PATH_MAX);
      free (line);

      fclose (f);
    }

  /* set .id_path and .pic_id*/
  snprintf (fest->id_path, PATH_MAX, "%s/%s.id", fest->cache_dir,
            fest->profile);
  {
    FILE *f = fopen (fest->id_path, "r");
    if (!f)
      {
        fest->pic_id = 0;
      }
    else
      {
        char *line = NULL;
        size_t n;
        getline (&line, &n, f);
        str_trim (line);
        fest->pic_id = strtol (line, NULL, 10);
        free (line);
        fclose (f);
      }
  }
}

void
fest_goto_id (struct fest_state *fest, int id)
{
  char cur_pic[PATH_MAX];
  fest_get_pic_path (fest, id, cur_pic);
  char *cmd = NULL;
  asprintf (&cmd, "/bin/feh --bg-max \"%s\"", cur_pic);
  fprintf (stderr, "Executing cmd: %s\n", cmd);
  if (system (cmd) == 0)
    {
      fest_write_id (fest);
    }
  free (cmd);
}

void
fest_goto_next (struct fest_state *fest)
{
  fest_goto_id (fest, fest->pic_id + 1);
}

void
fest_goto_prev (struct fest_state *fest)
{
  fest_goto_id (fest, fest->pic_id - 1);
}

void
fest_write_id (struct fest_state *fest)
{
  FILE *f = fopen (fest->id_path, "w");
  if (!f)
    {
      fprintf (stderr, "ERROR: failed to write to %s\n",
               fest->id_path);
      exit (EX_IOERR);
    }

  fprintf (f, "%d\n", fest->pic_id);
  fprintf (stderr, "INFO: pic_id writen to %s\n", fest->id_path);
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

static inline void
fest_write_session (struct fest_state *fest)
{
  FILE *f = fopen (fest->session_path, "w");
  if (!f)
    {
      fprintf (stderr, "ERROR: failed to write to %s\n",
               fest->id_path);
      exit (EX_IOERR);
    }

  fprintf (f, "%s\n", fest->profile_path);
  fprintf (stderr, "INFO: session writen to %s\n", fest->session_path);
  fclose (f);
}

static inline void
fest_get_pic_path (struct fest_state *fest, int id, char pic_path[PATH_MAX])
{
    FILE *f = fopen (fest->profile_path, "r");
  if (!f)
    {
      fprintf (stderr, "ERROR: cannot open %s: %s\n", fest->profile_path,
               strerror (errno));
      exit (EX_IOERR);
    }
  /*
    ^~ -- path
    ^/ -- path
    ^$ -- stop
    $ -- stop
    \n~ -- path
    \n/ -- path
    \n$ -- stop
   */
  int len = 0;
  int lastc = '\n';
  int c;
  while (1)
    {
      c = fgetc (f);
      if (c == EOF)
        {
          break;
        }
      else if (lastc == '\n')
        {
          if (c == '~' || c == '/')
            {
              ++len;
            }
        }
      lastc = c;
    }

  if (len == 0)
    {
      fprintf (stderr, "ERROR: profile %s has no valid pic path\n",
               fest->profile_path);
      exit (-1);
    }

  id = id % len;
  if (id < 0)
    {
      id += len;
    }
  fest->pic_id = id;

  rewind (f);
  while (1)
    {
      c = fgetc (f);
      if (c == EOF)
        {
          break;
        }
      else if (lastc == '\n')
        {
          if (c == '~')
            {
              if (id-- == 0)
                {
                  char *line;
                  size_t n;
                  getline (&line, &n, f);
                  snprintf (pic_path, PATH_MAX, "%s%s", fest->home_dir, line);
                  free (line);
                  str_trim (pic_path);
                  break;
                }
            }
          else if (c == '/')
            {
              if (id-- == 0)
                {
                  char *line;
                  size_t n;
                  getline (&line, &n, f);
                  snprintf (pic_path, PATH_MAX, "/%s", line);
                  free (line);
                  str_trim (pic_path);
                  break;
                }
            }
        }
      lastc = c;
    }
  fclose (f);
}
