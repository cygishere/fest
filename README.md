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
