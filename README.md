# fest - FEh background SeTter

## Usage
```shell
Usage:
  fest <pic_list_path> <pic_cur_id_path> (--id=<id> | --next | --prev | --start)
  fest --version
  fest --help

Options:
  -h     --help    Show this screen.
         --version Show version.
  -i<id> --id=<id> Set background to pic with id <id> in <pic_list_path> (zero-based)
  -n     --next    Set background to next pic
  -p     --prev    Set background to previous pic
  -s     --start   Set background to current pic
```

## pic_list file
- Put absolute path to your images in it.

- Lines starting with # will be ignored.

- Please add new line in the end of the file.

## pic_cur_id file
Please only put a single intger in it.

## TODOs
1. the `pic_cur_id_path` file should not be part of the config
  This file should be a implementation detail which the user should not care about.
  Move it to valid tmp directory.
2. if the `pic_cur_id_path` file is not found, create one for user
3. rm `<pic_cur_id_path>` argument in cli
4. if `<pic_list_path>` does not start with '/' or '~', search in the config dir
