# Why would you use it?
- **Version control**
- Edit with any platform
- Not only Word
- Limited features
- File size
- Exportability

## URL
- [markdown2word](https://hackernoon.com/say-yes-to-markdown-no-to-ms-word-be4692e7a8cd)

# Tools for markdown
- Here is a list of tools that might help you on the way:
- [Markdown editor for Windows](http://markdownpad.com/)
- [Markdown editor for Google Chrome](https://chrome.google.com/webstore/detail/markdown-editor/ekdcaddpmiodcipjfmffhhefijpdckaf)
- **[Markdown editor plugin for Vim](https://github.com/gabrielelana/vim-markdown)**
    - < leader > e  ==> edit something
    - Another reference URL : refer to manual : https://github.com/plasticboy/vim-markdown
- [Markdown pdf converter](https://github.com/alanshaw/markdown-pdf )
- [Markdown slideshow tool](https://github.com/gnab/remark )
- [markdown to word](https://github.com/benbalter/markdown_to_word)

# [mkdocs](https://www.mkdocs.org/)
- sudo apt install mkdocs
- mkdocs new my-project
- cd my-project
- mkdocs serve  <- local http server for 127.0.0.1
- mkdocs build  <- make site
## htppd for mkdocs
- [x] sudo apt-get install apache2
- [x] sudo /etc/init.d/apache2 start | stop | restart
- Default root directory is in /var/www/

# Example
- [ ] Item
    - [x] When I push space , mode will be changed.

- Paragraph
    - Start of the next paragraph }
    - Start of the previous paragraph {

- Motion
    - ]] start of the next header
    - [[ start of the previous header

- < leader >e
```python
D = {}
print "D"
```

- < leader >ft

| A          | B               |
| --         | --              |
| Don't know | Do you know me? |

- :TOhtml


## folding
- :set foldmethod=syntax

- zi - toggle folding

- zj - move to top of next fold
- zk - move to bottom of previous fold

- za - toggle current fold open and closed
- zo - open current fold
- zc - close current fold

- zA - toggle all current folds at the current cursor position
- zO - open all current folds at the current cursor position
- zC - close all current folds at the current cursor position

## ftplugin/markdown.vim
- setlocal ts=4 sw=4 fdn=1
    - tabstop  shiftwidth smartindent
    - setlocal foldenable
    - setlocal foldmethod=indent
- remove if statement (markdown_enable_folding)
    - setlocal foldmethod=expr
    - setlocal foldexpr=markdown#FoldLevelOfLine(v:lnum)

## spell check
- zg  : add word
    - ~/.vim/spell/en.utf-8.add
- set spelllang=en
- set spellfile=$HOME/Dropbox/vim/spell/en.utf-8.add
