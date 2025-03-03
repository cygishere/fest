# fest - FEh background SeTter

## Usage
```shell
Usage:
  fest (select | s) <profile>
  fest next
  fest prev
  fest status
  fest -v | --version
  fest -h | --help

Subcommands:
  (select | s) <profile>  Set current profile to <profile>,
                          which is a file name in ~/.config/fest.
  next                    Set current bg to the next pic of current profile
  prev                    Set current bg to the previous pic of current profile
  status                  Show current profile path and pic path

Options:
  -h     --help           Show this screen.
  -v     --version        Show version.
```

## <profile> file
- Lines starting with # will be ignored.

## TODOs
- support goto specific pic
- support display current status
