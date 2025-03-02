# fest - FEh background SeTter

## Usage
```shell
Usage:
  fest select <profile>
  fest next
  fest prev
  fest -v | --version
  fest -h | --help

Subcommands:
  select <profile>  set current profile to <profile>,
                    which is a file name in ~/.config/fest.
  next              set current bg to the next pic of current profile
  prev              set current bg to the previous pic of current profile

Options:
  -h     --help    Show this screen.
  -v     --version Show version.
```

## pic_list file
- Put absolute path to your images in it.

- Lines starting with # will be ignored.

- Please add new line in the end of the file.

## TODOs
1. the `pic_cur_id_path` file should not be part of the config
  This file should be a implementation detail which the user should not care about.
  Move it to valid tmp directory.
2. if the `pic_cur_id_path` file is not found, create one for user
3. rm `<pic_cur_id_path>` argument in cli
4. if `<pic_list_path>` does not start with '/' or '~', search in the config dir
