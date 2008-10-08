COMPILATION OF THE MEGAWAVE USER AND SYSTEM MANUALS

# main targets

* compile the user and system manuals, pdf version
  The result is `xxx/xxx_manual.pdf`.
    make pdf
* compile the user and system manuals, html version
  The result is `xxx/xxx_manual.html`.
    make html
* compile the user and system manuals, splitted version
  The result is the `xxx/xxx_manual_html` folders.
    make splithtml
* cleanup the compilation log, index, ...
    make clean
* cleanup everything, keep only the source
    make distclean
* compile everything and clean
   make && make clean

# other targets

* compile the user and system manuals, ps version
  The result is `xxx/xxx_manual.ps`.
    make ps
* compile the user and system manuals, txt version
  The result is `xxx/xxx_manual.txt`, enriched (use `less` to read it).
    make txt

# required tools

* for pdf
  * pdflatex
  * makeindex
* for ps
  * pdflatex
  * makeindex
  * pdf2ps
* for html
  * hevea
  * csplit, echo, cat
* for split html
  * hacha
  * sed
* for txt
  * recode
  * html2txt

# configuration

All the configuration is in `common/makefile`.
