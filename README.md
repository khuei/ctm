# ctm

> command-line interface temporary mail utility using 1secMail API

## Features

- [x] Generate random temporary email addresses
- [x] Create a custom temporary email address
- [x] Manage multiple temporary email addresses
- [x] Read email in plain text
- [x] Read email in rendered HTML
- [ ] Download and view attachments

## OS Support

- [x] Linux
- [ ] MacOS (not yet tested)
- [ ] Windows (not yet tested)

## Usage

### Managing Addresses

```
Usage: ctm addr [command]

create  -- create a custom email address
new     -- create a random email address
current -- display current email address
select  -- set an email address as current
delete  -- remove an email address
```

### Viewing Email

```
Usage: ctm [command]

refresh -- reload mailbox
list    -- show all messages in mailbox
read    -- display specified message
```

### Other Command

```
version -- display version information
help -- display help message
```

## Dependencies

- curl
- json-c
- lexbor

## Installation


to install, run:

``` sh
$ make install
```

to uninstall, run:

``` sh
$ make uninstall
```

## Manual Installation

Place the executable `ctm` in your $PATH. For example, you could also
include this directory in $PATH by adding the following in your profile:

``` sh
PATH=$PATH:path/to/ctm
```

To get zsh completion working, place the `_ctm` completion script in your
fpath. For example, you could add the following to your ~/.zshrc to include the
directory that contains the `_ctm` file in your $fpath:

``` sh
fpath=(path/to/ctm/completion/zsh $fpath)
```

To get bash completion working, source the `ctm.bash` completion script.
For example, add the following to your ~/.bashrc:

``` sh
. path/to/ctm/completion/bash/ctm.bash
```

Reload your shell and the completion script should be working

To get access to the manpage, either place `ctm.1` in your $MANPATH or
include this directory in the $MANPATH by adding the following in your profile:

``` sh
MANPATH=$MANPATH:path/to/ctm
```
